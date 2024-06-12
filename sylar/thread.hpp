
#ifndef __THREAD__HPP__
#define __THREAD__HPP__
#include<thread>
#include<functional>
#include<memory>
#include<pthread.h>
#include<sylar/logger.hpp>
#include<semaphore.h>
#include<errno.h>
#include<condition_variable>
#include<mutex>
#include<thread>
#include<string>
#include<pthread.h>

namespace sylar{

class Semaphore{ // 使用条件变量来实现信号量。
public:
    Semaphore(uint32_t count = 0);
    ~Semaphore();
    void wait();
    void notify();
private:
    Semaphore(const Semaphore& ) = delete;
    Semaphore(const Semaphore&&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;
private:
    
    std::condition_variable condition;
    uint32_t value;
    // std::shared_mutex mutex_;
};


class Thread{
public:
    using ptr = std::shared_ptr<Thread>;
    // 传入线程所要运行的函数
    Thread(
        const std::string& name,
        std::function<void()> cb
        );
    ~Thread();
    // 拿到线程id
    pid_t getID()const{return m_id;}
    // 拿到线程名字
    const std::string& getName()const {return m_name;}
    void join();
    static Thread* GetThis();
    static const std::string& GetName();
    static void setName(const std::string& name);
private:
    Thread(const Thread& ) = delete;
    Thread(const Thread&& ) = delete;
    Thread& operator=(const Thread&) = delete;
    static void* run(void* arg);

private:
    // std::unordered_map<std::string, std::thread> maps;
    pid_t m_id;
    std::thread m_thread;
    std::function<void()> m_cb;
    std::string m_name;
    Semaphore _semaphore;
};


}

#endif