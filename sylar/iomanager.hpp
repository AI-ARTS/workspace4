#pragma once
#include"schedule.hpp"
#include<atomic>
#include<unordered_map>
#include"Timer.hpp"
namespace sylar{
class IOmanager:public Schedule, public TimerManager{
public:
    enum Event{
        NONE = 0x00,
        READ = 0x01,
        WRITE = 0x04
    };
public:
    struct fdContext{
        struct EventContext{
            IOmanager* scheduler = nullptr;
            // 事件回调
            std::function<void()> m_cb;
            int m_fd = 0;
        };
        // 读事件
        EventContext read;
        // 写事件
        EventContext write;
        // 事件关联的句柄
        // int fd = 0;
        Event events_now = NONE;
        std::mutex mutex_event;
        bool flag = false; // 用不用翻转
        void reset(Event event);
        void reset(std::function<void()> cb,Event event);
        void reset(int fd, std::function<void()> cb, Event event,IOmanager* th);
        EventContext& GetEvent(IOmanager::Event event);
        void triggerEvent(IOmanager::Event event);
    };
public:
    // 构造函数
    IOmanager(int thread_count, const std::string& name="");

    bool isStopping()const{return m_stopping;}
    // 增加事件
    bool addEvent(int fd,std::function<void()> cb, IOmanager::Event event, bool change = false);
    bool delEvent(int fd, Event event, bool activate = false);
    bool changerEvent(int fd, Event event);
    IOmanager::Event GetFdEvent(int fd);
    // bool cancelEvent(int fd, Event event);
    // 不管什么事件，将所有的事件fd的都清除掉
    bool cancel(int fd);
    bool cancelAllEvent();
    ~IOmanager();
public:
    // void start()override;
    // void run()override;
    void idle(boost::coroutines2::coroutine<int>::push_type& yield)override;
    void stop()override;
    void tickle()override;// 通知任务到了
    // void contextResize(size_t size);// 扩容到多大
    bool hasThread(){return idele_thread_cout>0;}
private:

    int m_tickleFds[2]; //这是管道 0读1写
    std::atomic<size_t> m_number_event{0}; // 当前时间的数量
    // std::vector<fdContext*> m_events;// 保存的是事件
    // 这里的数据进行怎么办呢
    int m_epfd;
    std::unordered_map<int, std::shared_ptr<fdContext>> m_events;
};
}
