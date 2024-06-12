
#include"sylar.hpp"

#include "sylar/socket.hpp"
#include "sylar/sylar.hpp"
#include "sylar/iomanager.hpp"

static sylar::Logger::ptr g_looger = SYLAR_LOG_ROOT();

void test_socket() {
    
    // 进行解析地址
    sylar::IPAddress::ptr addr = sylar::Address::LookupAnyIPAddress("www.baidu.com");
    if(addr) {
        SYLAR_LOG_INFO(g_looger) << "get address: " << addr->toString();
    } else {
        SYLAR_LOG_ERROR(g_looger) << "get address fail";
        return;
    }
    // 将建一个socket
    sylar::Socket::ptr sock = sylar::Socket::CreateTCPSocket();

    addr->setPort(80); // 设置端口
    SYLAR_LOG_INFO(g_looger) << "addr=" << addr->toString();
    // 进行连接
    if(!sock->connect(addr)) {
        SYLAR_LOG_ERROR(g_looger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        SYLAR_LOG_INFO(g_looger) << "connect " << addr->toString() << " connected";
    }

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    if(rt <= 0) {
        SYLAR_LOG_INFO(g_looger) << "send fail rt=" << rt;
        return;
    }else{
        SYLAR_LOG_INFO(SYLAR_LOG_NAME("system"))<<"send successful...";
    }

    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());

    if(rt <= 0) {
        SYLAR_LOG_INFO(g_looger) << "recv fail rt=" << rt;
        return;
    }

    buffs.resize(rt);
    SYLAR_LOG_INFO(g_looger) << buffs;
}

void test2() {
    sylar::IOmanager iom(3);
    iom.schedule(&test_socket);
    iom.start();
    
    static const int times = 10000;
    while(true){
        sleep(times);
    }
    // sleep(10);
    // iom.stop();
}


int main(int argc, char** argv) {   
    test2();
    // iom.schedule(&test2);
    return 0;
}