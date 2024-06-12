

#include"fd_manager.hpp"
#include"hook.hpp"
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
    // bool m_isInit:1;
    // bool m_isScoket:1;
    // bool m_isClose:1;

    // bool m_userNonblock:1;
    // bool m_sysNonblock:1;

    // int m_fd;
    // // 读超时毫秒
    // uint64_t m_recvTimeout;
    // // 写超时毫秒
    // uint64_t m_sendTimeout; 
namespace sylar{

    FdCtx::FdCtx(int fd)
    :m_isInit(false)
    ,m_isScoket(false)
    ,m_isClose(false)
    ,m_userNonblock(false)
    ,m_sysNonblock(false)
    ,m_fd(fd)
    ,m_recvTimeout(-1)
    ,m_sendTimeout(-1)
    {
        init();
    }
    FdCtx::~FdCtx(){}

    bool FdCtx::init(){
        if(m_isInit)return true;
        m_recvTimeout = -1;
        m_sendTimeout = -1;
        struct stat fd_stat;
        if(fstat(m_fd, &fd_stat) == -1){
            m_isInit = false;
            m_isScoket = false;
        }else{
            m_isInit = true;
            m_isScoket = S_ISSOCK(fd_stat.st_mode);
        }

        if(m_isScoket){
            // 如果是sock
            int flags = fcntl_f(m_fd, F_GETFL, 0);
            // 如果不是无阻塞，默认设置阻塞
            if(!(flags & O_NONBLOCK)){
            fcntl_f(m_fd, F_SETFL, flags|O_NONBLOCK);
            }
            m_sysNonblock = true;
        }else{
            m_sysNonblock = false;
        }
        m_userNonblock = false;
        m_isClose = false;
        return m_isInit;
    }
    void FdCtx::setTimeout(int type, uint64_t v){
        // 设置超时
        if(type == SO_RCVTIMEO){
            m_recvTimeout = v;
        }else{
            m_sendTimeout = v;
        }
    }
    uint64_t FdCtx::getTimeout(int type){
        if(type == SO_RCVTIMEO){
            return m_recvTimeout;
        }else{
            return m_sendTimeout;
        }
    }

    FdManager::FdManager(){
        m_datas.resize(64);
    }

    FdCtx::ptr FdManager::get(int fd, bool auto_create){
        if(fd == -1){
            return nullptr;
        }

        std::shared_lock<std::shared_mutex> lock(m_mutex);
        if((int)m_datas.size() <=fd){
            if(auto_create == false){
                return nullptr;
            }
        }else{
            if(m_datas[fd] or !auto_create){
                return m_datas[fd];
            }
        }
        lock.unlock();
        // 创造文件描述符
        std::unique_lock<std::shared_mutex> lock2(m_mutex);
        FdCtx::ptr ctx(new FdCtx(fd));
        if(fd >= (int)m_datas.size()){
            m_datas.resize(fd*1.5);
        }
        m_datas[fd] = ctx;
        return ctx;
    }

    void FdManager::del(int fd){
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        if((int)m_datas.size()<fd)return;
        FdCtx::ptr fds = m_datas[fd];
        m_datas[fd].reset();
    }


}