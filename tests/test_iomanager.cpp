#include"sylar/sylar.hpp"
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>

// 让其进行sock服务器的监听事件

// 现在需要建立一个监听的事件

void requset_callback(){
    // 我在这里输出一下
    SYLAR_LOG_INFO_ROOT<<"test listen...";
}

void sock_test(){
    sylar::IOmanager iom(10);

    int lfd = socket(AF_INET,SOCK_STREAM,0);
    if(lfd == -1){
        SYLAR_LOG_ERROR_ROOT<<"Create sock fd error...";
    }
    // 端口复用
	int opt = 1;
	int ret = setsockopt(lfd, SOCK_STREAM, SO_REUSEPORT, &opt, sizeof(opt));
	if (ret == -1) {
		perror("setsockopt error");
		return;
	}

    struct sockaddr_in serv;
	serv.sin_addr.s_addr = htonl(INADDR_ANY);
	serv.sin_port = htons(8080);
	serv.sin_family = AF_INET;
    bind(lfd, (struct sockaddr*)&serv, sizeof(serv));
	listen(lfd,128);
    // 增加监听事件如果链接了就运行下面的代码
    iom.addEvent(lfd, requset_callback, sylar::IOmanager::READ);
    sylar::Timer::ptr ob;
    ob = iom.addTimer(2000,[&ob](){
        static int i = 0;
        i++;
        SYLAR_LOG_INFO_ROOT<<"test timer...i="<<i;
        if(i == 4){
            ob->reset(1000);
        }
        if(i == 8){
            ob->reset(2000);
        }
        if(i == 12){
            ob->cancel();
        }
    }, true);
    iom.start();
    // sleep(10);
    // iom.stop();
}


int main(){
    sock_test();
    return 0;
}