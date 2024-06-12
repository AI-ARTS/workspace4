

#include <iostream>
#include <boost/coroutine2/all.hpp>

using co_push_t = boost::coroutines2::coroutine<int>::push_type;
using co_pull_t = boost::coroutines2::coroutine<int>::pull_type;

void test2(co_push_t& yield2){
    for(int i=5;i<10;i++){
        yield2(i);
    }
}

void CoroutineFunction(co_push_t& yield)
{
    for (int i = 0; i < 5; ++i) {
        yield(i); // 将当前值推送给调用者
    }

    co_pull_t source2(test2);
    for(auto va: source2){
        std::cout<<va<<std::endl;
    }
}

int main()
{
    
    
    co_pull_t source(CoroutineFunction);
    for (auto value : source) {
        std::cout << value << std::endl; // 打印协程函数产生的值
    }
    return 0;
}