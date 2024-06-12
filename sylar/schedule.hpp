#pragma once
#include "thread.hpp"
#include<shared_mutex>
#include<boost/coroutine2/all.hpp>
#include<string>

namespace sylar{
class Schedule;
class Schedule:public std::enable_shared_from_this<Schedule>{
public:
    using ptr = std::shared_ptr<Schedule>;
    struct funcAndthread{
        std::function<void()> m_cb = nullptr;
        int thread_id = -1;
        funcAndthread();
        funcAndthread(std::function<void()> cb, int threadid = -1);
        void reset();
        void reset(std::function<void()> cb, int threadid = -1);
    };
public:
    Schedule(int thread_count = 1,const std::string& name = "");
    void schedule(std::function<void()> cb, int threadId = -1);
    void schedule(const struct funcAndthread& funct);
    template <class Iterator>
    void schedule(Iterator begin, Iterator end){
        // 这里整和成迭代器
        std::unique_lock<std::shared_mutex> lock(schedule_mutex);
        auto it = m_task.begin();
        while(it!=m_task.end() and begin!=end){
            if((*it).m_cb){
                it++;
                continue;
            }
            (*it).m_cb = (*begin);
            it++;
            begin++;
            task_number++;
        }

        if(begin == end) return;
        // 否则还存在
        while(begin!=end){
            task_number++;
            m_task.emplace_back((*begin));
            begin++;
        }
        lock.unlock();
        tickle();
    }

public:
    void start();
    // 所有的线程会先进入run，然后没给线程会进入idle里面进行阻塞
    void run();// 也需要重写这个run和start
    // 将其设置为一个协程
    virtual void idle(boost::coroutines2::coroutine<int>::push_type& yield);
    virtual void stop();
    virtual void tickle();// 通知任务到了
    virtual ~Schedule(); 
    static Schedule* GetThis();
public:
    int GetThreadCount()const{return m_thread_cout;}
    int GetIdleThread()const{return idele_thread_cout;}
    bool isStop()const{return m_stopping;}
    std::string GetName()const{return m_name;}
protected:
    std::vector<sylar::Thread::ptr> m_thread;
    std::vector<funcAndthread> m_task;
    std::shared_mutex schedule_mutex;
    int m_thread_cout = 1;
    int idele_thread_cout = 0;
    bool m_stopping = false;
    int task_number = 0;
    std::string m_name;
};
}
