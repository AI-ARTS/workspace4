
#pragma once

#include<memory>
#include<vector>
#include"thread.hpp"
#include"singleton.hpp"
#include<shared_mutex>

namespace sylar{

class FdCtx: public std::enable_shared_from_this<FdCtx>{
public:
    using ptr=std::shared_ptr<FdCtx>;
    FdCtx(int fd);
    ~FdCtx();
    bool isInit()const{return m_isInit;}
    bool isSocket()const {return m_isScoket;}
    bool isClose()const {return m_isClose;}

    void setUserNonblock(bool v){m_userNonblock = v;}
    bool getUserNonblock(){return m_userNonblock;}
    void setSysNonblock(bool v){m_sysNonblock = v;}
    bool getSysNonblock()const{return m_sysNonblock;}

    void setTimeout(int type, uint64_t v);
    uint64_t getTimeout(int type);

private:
    bool init();
private:
    bool m_isInit:1;
    bool m_isScoket:1;
    bool m_isClose:1;

    bool m_userNonblock:1;
    bool m_sysNonblock:1;

    int m_fd;
    // 读超时毫秒
    uint64_t m_recvTimeout;
    // 写超时毫秒
    uint64_t m_sendTimeout; 
};

class FdManager{
public:
    FdManager();
    FdCtx::ptr get(int fd, bool auto_create = false);

    void del(int fd);
private:
    std::shared_mutex m_mutex;
    std::vector<FdCtx::ptr> m_datas;
};

using FdMgr = sylar::Singleton<FdManager>;

}
