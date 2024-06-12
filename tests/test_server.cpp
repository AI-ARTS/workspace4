
#include"sylar/sylar.hpp"
#include"iomanager.hpp"
#include"server.hpp"


void run(){
    sylar::IOmanager iom(6);
    iom.start();
    auto addr = sylar::Address::LookupAny("127.0.0.0[:8033]");
    std::vector<sylar::Address::ptr> addrs;
    addrs.push_back(addr);

    sylar::TcpServer::ptr tcp_server(new sylar::TcpServer(&iom, &iom, &iom));
    std::vector<sylar::Address::ptr> fails;
    while(!tcp_server->bind(addrs, fails)) {
        sleep(2);
    }
    tcp_server->start();
    const int time_ = 1000;
    while(true){
        sleep(time_);
    }
}


int main(){
    run();
    return 0;
}