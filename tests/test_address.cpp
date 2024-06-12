
#include"sylar/sylar.hpp"



// void test(){
//     std::vector<sylar::Address::ptr> addrs;
    
//     SYLAR_LOG_INFO_ROOT<<"begin...";
//     bool v = sylar::Address::Lookup(addrs, "www.baidu.com", AF_INET);

//     SYLAR_LOG_INFO_ROOT<<"end";
//     if(!v){
//         SYLAR_LOG_ERROR_ROOT << "lookup fail";
//         return;
//     }

//     for(size_t i = 0; i < addrs.size(); ++i) {
//         SYLAR_LOG_INFO_ROOT << i << " - " << addrs[i]->toString();
//     }
// }


// void test_ipv4() {
//     //auto addr = sylar::IPAddress::Create("www.sylar.top");
//     auto addr = sylar::IPAddress::Create("127.0.0.8");
//     if(addr) {
//         SYLAR_LOG_ERROR_ROOT << addr->toString();
//     }
// }


// vector<int> fds

// void bind(){
//     sylar::IOmanager iom(3);
//     iom.start();
//     auto addr = sylar::Address::LookupAny("127.0.0.0[:8033]");
//     std::vector<sylar::Address::ptr> addrs;
//     addrs.push_back(addr);
//     int sfd = ::socket(AF_INET, SOCK_STREAM, 0);
//     iom.addEvent() 


// }








int main(){
    // test();
    // test_ipv4();
    return 0;
}