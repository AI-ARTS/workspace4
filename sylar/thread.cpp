

#include"sylar/thread.hpp"

namespace sylar{
     pid_t getThreadid();
static thread_local Thread* t_thread = nullptr;

static thread_local std::string t_thread_name = "UNKNOW";

static std::mutex g_mutex;

Semaphore::Semaphore(u_int32_t count){
    // 初始化
    value = count;
}

Semaphore::~Semaphore(){
    
}

void Semaphore::wait() {
    while (true) {
        std::unique_lock<std::mutex> lock(g_mutex);
        condition.wait(lock,[this](){return this->value>0;});
        this->value--;
        return;
    }
}

void Semaphore::notify(){
    // 进行放置
    this->value++;
    condition.notify_one();
}



Thread* Thread::GetThis(){
    return t_thread;
}

const std::string& Thread::GetName(){
    return t_thread_name;
}


void Thread::setName(const std::string& name){
    if(t_thread){
        t_thread->m_name = name;
    }
    t_thread_name = name;
}

Thread::Thread(
        const std::string& name,
        std::function<void()> cb
    ){
        m_cb = cb;
        m_name = name;
        m_thread = std::thread(run,this);
        // maps.emplace(name, run); // 开始运行run。等到通知时再进行接下来的
        _semaphore.wait();
    }

Thread::~Thread(){
    if (m_thread.joinable()) {
        m_thread.detach();
    }
}

void Thread::join(){
    m_thread.join();
} 

void* Thread::run(void* arg){
    Thread* thread = (Thread*)arg;
    t_thread = thread; // 没个线程记录一下自己封装后的线程的指针
    if(thread->getName().size()){
        t_thread_name = thread->getName(); // 传入名字
    }
    thread->m_id = sylar::getThreadid(); // 库中标识符
    // t_thread_name = thread->m_name;
    // pthread_setname_np(pthread_self(),thread->m_name.substr(0,15).c_str()); 
    std::function<void()> cb;
    cb.swap(thread->m_cb); // 拿到实际的任务
    thread->_semaphore.notify();
    cb();
    return nullptr;
    // return 0;
}


}