#include"sylar/sylar.hpp"


void test2(){
    SYLAR_LOG_INFO_ROOT<<"schedule..";
}




int main(){
    sylar::Schedule sch(5);
    sch.schedule(test2);
    sch.start();
    sch.schedule(test2);
    sch.schedule(test2);
    sch.stop();
    return 0;
}