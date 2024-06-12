
#pragma once

#include <memory>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>



namespace sylar{
class IPAddress;

class Address{
public:
    using ptr = std::shared_ptr<Address>;
    static Address::ptr Create(const sockaddr* addr, socklen_t addrlen);
    
    // 通过host返回所有的addres
    static bool Lookup(std::vector<Address::ptr>& result, const std::string& host,
        int family = AF_INET, int type = 0, int protocol=0);
    
    // 通过host返回任意一个
    static Address::ptr LookupAny(const std::string& host,
        int family = AF_INET, int type = 0, int protocol = 0);
    
    static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string& host,
        int family = AF_INET, int type = 0, int protocol = 0);

    virtual ~Address(){}
    
    // 返回协议簇
    int getFamily()const;
    // 返回一下结构体
    virtual const sockaddr* getAddr()const =0;

    virtual sockaddr* getAddr() = 0;
    // 返回其长度
    virtual socklen_t getAddrLen()const = 0;

    // 可读性输出地址
    virtual std::ostream& insert(std::ostream& os)const = 0;

    // 返回可读性字符串
    std::string toString() const;
    bool operator<(const Address& rhs) const;
    bool operator==(const Address& rhs) const;
    bool operator!=(const Address& rhs) const;
};


class IPAddress: public Address{
public:
    using ptr = std::shared_ptr<IPAddress>;
    static IPAddress::ptr Create(const char* address, uint16_t port=0);
    virtual uint32_t getPort() const=0;
    virtual void setPort(uint16_t v)=0;
    virtual ~IPAddress(){};
};


class IPv4Address: public IPAddress{
public:
    using ptr = std::shared_ptr<IPv4Address>;
    // 使用点分十进制创建IPv4地址
    static IPv4Address::ptr Create(const char* address, uint16_t port=0);
    // 使用sockaddr_in 进行初始化
    IPv4Address(const sockaddr_in& address);
    // 使用二进制地址构造
    IPv4Address(uint32_t address = INADDR_ANY, uint16_t port=0);
    const sockaddr* getAddr()const override;
    sockaddr* getAddr()override;
    socklen_t getAddrLen()const override;
    std::ostream& insert(std::ostream& os)const override;
    uint32_t getPort()const override;
    void setPort(uint16_t v) override;
    ~IPv4Address(){}
private:
    sockaddr_in m_addr;
};


class IPv6Address : public IPAddress {
public:
    using ptr = std::shared_ptr<IPv6Address>;
    static IPv6Address::ptr Create(const char* address, uint16_t port = 0);
    IPv6Address();
    IPv6Address(const sockaddr_in6& address);
    IPv6Address(const uint8_t address[16], uint16_t port = 0);
    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;

    uint32_t getPort() const override;
    void setPort(uint16_t v) override;
    
    // 可读性输出地址
    ~IPv6Address(){}
private:
    sockaddr_in6 m_addr;
};


}