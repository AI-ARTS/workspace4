
#include"iomanager.hpp"
#include<mutex>
#include<sys/epoll.h>
#include<unistd.h>
#include<fcntl.h>
#include<assert.h>
#include"hook.hpp"

namespace sylar{
    void IOmanager::fdContext::reset(Event event){
        std::unique_lock<std::mutex> lock(mutex_event);
        if(event !=READ and event != WRITE) return;
        if(event == READ){
            read.m_cb = nullptr;
            read.m_fd = 0;
            read.scheduler = nullptr;
        }else{
            write.m_cb = nullptr;
            write.m_fd = 0;
            write.scheduler = nullptr;
        }
    }
    void IOmanager::fdContext::reset(std::function<void()> cb, Event event){
        // EventContext fd_ctx = GetEvent(event); // 从自身内部拿到event
        // std::unique_lock<std::mutex> lock(mutex_event);
        // fd_ctx.m_cb = cb;
        // // fd_ctx.scheduler = std::dynamic_cast<IOmanager*>(t_scheduler);
        // fd_ctx.scheduler = dynamic_cast<IOmanager*>(t_scheduler);
        std::unique_lock<std::mutex> lock(mutex_event);
        if(event !=READ and event != WRITE) return;
        if(event == READ){
            read.m_cb = cb;
        }else{
            write.m_cb = cb;
        }
    }
    void IOmanager::fdContext::reset(int fd,std::function<void()> cb, Event event, IOmanager* th){
        // EventContext fd_ctx = GetEvent(event); // 从自身内部拿到event
        std::unique_lock<std::mutex> lock(mutex_event);
        if(event !=READ and event != WRITE) return;
        if(event == READ){
            read.m_cb = cb;
            read.m_fd = fd;
            read.scheduler = th;
        }else{
            write.m_cb = cb;
            write.m_fd = fd;
            write.scheduler = th;
        }
    }

    IOmanager::fdContext::EventContext& IOmanager::fdContext::GetEvent(IOmanager::Event event){
        // return events_now;
        switch(event){
            case IOmanager::READ:
                return read;
            case IOmanager::WRITE:
                return write;
            default:
                break;
        }
        SYLAR_LOG_ERROR(SYLAR_LOG_NAME("system"));
        throw std::invalid_argument("event");
    }

    void IOmanager::fdContext::triggerEvent(IOmanager::Event event){
        EventContext ctx = GetEvent(event);
        std::function<void()> cb;
        {
            std::unique_lock<std::mutex> lock(mutex_event);
            if(events_now == NONE or !ctx.m_cb)return;
            cb = ctx.m_cb;
        }
        ctx.scheduler->schedule(cb);
    }

    // 构造函数
    IOmanager::IOmanager(int thread_count, const std::string& name)
    :Schedule(thread_count, name)
    {
        // 预留64的空间
        // m_events.reverse(64);
        sylar::set_hook_enable(true);
        // 要做的事情就是就行创建epoll
        m_epfd = epoll_create(128);
        if(m_epfd == -1){
            SYLAR_LOG_ERROR(SYLAR_LOG_NAME("system"))<<"create epoll base error...";
            return;
        }
        // 将创建管道
        int rt = pipe(m_tickleFds);
        if(rt == -1){
            SYLAR_LOG_ERROR(SYLAR_LOG_NAME("system"))<<"create pipe base error...";
            exit(EXIT_FAILURE);
        }
        // 将其挂载树上 我懂了这个iomanager的原理了。
        epoll_event epev;
        // 0读 边沿触发
        epev.events = EPOLLIN | EPOLLET;
        epev.data.fd = m_tickleFds[0];
        // 设置非阻塞
        rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
        if(rt == -1){
            SYLAR_LOG_ERROR(SYLAR_LOG_NAME("system"))<<"set NONBLOCK error...";
            exit(EXIT_FAILURE);
        }
        // 添加到树上
        rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &epev);
        if(rt == -1){
            SYLAR_LOG_ERROR(SYLAR_LOG_NAME("system"))<<"add tree error...";
            exit(EXIT_FAILURE);
        }
    }

    bool IOmanager::addEvent(int fd, std::function<void()> cb, Event event, bool change){
        // 存在就修改：不存在就增加
        // 不允许挂在空事件
        if(event == NONE  and event !=READ and event != WRITE) return false;
        // 增加事件
        std::unique_lock<std::shared_mutex> lock(schedule_mutex);
        auto it = m_events.find(fd);
        int op  = 0;
        if(it == m_events.end()){
            // 说明不存在重建且挂载
            m_events[fd] = std::shared_ptr<fdContext>(new fdContext);
            op = EPOLL_CTL_ADD;
        }else{
            // 说明其实在树上，但是现在需要进行修改
            op = EPOLL_CTL_MOD;
        }
        // EventContext events = fdContext::GetEvent(event);
        m_events[fd]->events_now = event;
        m_events[fd]->flag = change;
        // // 这个地方没有将其进行改变
        // fdContext::EventContext events = m_events[fd]->GetEvent(event);
        // events.m_fd = fd;
        // events.m_cb = cb;
        // events.scheduler = this;

        m_events[fd]->reset(fd, cb, event, this);

        epoll_event evep;
        evep.events = event | EPOLLET;
        evep.data.fd = fd;
        // 结构体给出挂载上面
        evep.data.ptr = m_events[fd].get();
        lock.unlock();

        int rt = epoll_ctl(m_epfd, op, fd, &evep);
        if(rt == -1){
            SYLAR_LOG_ERROR(SYLAR_LOG_NAME("system"))<<"add event error...";
            return false;
        }
        m_number_event++;
        // 否则这就是挂上了事件了
        fdContext::EventContext events_new = m_events[fd]->GetEvent(event);
        // SYLAR_LOG_INFO(SYLAR_LOG_NAME("system"))<<"event_new-cb:"<<(events_new.m_cb == nullptr)
        //     <<"event_new-mfd:"<<events_new.m_fd<<" lfd:"<<fd;
        return true;
    }

    // 直接从树上摘除
    bool IOmanager::delEvent(int fd,Event event ,bool activate){
        // 对事件进行取消
        // 对其从树上删除的同时不保存其fd
        std::shared_lock<std::shared_mutex> lock(schedule_mutex);
        auto it = m_events.find(fd);
        if(it == m_events.end()){
            return false; // 不存在树上
        }
        lock.unlock();
        std::unique_lock<std::shared_mutex> lock2(schedule_mutex);

        // 在树上，从树上删除
        epoll_event epve;
        epve.events = (it->second)->events_now;
        epve.data.ptr = (it->second).get();
        m_number_event--;

        int rt = epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, &epve);
        if(rt == -1) {
            SYLAR_LOG_ERROR(SYLAR_LOG_NAME("system"))<< "del event faild...";
            return false;
        }
        // 对事件里的函数进行触发吗？感觉触发了不合理啊
				if (activate){
					// 强制触发event 事件
						std::shared_ptr<fdContext> fdc = it->second;
						if(event == IOmanager::READ){
							fdc->read.m_cb();
						}else if(event == IOmanager::WRITE){
							fdc->write.m_cb();
						}
				}
        m_events.erase(it);
        return true;
    }

    // cancel将树上的全部清除？
    bool IOmanager::cancelAllEvent(){

        for(auto &it:m_events){
            if(delEvent(it.first,(Event)0,false)){
                continue;
            }
            SYLAR_LOG_ERROR(SYLAR_LOG_NAME("system"))<<"cancel all event error...";
            return false;
        }
        SYLAR_LOG_ERROR(SYLAR_LOG_NAME("system"))<<"cancel all event successful...";
        m_events.clear();
        m_number_event = 0;
        return true;
    }

    IOmanager::~IOmanager(){
        // SYLAR_LOG_INFO(SYLAR_LOG_NAME("system"))<<"in ~...";
        stop(); // 关闭任务的再度加入
        // 要关闭io的线程去查看是否还有任务，如果有任务也跳过去运行任务
        // 运(行run帮助运行任务
        run();
        for(size_t i=0;i<m_thread.size();i++){
            m_thread[i]->join();
        }
        // SYLAR_LOG_INFO(SYLAR_LOG_NAME("system"))<<"end ~...";
    }
    
    // 从run进入这里
    void IOmanager::idle(boost::coroutines2::coroutine<int>::push_type& yield){
        // 退出的判断逻辑就是
        // 其中的逻辑就是tasknumber存在
        // SYLAR_LOG_INFO(SYLAR_LOG_NAME("system"))<<"in idle...";
        const int epoll_max_number = 256;
        struct epoll_event epvs[epoll_max_number];
        int rt = 0;
        std::vector<std::function<void()>> obs;
        while(true){
            // SYLAR_LOG_INFO(SYLAR_LOG_NAME("system"))<<"idle wait event...";
            // epollwait
            do{
                uint64_t wait_times = 0; // 毫秒级
                uint64_t timer_waite = GetNextTimerMs();
                wait_times = wait_times>timer_waite? timer_waite:wait_times;
                // SYLAR_LOG_INFO(SYLAR_LOG_NAME("system"))<<"wait timer: "<< wait_times;
                rt = epoll_wait(m_epfd, epvs, epoll_max_number, wait_times);
                if(rt<0 and errno == EINTR){
                    continue; // 重试
                }else{
                    break;
                }
                // 我需要存在一个跳出的函数
            }while(true);
            // 这里处理定时器的调度
            // 如果stop就停止时间调去器
            GetExpiredCb(obs);
            if(obs.size()){
                schedule(obs.begin(), obs.end());
                obs.clear();
            }
            for(int i=0;i<rt;i++){
                if(epvs[i].data.fd == m_tickleFds[0]){
                    // 说明这是信号，将其读完就行了
                    char buff[10]{0};
                    read(m_tickleFds[0],buff, 1);
                    // SYLAR_LOG_INFO(SYLAR_LOG_NAME("system"))<<"read "<<t;
                }else{
                    // 这里就是所要调度的事件
                    // 首先拿出事件
                    fdContext* fd_ctx = (fdContext*)epvs[i].data.ptr;
                    epoll_event event = epvs[i];
                    if(event.events &(EPOLLERR | EPOLLHUP)){
                        // 也就是说现在见错误的事件和挂起的时间归类为读写事件
                        event.events |= (EPOLLIN | EPOLLOUT) & fd_ctx->events_now;
                    }

                    int real_events = NONE; // 从其中拿到
                    if(event.events & EPOLLIN){
                        real_events |= READ;
                    }
                    if(event.events & EPOLLOUT){
                        real_events |=WRITE;
                    }
                    // 拿到真实发生的事件是什么
                    if(fd_ctx->events_now & (real_events == NONE)){
                        // 如果是空值的话就循环
                        continue;
                    }
                    if(real_events & READ){
                        // 读事件触发了
                        fd_ctx->triggerEvent(READ);
                        --m_number_event;
                    }
                    if(real_events & WRITE){
                        fd_ctx->triggerEvent(WRITE);
                        --m_number_event;
                    }
                }
            }
            yield(0);
            std::shared_lock<std::shared_mutex> lock_(schedule_mutex);
            if(m_stopping and task_number == 0){
                lock_.unlock();
                break;
            }
        }
        yield(0);
    }


    void IOmanager::stop(){
        std::unique_lock<std::shared_mutex> lock(schedule_mutex);
        m_stopping = true;
        tickle();
        lock.unlock();
        run();// 进去帮忙啊
    }
    void IOmanager::tickle(){
        // 往1写东西
        int rt = write(m_tickleFds[1],"T", 1);
        if(rt == -1){
            // 通知失败
            SYLAR_LOG_ERROR(SYLAR_LOG_NAME("system"))<<"tickle signal error...";
            assert(0);
        }
    }
    bool IOmanager::cancel(int fd){
        // 这里先检索
        std::shared_lock<std::shared_mutex> lock(schedule_mutex);
        auto it = m_events.find(fd);
        lock.unlock();
        if(it == m_events.end()) return false;

        std::unique_lock<std::shared_mutex> lock2(schedule_mutex);
        std::shared_ptr<fdContext> fd_ctx = it->second;
        m_events.erase(it);
        struct epoll_event epv;
        epv.events = it->second->events_now;
        epv.data.ptr = it->second.get();
        epv.data.fd = fd;
        int rt = epoll_ctl(m_epfd,EPOLL_CTL_DEL, fd,&epv);
        if(rt == -1){
            SYLAR_LOG_ERROR(SYLAR_LOG_NAME("system"))<<"cancel error:"<<fd;
        return false;
        }
        return true;
    }

    bool IOmanager::changerEvent(int fd, IOmanager::Event event){
        std::shared_lock<std::shared_mutex> lock(schedule_mutex);
        auto it = m_events.find(fd);
        lock.unlock();
        if(it == m_events.end()) return false;
        
        std::unique_lock<std::shared_mutex> lock2(schedule_mutex);
        std::shared_ptr<fdContext> fd_ctx = it->second;
        // m_events.erase(it);
        it->second->events_now = event;
        struct epoll_event epv;
        epv.events = it->second->events_now | EPOLLET;
        epv.data.ptr = it->second.get();
        epv.data.fd = fd;
        int rt = epoll_ctl(m_epfd,EPOLL_CTL_MOD, fd,&epv);
        if(rt == -1){
            SYLAR_LOG_ERROR(SYLAR_LOG_NAME("system"))<<"mod error:"<<fd;
        return false;
        }
        return true;
    }

    IOmanager::Event IOmanager::GetFdEvent(int fd){
        std::shared_lock<std::shared_mutex> lock(schedule_mutex);
        auto it = m_events.find(fd);
        lock.unlock();
        if(it == m_events.end()) return NONE;
        // std::shared_ptr<fdContext> fd_ctx = it->second;
        return it->second->events_now;
    }

}


// 调试bug的问题所在
// 问题1: 当前的时钟拿错了
// 问题2: 是没有将Timer进行空构造
// 问题3: 现在调度器停止会出现错误
