
#include"schedule.hpp"
#include"util.hpp"
#include<chrono>

namespace sylar{
    static Schedule* t_scheduler = nullptr; // 为了随时能够拿到当前所使用的调度器
    Schedule* Schedule::GetThis(){return t_scheduler;}
    Schedule::Schedule(int thread_count,const std::string& name){
        m_thread_cout = thread_count;
        m_name = name;
        t_scheduler = this; // 那个线程创建，哪个线程就保存这个地址
    }

    Schedule::funcAndthread::funcAndthread(){}
    Schedule::funcAndthread::funcAndthread(std::function<void()> cb, int threadid){
        thread_id = threadid;
        m_cb = cb;
    }
    void Schedule::funcAndthread::reset(){
        m_cb = nullptr;
        thread_id = -1;
    }
    void Schedule::funcAndthread::reset(std::function<void()> cb, int threadid){
        m_cb.swap(cb);
        thread_id = threadid;
    }

    void Schedule::start(){
        // 进行线程的创建
        for(int i = 0;i<m_thread_cout;i++){
            m_thread.emplace_back(new Thread("local_thread"+std::to_string(i),[this](){
                this->run();}));
        }
        SYLAR_LOG_INFO(SYLAR_LOG_NAME("system"))<<"thread creat over";
    }
    // 进行阻塞
    Schedule::~Schedule(){
        // 将所有的线程进行join
        m_stopping = true;
        for(size_t i=0;i<m_thread.size();i++){
            m_thread[i]->join();
        }
        SYLAR_LOG_INFO(SYLAR_LOG_NAME("system"))<<"join thread...";
    }

    void Schedule::schedule(std::function<void()> cb, int threadId){
        if(m_stopping) return;
        std::unique_lock<std::shared_mutex> lock(schedule_mutex);
        // m_task.emplace_back(cb, threadId);
        // 对任务队列进行遍历寻找空的任务
        for(auto& it:m_task){
            if(it.m_cb) continue;
            it.reset(cb, threadId);
            task_number++;
            lock.unlock();
            tickle();
            return;
        }
        task_number++;
        m_task.emplace_back(cb, threadId);
        lock.unlock();
        tickle();
    }

    void Schedule::run(){
        // 停止了定时器一定是不起作用了，为什么主线程仍然在运行呢。
        auto idle_fiber = boost::coroutines2::coroutine<int>::pull_type([this](boost::coroutines2::coroutine<int>::push_type& yield) {
            this->idle(yield);
        }); // 这里开始对任务的遍历且运行
        while(true){
            struct funcAndthread task;
            {
                std::unique_lock<std::shared_mutex> lock(schedule_mutex);
                for(size_t i =0;i<m_task.size();i++){
                    if(m_task[i].m_cb and (m_task[i].thread_id = -1 or m_task[i].thread_id == sylar::getThreadid())){
                        task = m_task[i];
                        m_task[i].reset(); // 将其置空
                        break;
                    }
                }
                if(task.m_cb) idele_thread_cout--; // 由于接下来就进行运行了
            }
            // 对任务进行运行
            if(task.m_cb){
                task.m_cb();// 开始运行
                std::unique_lock<std::shared_mutex> lock2(schedule_mutex);
                task_number--; // 这里进行的是事件计数减一
                lock2.unlock();
            }


            std::shared_lock<std::shared_mutex> lock3(schedule_mutex);
            if(m_stopping and task_number == 0){ 
                lock3.unlock();
                // SYLAR_LOG_INFO(SYLAR_LOG_NAME("system"))<<" func return ...";
                return;
            } // 如果现在收到了退出的信号，就退出。。
            // idele_thread_cout++;
            // SYLAR_LOG_INFO(SYLAR_LOG_NAME("system"))<<" task_number..."<<task_number
                // <<" m_sttoping: "<<m_stopping;
            lock3.unlock();
            idle_fiber();
            // SYLAR_LOG_INFO(SYLAR_LOG_NAME("system"))<<"to idle...";
        }
    }


    void Schedule::idle(boost::coroutines2::coroutine<int>::push_type& yield){
        // 陷入在这里
        while(true){
            SYLAR_LOG_INFO(SYLAR_LOG_NAME("system"))<<"idle wait...";
            {
                std::shared_lock<std::shared_mutex> lock(schedule_mutex);
                if(task_number == 0){
                    if(m_stopping) break;
                    continue;
                }
                else{
                    lock.unlock();
                    yield(0);
                }
            }
        }
    }

    void Schedule::stop(){
        std::unique_lock<std::shared_mutex> lock(schedule_mutex);
        m_stopping = true;
        run();// 帮助运行函数
    }
    void Schedule::tickle(){}
}
