
#include"sylar/sylar.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <chrono>

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_sleep() {
    sylar::IOmanager iom(10);
    iom.schedule([](){
        sleep(2);
        SYLAR_LOG_INFO(g_logger) << "sleep 2";
    });

    iom.schedule([](){
        sleep(3);
        SYLAR_LOG_INFO(g_logger) << "sleep 3";
    });
    iom.start();
    SYLAR_LOG_INFO(g_logger) << "test_sleep";
    std::this_thread::sleep_for(std::chrono::seconds(10000));
}

// void test_sock() {
//     SYLAR_LOG_INFO_ROOT<<"in sock...";
//     int sock = socket(AF_INET, SOCK_STREAM, 0);

//     sockaddr_in addr;
//     memset(&addr, 0, sizeof(addr));
//     addr.sin_family = AF_INET;
//     addr.sin_port = htons(80);
//     inet_pton(AF_INET, "110.242.68.66", &addr.sin_addr.s_addr);

//     SYLAR_LOG_INFO(g_logger) << "begin connect";
//     int rt = connect(sock, (const sockaddr*)&addr, sizeof(addr));
//     SYLAR_LOG_INFO(g_logger) << "connect rt=" << rt << " errno=" << errno;

//     if(rt) {
//         return;
//     }

//     const char data[] = "GET / HTTP/1.0\r\n\r\n";
//     rt = send(sock, data, sizeof(data), 0);
//     SYLAR_LOG_INFO(g_logger) << "send rt=" << rt << " errno=" << errno;

//     if(rt <= 0) {
//         return;
//     }

//     std::string buff;
//     buff.resize(4096);

//     rt = recv(sock, &buff[0], buff.size(), 0);
//     SYLAR_LOG_INFO(g_logger) << "recv rt=" << rt << " errno=" << errno;

//     if(rt <= 0) {
//         return;
//     }

//     buff.resize(rt);
//     SYLAR_LOG_INFO(g_logger) << buff;
// }


void test_accept(){
    struct sockaddr_in sin;
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(lfd, F_SETFL, O_NONBLOCK);                //将socket设为非阻塞

    memset(&sin, 0, sizeof(sin));               //bzero(&sin, sizeof(sin))
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(8033);
    bind(lfd, (struct sockaddr *)&sin, sizeof(sin));
    listen(lfd, 20);
    int rt = 0;

    do{
        // sleep(2);
        rt = accept(lfd, nullptr, nullptr);
    }while(rt ==-1 and (errno ==EAGAIN or errno==EWOULDBLOCK) );
    
    if(rt == -1){
            SYLAR_LOG_ERROR(g_logger) << "accept errno=" << errno
        << " errstr=" << strerror(errno);
    }
    // std::cout<<rt<<std::endl;
}


int main(int argc, char** argv) {
    
    test_accept();
    // std::cout<<sylar::is_hook_enable()<<std::endl;
    // test_sleep();
    // sylar::IOmanager iom(3);
    // iom.schedule(test_accept);
    // iom.start();
    // // sleep(2);
    // iom.stop();
    return 0;
}