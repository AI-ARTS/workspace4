#include"Timer.hpp"
#include"util.hpp"
#include"logger.hpp"

namespace sylar{


Timer::Timer(){
	m_next = sylar::GetCurrentMS();
}

Timer::Timer(uint64_t gap,std::function<void()> cb, bool recurring, TimerManager* Tmanager){
    m_recurring = recurring;
    m_gap = gap;
    m_next = sylar::GetCurrentMS()+gap;
    m_cb = cb;
    m_timeManager = Tmanager;
} 

bool Timer::cancel(){
    // 将自身的定时器进行清除掉
    std::shared_lock<std::shared_mutex> lock(m_timeManager->TimerRWMutex);
    auto it = m_timeManager->m_timers.find(shared_from_this());
    lock.unlock();
    if(it == m_timeManager->m_timers.end()){
        // 这里进行返回
        return false;
    }
    std::unique_lock<std::shared_mutex> lock2(m_timeManager->TimerRWMutex);
    m_timeManager->m_timers.erase(it);
    return true;
}

void Timer::refresh(){
    // 对自己重新进行刷新
    std::shared_lock<std::shared_mutex> lock(m_timeManager->TimerRWMutex);
    auto it = m_timeManager->m_timers.find(shared_from_this());
    lock.unlock();
    if(it == m_timeManager->m_timers.end()) return;
    Timer::ptr temp = (*it);
    m_timeManager->m_timers.erase(it);
    temp->m_next = sylar::GetCurrentMS()+m_gap;
    std::unique_lock<std::shared_mutex> lock2(m_timeManager->TimerRWMutex);
    m_timeManager->m_timers.insert(temp);
}

void Timer::reset(){
    //存在就置空
    cancel();// 先将自删除了
    m_gap = 0;
    m_next = 0;
    m_recurring = false;
    m_cb = nullptr;
    m_timeManager = nullptr;
}

// 从新设置间隔
void Timer::reset(uint64_t gap, bool from_now){
        // 对自己重新进行刷新
    std::unique_lock<std::shared_mutex> lock(m_timeManager->TimerRWMutex);
    auto it = m_timeManager->m_timers.find(shared_from_this());
    if(it == m_timeManager->m_timers.end()) return;
    Timer::ptr temp = (*it);
    m_timeManager->m_timers.erase(it);
	// 这里没写完啊
	temp->m_gap = gap;
	if(from_now){
		temp->m_next = sylar::GetCurrentMS()+gap;
	}else{
		temp->m_next = temp->m_next-gap;
	}
	// 这里需要将其再加入到定时器队列里面
	m_timeManager->m_timers.insert(temp);
}
Timer::Timer(uint64_t ms){
	m_gap = ms;
	m_next = sylar::GetCurrentMS()+ms;
}

Timer::Timer(uint64_t next,std::function<void()> cb){
    m_next = next; // 下一个时间节点
	m_cb = cb;// 运行到这里的时候进行与行这个函数
    // 为的就是将其设置为需要运行时间节点
}

uint64_t Timer::GetNextMs()const{
	return m_next;
}

TimerManager::TimerManager(){}

Timer::ptr TimerManager::addTimer(uint64_t ms, std::function<void()> cb, bool recurring){
	// 首先创建定时器，然后加入进入
	Timer::ptr timer(new Timer(ms, cb, recurring, this));
	std::unique_lock<std::shared_mutex> lock(TimerRWMutex);
	m_timers.insert(timer);
	return timer;
}

void TimerManager::addTimer(Timer::ptr timer){
	std::unique_lock<std::shared_mutex> lock(TimerRWMutex);
	m_timers.insert(timer);
}


static void onTimer(std::weak_ptr<void> weak_cond, std::function<void()>cb
				){
		// 条件定时器，在规定的事件进行检查weak_cond存不存在。
		// 存在就说明现在条件成熟，可以进行触发
		std::shared_ptr<void> temp = weak_cond.lock();
		if(temp){
				cb();
		}
}


// 这里需要对condition进行加入了，条件做成函数

Timer::ptr TimerManager::addCondtionTimer(uint64_t ms, std::function<void()> cb,
				std::weak_ptr<void> weak_cond,
				bool recurring
				){
		return addTimer(ms, std::bind(&onTimer, weak_cond, cb),recurring);
}

uint64_t TimerManager::GetNextTimerMs(){
	// 这里将拿到下一次执行的时间
	// 后面这里将结合配置系统进行等待
	if(m_timers.size()){
		std::shared_lock<std::shared_mutex> lock(TimerRWMutex);
		if(((*m_timers.begin())->GetNextMs()) <= sylar::GetCurrentMS()){
			return 0;
		}else{
			return (*m_timers.begin())->GetNextMs()-sylar::GetCurrentMS();
		}
	}
	return 3000;// 固定3秒停歇;
}

void TimerManager::GetExpiredCb(std::vector<std::function<void()>>& obs){
	// 将当前小于目前事件的定时的时间全部拿出来
		Timer::ptr now_timer(new Timer);
		std::unique_lock<std::shared_mutex> lock(TimerRWMutex);
		auto it = m_timers.lower_bound(now_timer);
		// SYLAR_LOG_INFO(SYLAR_LOG_NAME("system"))<<"begin timer"<<(*m_timers.begin())->GetNextMs()
		// 										<<"\t"<<"now_timer"<<now_timer->GetNextMs();
		// lock.unlock();
		if(it != m_timers.end() and (*it)->GetNextMs() == now_timer->GetNextMs()){
			it++;
		}
		std::vector<Timer::ptr> temp;
		temp.insert(temp.begin(), m_timers.begin(),it);
		// std::unique_lock<std::shared_mutex> lock2(TimerRWMutex);
		m_timers.erase(m_timers.begin(), it);
		lock.unlock();
		for(auto item:temp){
			obs.push_back(item->m_cb);
			if(item->m_recurring){
				// 如果说现在是循环的话，重置
				item->m_next = sylar::GetCurrentMS()+item->m_gap;
				addTimer(item);
			}else{
				item->m_cb = nullptr;
			}
		}
}

bool TimerManager::hasTimer()const{
	return !(m_timers.size() == 0);
}

}
