
#include"hook.hpp"
#include<dlfcn.h>
#include<boost/coroutine2/all.hpp>
#include"config.hpp"
#include"logger.hpp"
#include"iomanager.hpp"
#include"fd_manager.hpp"
#include"macro.hpp"
#include"schedule.hpp"
#include<tuple>
// #include<apply>

namespace sylar{

// 首先需要进行覆盖
    static thread_local bool t_hook_enable = true;// 使能标识位
    #define HOOK_FUN(XX) \
        XX(sleep) \
        XX(usleep) \
        XX(nanosleep) \
        XX(socket) \
        XX(connect) \
        XX(accept) \
        XX(read) \
        XX(readv) \
        XX(recv) \
        XX(recvfrom) \
        XX(recvmsg) \
        XX(write) \
        XX(writev) \
        XX(send) \
        XX(sendto) \
        XX(sendmsg) \
        XX(close) \
        XX(fcntl) \
        XX(ioctl) \
        XX(getsockopt) \
        XX(setsockopt)

void hook_init(){
    static bool is_inited = false;
    if(is_inited) return;
    #define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
        HOOK_FUN(XX);
    #undef XX
}


struct _HookIniter{
    _HookIniter(){
        hook_init(); // 对被覆盖的进行勾出来
    }
};

static _HookIniter s_hook_initer;

bool is_hook_enable(){
    return t_hook_enable;
}

void set_hook_enable(bool flag){
    t_hook_enable = flag;
}
}

struct timer_info{
    int cancelled = 0;
};


template<typename OriginFun, typename... Args>
static ssize_t do_io(int fd, OriginFun fun, const char* hook_fun_name
    , uint32_t event, int timeout_so, Args&&... args){
        if(!sylar::t_hook_enable){
            // 如果不hook就直接返回
            return fun(fd, std::forward<Args>(args)...);
        }

        sylar::FdCtx::ptr ctx = sylar::FdMgr::GetInstance()->get(fd);
        // 如果描述符管理器不存在直接返回原函数

        if(!ctx){
            return fun(fd, std::forward<Args>(args)...);
        }
        if(ctx->isClose()){
            errno = EBADF;
            return -1;
        }

        if(!ctx->isSocket() || ctx->getUserNonblock()){
            return fun(fd, std::forward<Args>(args)...);
        }

        // 输入的文件描述符的type
        ctx->setTimeout(timeout_so, 1000);
        uint64_t to = ctx->getTimeout(timeout_so);
        std::shared_ptr<timer_info> tinfo(new timer_info);
		// 将这个扯出一个弱链接
	    std::weak_ptr<timer_info> weak_cond(tinfo);
	    // 拿到 调度器
        // 现在存在问题，这个线程内部可能不存在调度器这就麻烦了。
		sylar::IOmanager* iom = dynamic_cast<sylar::IOmanager*>(sylar::Schedule::GetThis());
		ssize_t n = fun(fd, std::forward<Args>(args)...);
				// 中断重试
		while(n == -1 and errno == EINTR){
			n = fun(fd, std::forward<Args>(args)...);
		}
		if(n == -1 and (errno == EAGAIN)){
            // 如果说是资源分配不足
		    sylar::Timer::ptr condtionTimer;
			// 先将定时事件安装上去
			if(to!=(uint64_t)-1){
			    // 设置定时大事
			    condtionTimer = iom->addCondtionTimer(to,[weak_cond, fd, iom, event](){
                    auto t = weak_cond.lock();
					if(!t or t->cancelled) return;
					t->cancelled = ETIMEDOUT;// 修改条件 
					iom->delEvent(fd,(sylar::IOmanager::Event)(event), true);// 对事件删除
                },weak_cond);
				auto idle_fiber = boost::coroutines2::coroutine<int>::pull_type([iom](boost::coroutines2::coroutine<int>::push_type&yield){
									yield(0);// 这样先不进入idle中:先出去增加事件
									iom->idle(yield);}); // 这样就会瞬间运行到idle，轮询一圈的时候如何调回来到这里呢
                
                // 增加事件，关键是在n的状态的改变。
				// 开始增加事件： 我怎么觉的这块要出错误: 这样就事件出发了就随便寻个线程
                // 给运行了
                bool flag = true;

                 iom->addEvent(fd, [&flag](){
                    flag = false;
                }, sylar::IOmanager::Event(event));

                while(n == -1 and errno == EAGAIN){
                    flag = true;
                    while(flag){
                        idle_fiber();// 不断的跳出去
                    }
                    // 这说明来事件
                    n = fun(fd, std::forward<Args>(args)...);
                }
                // 没有超时，运行去除
                // 这里对这个fd 的事情运行完了
                bool delEventFalg = iom->delEvent(fd, (sylar::IOmanager::Event)(event)); // 不进行触发
                if(!delEventFalg){
                    SYLAR_LOG_ERROR(SYLAR_LOG_NAME("system"))<<"del event Error...";
                }
                if(n == -1){
                    // 这里是到达-1了说明发生了未知的错误
                    // 进行事件的剔除，事件的剔除已经i
                    if(!delEventFalg){
                        // 发生错误了打印日志
                        SYLAR_LOG_ERROR(SYLAR_LOG_NAME("system"))<<"del event error...";
                        return -1;
                    }
                    // 定时器删除
                    if(condtionTimer) condtionTimer->cancel();
                }
            }
        }
	return n; // 暂时这里直接给出结果，不进行异步了
}

extern "C"{

#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);
#undef XX


unsigned int sleep(unsigned int seconds){
    if(!sylar::t_hook_enable){
        return sleep_f(seconds);
    }
    // 增加定时器来通知
    // 增加完定时器后跳回到这里
    sylar::IOmanager* iom = dynamic_cast<sylar::IOmanager*>(sylar::Schedule::GetThis());
    auto idle_fiber = boost::coroutines2::coroutine<int>::pull_type([iom](boost::coroutines2::coroutine<int>::push_type& yield){
        yield(0);
        iom->idle(yield);
    });	
	bool flag = true;
    // 增加定时器
    sylar::Timer::ptr sleepTimer;
	sleepTimer = iom->addTimer(seconds*1000,[&flag,&sleepTimer](){
						flag = false;
						},false);

		while(flag){
			idle_fiber();// 不断的让其跳入这里
		}
		return 0;
}

int usleep(useconds_t usec) {
    if(!sylar::t_hook_enable) {
        return usleep_f(usec);
    }
		
	sylar::IOmanager* iom = dynamic_cast<sylar::IOmanager*>(sylar::Schedule::GetThis());
	int flag = true;
    auto idle_fiber = boost::coroutines2::coroutine<int>::pull_type([iom](boost::coroutines2::coroutine<int>::push_type& yield){
        yield(0);
        iom->idle(yield);    
    });
	// 增加定时器
    sylar::Timer::ptr sleepTimer;
		sleepTimer = iom->addTimer(usec/1000,[&flag,&sleepTimer](){
						flag = false;
						sleepTimer->cancel();// 将自己取消掉
						},false);

		while(flag){
			idle_fiber();// 不断的让其跳入这里
		}
		return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
    if(!sylar::t_hook_enable) {
        return nanosleep_f(req, rem);
    }

    sylar::IOmanager* iom = dynamic_cast<sylar::IOmanager*>(sylar::Schedule::GetThis());
    auto idle_fiber = boost::coroutines2::coroutine<int>::pull_type([iom](boost::coroutines2::coroutine<int>::push_type& yield){
        yield(0);
        iom->idle(yield);    
    });
    int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 /1000;
    int flag = true;
    sylar::Timer::ptr sleepTimer;
    sleepTimer = iom->addTimer(timeout_ms,[&flag, &sleepTimer](){
                    flag = false;
                    sleepTimer->cancel();
                    });
    while(flag){
        idle_fiber();
    }
    return 0;
}


int socket(int domain, int type, int protocol){
	if(!sylar::t_hook_enable){
		return socket_f(domain, type, protocol);
	}
	// hook了这里做什么操作呢；// 这里进行管理
	int sockfd = socket_f(domain, type, protocol);
	if(sockfd == -1){
		return -1;
	}
	sylar::FdCtx::ptr fd_ctx = sylar::FdMgr::GetInstance()->get(sockfd, true);
	return sockfd;
}

// int sockfd, const struct sockaddr *addr, socklen_t addrlen
int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms){
	// 老样子首先判断是否hook住了
	if(!sylar::t_hook_enable){
		return connect_f(fd, addr, addrlen);
	}
	sylar::FdCtx::ptr ctx = sylar::FdMgr::GetInstance()->get(fd);
	if(!ctx) return connect_f(fd, addr, addrlen);

	if(ctx->isClose()){
		errno = EBADF;
		return -1;
	}
	
	if(ctx->getUserNonblock()){
		return connect_f(fd, addr, addrlen);

	}
	int n = connect_f(fd, addr, addrlen);
	if(n == 0){
		return 0;
	}else if(n !=-1 or errno == EINPROGRESS){
		return 0;
	}

	sylar::IOmanager* iom = dynamic_cast<sylar::IOmanager*>(sylar::Schedule::GetThis());
	sylar::Timer::ptr timer;
	std::shared_ptr<timer_info> tinfo(new timer_info);
	std::weak_ptr<timer_info> winfo(tinfo);
	

	// 创建协程
	auto idle_fiber = boost::coroutines2::coroutine<int>::pull_type([iom](
			boost::coroutines2::coroutine<int>::push_type& yield
							){
					yield(0);
					iom->idle(yield);
					});
	bool flag = true;

	if(timeout_ms!=(uint64_t)-1){
		timer = iom->addCondtionTimer(timeout_ms, [winfo, &flag, fd, iom](){
						auto t = winfo.lock();
						if(!t or t->cancelled){
							return;
						}
						flag = false;
						t->cancelled = ETIMEDOUT;
						iom->delEvent(fd, sylar::IOmanager::WRITE, true);
						}, winfo);
	}
		bool rt = iom->addEvent(fd,[&flag,&timer,iom,fd](){
				// 定时到点
					flag = false; //到点了
					// 取消事件和定时器
					timer->cancel();
					timer.reset();
					flag = false;
					iom->delEvent(fd, sylar::IOmanager::WRITE, true);// 强制触发
						},sylar::IOmanager::WRITE);
		if(rt){
			// 这里增加好了事件 这里跳走
			// 检测是否挂好了
            // 就是是否处于关闭状态
			while(flag and iom->isStopping()){
				idle_fiber();// 让出cpu控制权，// 这里可能出现
				// 使其去进行请求的处理。处理一趟回来查看falg被触发了没
			}
			// 执行回来了
			if(timer) timer->cancel();
			if(tinfo->cancelled){

				errno = tinfo->cancelled;
				return -1;
			}

		}else{
			// 增加失败
			if(timer)timer->cancel();

			SYLAR_LOG_ERROR(SYLAR_LOG_NAME("system"))<<"connect addEvent ("<<fd<<", WRITE) error";
		}
		int error = 0;
		socklen_t len = sizeof(int);
		if(getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len)){
			return -1;
		}


		if(!error) return 0;
		else{
			errno = error;
			return -1;
		}
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    return connect_with_timeout(sockfd, addr, addrlen, 5000);
}

int accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
    int fd = accept_f(s, addr, addrlen);
    // 现在就是说没有hook到这个函数
    // int fd = do_io(s, accept_f, "accept", sylar::IOmanager::READ, SO_RCVTIMEO, addr, addrlen);
    if(fd >= 0) {
        sylar::FdMgr::GetInstance()->get(fd, true);
    }
    return fd;
}

ssize_t read(int fd, void *buf, size_t count) {
    return do_io(fd, read_f, "read", sylar::IOmanager::READ, SO_RCVTIMEO, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, readv_f, "readv", sylar::IOmanager::READ, SO_RCVTIMEO, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return do_io(sockfd, recv_f, "recv", sylar::IOmanager::READ, SO_RCVTIMEO, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    return do_io(sockfd, recvfrom_f, "recvfrom", sylar::IOmanager::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    return do_io(sockfd, recvmsg_f, "recvmsg", sylar::IOmanager::READ, SO_RCVTIMEO, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return do_io(fd, write_f, "write", sylar::IOmanager::WRITE, SO_SNDTIMEO, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, writev_f, "writev", sylar::IOmanager::WRITE, SO_SNDTIMEO, iov, iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags) {
    return do_io(s, send_f, "send", sylar::IOmanager::WRITE, SO_SNDTIMEO, msg, len, flags);
}

ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
    return do_io(s, sendto_f, "sendto", sylar::IOmanager::WRITE, SO_SNDTIMEO, msg, len, flags, to, tolen);
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
    return do_io(s, sendmsg_f, "sendmsg", sylar::IOmanager::WRITE, SO_SNDTIMEO, msg, flags);
}

int close(int fd) {
    if(!sylar::t_hook_enable) {
        return close_f(fd);
    }

    sylar::FdCtx::ptr ctx = sylar::FdMgr::GetInstance()->get(fd);
    if(ctx) {
        auto iom = dynamic_cast<sylar::IOmanager*>(sylar::Schedule::GetThis());
        if(iom) {
            iom->cancel(fd);
        }
        sylar::FdMgr::GetInstance()->del(fd);
    }
    return close_f(fd);
}

int fcntl(int fd, int cmd, ... /* arg */ ) {
    va_list va;
    va_start(va, cmd);
    switch(cmd) {
        case F_SETFL:
            {
                int arg = va_arg(va, int);
                va_end(va);
                sylar::FdCtx::ptr ctx = sylar::FdMgr::GetInstance()->get(fd);
                if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                    return fcntl_f(fd, cmd, arg);
                }
                ctx->setUserNonblock(arg & O_NONBLOCK);
                if(ctx->getSysNonblock()) {
                    arg |= O_NONBLOCK;
                } else {
                    arg &= ~O_NONBLOCK;
                }
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETFL:
            {
                va_end(va);
                int arg = fcntl_f(fd, cmd);
                sylar::FdCtx::ptr ctx = sylar::FdMgr::GetInstance()->get(fd);
                if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                    return arg;
                }
                if(ctx->getUserNonblock()) {
                    return arg | O_NONBLOCK;
                } else {
                    return arg & ~O_NONBLOCK;
                }
            }
            break;
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif
            {
                int arg = va_arg(va, int);
                va_end(va);
                return fcntl_f(fd, cmd, arg); 
            }
            break;
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif
            {
                va_end(va);
                return fcntl_f(fd, cmd);
            }
            break;
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
            {
                struct flock* arg = va_arg(va, struct flock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETOWN_EX:
        case F_SETOWN_EX:
            {
                struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        default:
            va_end(va);
            return fcntl_f(fd, cmd);
    }
}

int ioctl(int d, unsigned long int request, ...) {
    va_list va;
    va_start(va, request);
    void* arg = va_arg(va, void*);
    va_end(va);

    if(FIONBIO == request) {
        bool user_nonblock = !!*(int*)arg;
        sylar::FdCtx::ptr ctx = sylar::FdMgr::GetInstance()->get(d);
        if(!ctx || ctx->isClose() || !ctx->isSocket()) {
            return ioctl_f(d, request, arg);
        }
        ctx->setUserNonblock(user_nonblock);
    }
    return ioctl_f(d, request, arg);
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
    return getsockopt_f(sockfd, level, optname, optval, optlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    if(!sylar::t_hook_enable) {
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }
    if(level == SOL_SOCKET) {
        if(optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
            sylar::FdCtx::ptr ctx = sylar::FdMgr::GetInstance()->get(sockfd);
            if(ctx) {
                const timeval* v = (const timeval*)optval;
                ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
            }
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);
}



}

