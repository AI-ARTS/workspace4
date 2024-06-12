#include "server.hpp"
#include "config.hpp"
#include "logger.hpp"

namespace sylar {

static sylar::configvar<uint64_t>::ptr g_tcp_server_read_timeout =
    sylar::config::Lookup("tcp_server.read_timeout", (uint64_t)(60 * 1000 * 2),
            "tcp server read timeout");

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

TcpServer::TcpServer(sylar::IOmanager* worker,
                    sylar::IOmanager* io_worker,
                    sylar::IOmanager* accept_worker)
    :m_worker(worker)
    ,m_ioWorker(io_worker)
    ,m_acceptWorker(accept_worker)
    ,m_recvTimeout(g_tcp_server_read_timeout->getValue())
    ,m_name("sylar/1.0.0")
    ,m_isStop(true) {
}

TcpServer::~TcpServer() {
    for(auto& i : m_socks) {
        i->close();
    }
    m_socks.clear();
}

void TcpServer::setConf(const TcpServerConf& v) {
    m_conf.reset(new TcpServerConf(v));
}

bool TcpServer::bind(sylar::Address::ptr addr, bool ssl) {
    std::vector<Address::ptr> addrs;
    std::vector<Address::ptr> fails;
    addrs.push_back(addr);
    return bind(addrs, fails, ssl);
}

bool TcpServer::bind(const std::vector<Address::ptr>& addrs
                        ,std::vector<Address::ptr>& fails
                        ,bool ssl) {
    m_ssl = ssl;
    for(auto& addr : addrs) {
        Socket::ptr sock = ssl ? SSLSocket::CreateTCP(addr) : Socket::CreateTCP(addr);
        if(!sock->bind(addr)) {
            SYLAR_LOG_ERROR(g_logger) << "bind fail errno="
                << errno << " errstr=" << strerror(errno)
                << " addr=[" << addr->toString() << "]";
            fails.push_back(addr);
            continue;
        }
        if(!sock->listen()) {
            SYLAR_LOG_ERROR(g_logger) << "listen fail errno="
                << errno << " errstr=" << strerror(errno)
                << " addr=[" << addr->toString() << "]";
            fails.push_back(addr);
            continue;
        }else{
            SYLAR_LOG_ERROR(g_logger) << "listen successful="<<"["<<addr->toString()<<"]";
        }
        m_socks.push_back(sock);
    }

    if(!fails.empty()) {
        m_socks.clear();
        return false;
    }

    for(auto& i : m_socks) {
        SYLAR_LOG_INFO(g_logger) << "type=" << m_type
            << " name=" << m_name
            << " ssl=" << m_ssl
            << " server bind success: " << *i;
    }
    return true;
}

void TcpServer::startAccept(Socket::ptr sock) {
    while(!m_isStop) {
        // SYLAR_LOG_INFO(g_logger)<<"in accept socket...";
        Socket::ptr client = sock->accept();
        if(client) {
            // SYLAR_LOG_INFO(g_logger)<<"accept client";
            client->setRecvTimeout(m_recvTimeout);
            m_ioWorker->schedule(std::bind(&TcpServer::handleClient,
                        shared_from_this(), client));
        } else {
            // SYLAR_LOG_ERROR(g_logger) << "accept errno=" << errno
            //     << " errstr=" << strerror(errno);
        }
    }
}

bool TcpServer::start() {
    if(!m_isStop) {
        return true;
    }
    m_isStop = false;
    for(auto& sock : m_socks) {
        m_acceptWorker->schedule(std::bind(&TcpServer::startAccept,
                    shared_from_this(), sock));
    }
    return true;
}

void TcpServer::stop() {
    m_isStop = true;
    auto self = shared_from_this();
    m_acceptWorker->schedule([this, self]() {
        for(auto& sock : m_socks) {
            sock->cancelAll();
            sock->close();
        }
        m_socks.clear();
    });
}

// 重写这个函数接收到链接的时候进行的操作
void TcpServer::handleClient(Socket::ptr client) {
    // SYLAR_LOG_INFO(g_logger) << "handleClient: " << *client;
    // 对齐进行读写的轮训
    // 先读。然后发{回去东西
    char buff[4096]{0};
    int count = client->recv(buff, sizeof(buff), 0);
    if(count == -1){
        // SYLAR_LOG_ERROR(g_logger)<<"read data error...";
    }else{
        // SYLAR_LOG_INFO(g_logger)<<"read successful: "<<buff;
    }
    char sendata[] = "hello...\r\n";
    count = client->send(sendata, sizeof(sendata), 0);
    if(count == -1){
        // SYLAR_LOG_ERROR(g_logger)<<"send data error...";
    }else{
        // SYLAR_LOG_INFO(g_logger)<<"send data successful...";
    }
}

bool TcpServer::loadCertificates(const std::string& cert_file, const std::string& key_file) {
    for(auto& i : m_socks) {
        auto ssl_socket = std::dynamic_pointer_cast<SSLSocket>(i);
        if(ssl_socket) {
            if(!ssl_socket->loadCertificates(cert_file, key_file)) {
                return false;
            }
        }
    }
    return true;
}

std::string TcpServer::toString(const std::string& prefix) {
    std::stringstream ss;
    ss << prefix << "[type=" << m_type
       << " name=" << m_name << " ssl=" << m_ssl
       << " worker=" << (m_worker ? m_worker->GetName() : "")
       << " accept=" << (m_acceptWorker ? m_acceptWorker->GetName() : "")
       << " recv_timeout=" << m_recvTimeout << "]" << std::endl;
    std::string pfx = prefix.empty() ? "    " : prefix;
    for(auto& i : m_socks) {
        ss << pfx << pfx << *i << std::endl;
    }
    return ss.str();
}

}