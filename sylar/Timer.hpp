
#pragma once

#include<memory>
#include<set>
#include<functional>
#include<shared_mutex>
#include<mutex>


namespace sylar{
    // 进行timer模块的处理
class TimerManager;
class Timer:public std::enable_shared_from_this<Timer>{
friend class TimerManager;
public:
using ptr = std::shared_ptr<Timer>;
Timer(uint64_t gap,std::function<void()>,bool recurring, TimerManager* Tmanager);
    bool cancel();// 取消定时器
    void refresh();// 刷新定时器
    void reset();
    // 是否从当前进行计算
    void reset(uint64_t gap, bool from_now=true);
private:
    Timer();
    Timer(uint64_t ms);
    // 执行的时间戳
    Timer(uint64_t next, std::function<void()> cb);
private:
    uint64_t m_gap = 0;
    uint64_t m_next = 0;
    // 定时器是否循环触发
    bool m_recurring = false; 
    std::function<void()> m_cb;
    TimerManager* m_timeManager;
public:
		uint64_t GetNextMs()const;
private:
    struct comparator{
        bool operator()(const Timer::ptr& a,const Timer::ptr& b)const{
            return a->m_next<b->m_next;
        }
    };
};

class TimerManager{
friend class Timer;
public:
    TimerManager();
    virtual ~TimerManager(){}
    Timer::ptr addTimer(uint64_t ms, std::function<void()> cb, bool recurring=false);
    void addTimer(Timer::ptr);
    // 直接删除了。
    Timer::ptr addCondtionTimer(uint64_t ms, std::function<void()> cb,
        std::weak_ptr<void> weak_cond, // 条件
        bool recurring = false  // 是否循环
    );
    uint64_t GetNextTimerMs();
    void GetExpiredCb(std::vector<std::function<void()>>& obs);
    bool hasTimer()const;
private:
    std::set<Timer::ptr, Timer::comparator> m_timers;
    std::shared_mutex TimerRWMutex;
};


}

