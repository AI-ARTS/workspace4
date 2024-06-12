#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include<sys/syscall.h>
#include<sys/types.h>
#include<unistd.h>
#include<thread>
#include<vector>
#include<string>
#include<execinfo.h>
#include<iostream>
#include<sstream>


// fiber->util
namespace sylar{
    uint64_t getFiberId();
    pid_t getThreadid();
    uint64_t GetCurrentMS();
    void Backtrace(
        std::vector<std::string>& bt,
        int size,
        int skip
    );
    std::string BacktraceToString(
        int size=64, int skip=2, const std::string& prefix=""
    );


class StringUtil {
public:
    static std::string Format(const char* fmt, ...);
    static std::string Formatv(const char* fmt, va_list ap);

    static std::string UrlEncode(const std::string& str, bool space_as_plus = true);
    static std::string UrlDecode(const std::string& str, bool space_as_plus = true);

    static std::string Trim(const std::string& str, const std::string& delimit = " \t\r\n");
    static std::string TrimLeft(const std::string& str, const std::string& delimit = " \t\r\n");
    static std::string TrimRight(const std::string& str, const std::string& delimit = " \t\r\n");


    static std::string WStringToString(const std::wstring& ws);
    static std::wstring StringToWString(const std::string& s);

};

}

#endif