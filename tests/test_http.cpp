
#include"sylar/http/http.hpp"
#include"sylar/sylar.hpp"


void test(){
	sylar::http::HttpRequest::ptr req(new sylar::http::HttpRequest);
	req->setHeader("host", "www.sylar.top");
	req->setBody("hello sylar");
	req->dump(std::cout)<<std::endl;
}


int main(){
	test();
	return 0;
}
