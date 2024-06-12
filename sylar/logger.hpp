
#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdarg.h>
#include <map>
#include<iostream>
#include<functional>
#include<time.h>
#include<tuple>
#include<thread>
#include<unordered_map>
#include<chrono>
#include<iomanip>
#include"singleton.hpp"
#include<yaml-cpp/yaml.h>
#include<mutex>
#include<shared_mutex>
#include<filesystem>
#include"thread.hpp"


namespace sylar{
    uint64_t getFiberId();
    pid_t getThreadid();
}
// 进行宏的构建

#define SYLAR_LEVEL(logger, level) \
    if(logger->getLevel()<=level) \
        sylar::LoggerWarp(sylar::LoggerEvent::ptr(new sylar::LoggerEvent(logger, level ,__FILE__,__LINE__,sylar::getThreadid(),sylar::getFiberId(),\
        sylar::Thread::GetName() ,__FUNCTION__))).getSS()

#define SYLAR_LOG_DEBUG(logger) SYLAR_LEVEL(logger, sylar::LogLevel::DEBUG)
#define SYLAR_LOG_INFO(logger) SYLAR_LEVEL(logger, sylar::LogLevel::INFO)
#define SYLAR_LOG_WARN(logger) SYLAR_LEVEL(logger, sylar::LogLevel::WARN)
#define SYLAR_LOG_ERROR(logger) SYLAR_LEVEL(logger, sylar::LogLevel::ERROR)
#define SYLAR_LOG_FATAL(logger) SYLAR_LEVEL(logger, sylar::LogLevel::FATAL)


#define SYLAR_LOG_ROOT() sylar::logMgr::GetInstance()->getRoot()
#define SYLAR_LOG_NAME(name) sylar::logMgr::GetInstance()->GetLogger(name)


#define SYLAR_LOG_DEBUG_ROOT SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT())
#define SYLAR_LOG_INFO_ROOT SYLAR_LOG_INFO(SYLAR_LOG_ROOT())
#define SYLAR_LOG_WRAN_ROOT SYLAR_LOG_WARN(SYLAR_LOG_ROOT())
#define SYLAR_LOG_ERROR_ROOT SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())
#define SYLAR_LOG_FATAL_ROOT SYLAR_LOG_FATAL(SYLAR_LOG_ROOT())



struct loggerAppenderPakage{
    int type = 1; // 1 std, 2 file
    int level = 0;
    std::string formatter;
    std::string file;
};


// 注意是类的定义顺序和结构 
namespace sylar
{
    class LogLevel;
    // 日志系统引擎
    class Logger;
    // 日志管理者
    class LoggerManager;
    // 日志记录者，在现场记录
    class LoggerEvent;
    // 日志输出地
    class Appender;
    // 继承虚函数输出地appender
    class stdOutAppender;
    // 文件输出地
    class fileAppender;
    // 格式器
    class Formatter;
    // 输入信息格式器

    class MessageFormatterItem;    
    // Level格式输出器
    class LevelFormatterItem;
    // 程序启动格式输出器
    class ElapseFormatterItem;
    // 返回线程ID
    class ThreadIDFormatterItem;
    // 协程号
    class FiberIDFormatterItem;
    class DateTimeFormatterItem;
    class FileNameFormatterItem;
    class LineFormatterItem;
    class StringFormatterItem;
    class NewLineFormatterItem; // 输出换行
    class TabFormatterItem; // 输出tab
    class ThreadNameFormatterItem; // 线程号
    class LoggerNameFormatterItem; // 日志名字
    class funcNameFormatterItem; // 现场函数名字
    class manymsgFormatterItem; // 包含了%c.%M(%F.%L)

    // 程序启动直接记录结果。
    static auto starttime_ = std::chrono::system_clock::now();
    static time_t process_starttime = std::chrono::system_clock::to_time_t(starttime_);
    
    using logMgr=sylar::Singleton<LoggerManager>;

    // 日志级别
    class LogLevel{
    public:
        enum Level{
            UNKNOW = 0,
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5
        };
        static LogLevel::Level fromString(const std::string& str);
        static std::string ToString(const LogLevel::Level& level);
    };

    // 现场记录者
    class LoggerEvent{
    public:
        using ptr = std::shared_ptr<LoggerEvent>;
                /**
                * @brief 构造函数
                * @param[in] logger 日志器
                * @param[in] level 日志级别
                * @param[in] file 文件名
                * @param[in] line 文件行号
                * @param[in] elapse 程序启动依赖的耗时(毫秒)
                * @param[in] thread_id 线程id
                * @param[in] fiber_id 协程id
                * @param[in] time 日志事件(秒)
                * @param[in] thread_name 线程名称
            */
        // 现场收集所需要的日志级别，然后日志器设置有日志级别，输出地存在自身的级别。层级过滤
        LoggerEvent(std::shared_ptr<Logger> logger,LogLevel::Level level,const std::string& filename 
        , uint32_t line, pid_t thread_id, uint64_t fiber_id, const std::string& threadname,
            std::string funcname
        )
        :m_logger(logger),
        m_filename(filename),
        m_line(line),
        m_elapse(process_starttime),
        m_thread_id(thread_id),
        m_fiber_id(fiber_id),
        m_threadname(threadname),
        m_funcname(funcname),
        m_level(level)
        {   
            // 现场直接收集时间事件
            auto current_t = std::chrono::system_clock::now();
            m_time = std::chrono::system_clock::to_time_t(current_t);
        }
        
        LoggerEvent(){}

        void record(std::shared_ptr<Logger> logger, const std::string& filename 
        , uint32_t line, uint64_t thread_id, uint32_t fiber_id, const std::string& threadname,
            std::string funcname){
            m_logger = logger;
            m_filename = filename;
            m_line = line;
            m_thread_id = thread_id;
            m_fiber_id = fiber_id;
            m_threadname = threadname;
            m_funcname = funcname;
            content.clear();
        }

        std::shared_ptr<Logger> getLogger()const{return m_logger;}
        std::string getfileName()const {return m_filename;}
        uint32_t getline()const {return m_line;}
        time_t getelapse()const {return m_elapse;}
        pid_t getthreadid()const {return m_thread_id;}
        uint32_t getfiberid()const {return m_fiber_id;}
        std::time_t gettime()const{return m_time;}
        std::string getthreadname()const{return m_threadname;}
        std::string getFunName()const{return m_funcname;}
        std::string getContent()const {return content;}
        LogLevel::Level getLevel()const{return m_level;}
        void inputMessage(std::string data){
            content = data;
        }
    private:
        // 对现场进行记录结果
        // 记录需要记录
        std::shared_ptr<Logger> m_logger;
        // LogLevel::Level m_level;
        std::string m_filename;
        uint32_t m_line;
        std::time_t m_elapse; // 启动时间戳
        pid_t m_thread_id;
        uint64_t m_fiber_id;
        // uint64_t time; // 时间戳
        std::time_t m_time;
        std::string m_threadname;
        std::string content = ""; // 输入内容
        std::string m_funcname;
        LogLevel::Level m_level;
    };


    // 输出地有什么，有级别，有格式。共有的
    // 输出地可自行设置自什么的输出格式
    class Appender{
    public:
        using ptr = std::shared_ptr<Appender>;
        virtual ~Appender(){}
        virtual loggerAppenderPakage toYamlString()=0;
        // 当然这是记录
        virtual void log(std::shared_ptr<Logger>, LoggerEvent::ptr, LogLevel::Level){};
        void setFormatter(std::shared_ptr<Formatter> formatter_){
            // 写配置
            std::unique_lock<std::shared_mutex> lock(mutex_Appender);
            formatter = formatter_;

            }
        std::shared_ptr<Formatter> getFormatter()const{
            // 读配置
            std::shared_lock<std::shared_mutex> lock(mutex_Appender);
            return formatter;
            }

        void setLevel(LogLevel::Level level){
            std::unique_lock<std::shared_mutex> lock(mutex_Appender);
            m_level = level;
            }
        LogLevel::Level getLevel()const{
            std::shared_lock<std::shared_mutex> lock(mutex_Appender);
            return m_level;
            }
    // 保护访问权限的私有变量
    protected:
        // 输出地有什么需要有自己的级别
        // 这里是统一输出地的格式
        LogLevel::Level m_level = LogLevel::DEBUG;
        std::shared_ptr<Formatter> formatter;
        mutable std::shared_mutex mutex_Appender;// 用于配置信息的读写锁
        // mutable std::mutex mutex_appender; // 互斥锁：用于输出信息的加锁
    };

        // 作用就是格式解析器，就是将格式进行解析，如何输出，然后进行数据的输出
    class Formatter{
    public:
        using ptr = std::shared_ptr<Formatter>;
        // 首先就是接受其格式
        Formatter(const std::string& pattern="[%d{%F %T}]-[%p][%r][%t][%c][%M][%F][%L][%f][%T] %m %n");
        // 根据格式器，现场记录者进行整个格式的解析，进行
        std::string log(std::shared_ptr<Logger> , LoggerEvent::ptr, LogLevel::Level); // 组装成字符串之后进行返回
        // 用于格式的解析工作
        void parsePattern();
        // 格式小样
        class formatterItem{
        public:
            using ptr = std::shared_ptr<formatterItem>;
            // 这里的默认字符串的作用是什么呢？
            formatterItem(const std::string& fmt=""){};
            virtual ~formatterItem(){};
            // 这个函数的作用就是传入现场信息和日志器的一些信息
            virtual void format(std::ostream& os, std::shared_ptr<Logger> logger,LoggerEvent::ptr event, LogLevel::Level level){}
        };
        std::string gettimeformatter()const{
            // 读锁
            std::shared_lock<std::shared_mutex> lock(mutex_formatter);
            return timeformater;
            }
        void settimeformatter(const std::string timeformat){
            // 写锁
            std::unique_lock<std::shared_mutex> lock(mutex_formatter);
            timeformater = timeformat;
            }
        std::string getPattern()const{
            // 读锁
            std::shared_lock<std::shared_mutex> lock(mutex_formatter);
            return m_pattern;
            }
        void setPattern(const std::string &pattern){
            // 写锁
            std::unique_lock<std::shared_mutex> lock(mutex_formatter);
            m_pattern = pattern;
            }
    private:
        // 这也要加个读写锁
        mutable std::shared_mutex mutex_formatter;
        // 用户输入的格式结果
        std::string m_pattern;
        // bool m_error = false; // 表示现在的格式器存不存错误是否成功
        std::vector<std::pair<std::string, formatterItem::ptr>> logmsg;
        // 第二种写法:这种写法的感悟，就是如果不想的在初始化的时候全部进行空间分配，可以写成函数，这样代码效率更高
        static std::shared_ptr<Formatter::formatterItem> getItemfunction(const char& itemC);
        std::string timeformater="%F %T"; // 这里记录的是时间的格式
    };


    // 这个继承可是知识点:必须加上syl
    class Logger:public std::enable_shared_from_this<Logger>{
    public:
    friend class sylar::LoggerManager;
        using ptr = std::shared_ptr<Logger>;
        
        Logger(std::string name="root");

        void addAppender(std::shared_ptr<Appender> appender);
        void delAppender(std::shared_ptr<Appender> appender);
        
        void log(LogLevel::Level ,LoggerEvent::ptr);
        void debug(LoggerEvent::ptr); // 对所有输出地进行级别的入口
        void info(LoggerEvent::ptr); 
        void warn(LoggerEvent::ptr); 
        void error(LoggerEvent::ptr); 
        void fatal(LoggerEvent::ptr); 
        LogLevel::Level getLevel(){ 
            std::shared_lock<std::shared_mutex> lock(mutex_);
            return LoggerLevelLogger;
            }
        void setLoggerLevel(const LogLevel::Level& loglevel){
            std::unique_lock<std::shared_mutex> lock(mutex_);
            LoggerLevelLogger =loglevel;
            }
        std::string getDateformatter();
        std::string getloggername()const {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            return LoggerName;
            }
        std::shared_ptr<Formatter> getmFormatter()const{
            std::shared_lock<std::shared_mutex> lock(mutex_);
            return m_formatter;
            }
        void setFormatter(std::shared_ptr<Formatter> mf){
            // std::unique_lock<std::shared_mutex> lock(mutex_);
            m_formatter = mf;
            }
        
        void setFormatter(const std::string& val){
            // std::unique_lock<std::shared_mutex> lock(mutex_);
            m_formatter.reset(new sylar::Formatter(val));
        }

        void clearAppenders(){
            // std::unique_lock<std::shared_mutex> lock(mutex_);
            appenderVec.clear();
        }
        std::vector<std::shared_ptr<Appender>> getAppenders()const{
            // std::shared_lock<std::shared_mutex> lock(mutex_);
            return appenderVec;
            }
        Logger::ptr m_root;

        std::string toYamlstring(){
            // 输出格式的时候统一都不要动了
            std::unique_lock<std::shared_mutex> lock(mutex_);
            // 将日志器的配置输出为yamlstring
            YAML::Node node;
            node["name"] = LoggerName;
            node["level"] = LogLevel::ToString(LoggerLevelLogger);
            if(!m_formatter->getPattern().empty()){
                node["formatter"] = m_formatter->getPattern();
            }
            for(auto& i:getAppenders()){
                loggerAppenderPakage lad;
                lad = i->toYamlString(); // 这里的事输出地自己的锁
                YAML::Node appender;
                if(lad.type == 1){
                    appender["type"] = "stdOutAppender";
                }else if(lad.type == 2){
                    appender["type"] = "fileAppender";
                }
                if (!lad.file.empty())
                {
                    appender["file"] = lad.file;
                }
                appender["level"] = LogLevel::ToString(LogLevel::Level(lad.level));
                if(!lad.formatter.empty()){
                    appender["formatter"] = lad.formatter;
                }
                node["appenders"].push_back(appender);
            }
            std::stringstream ss;
            ss<< node;
            return ss.str();
        }
        
    private:
        // 加锁
        mutable std::shared_mutex mutex_;
        mutable std::mutex mutex_unique;
        std::string LoggerName; // 此logger的名字
        std::vector<std::shared_ptr<Appender>> appenderVec; // 保存的是文件输出地
        // 日志器的日志级别
        LogLevel::Level LoggerLevelLogger=LogLevel::DEBUG;
        // 日志系统保存的格式器
        std::shared_ptr<Formatter> m_formatter; 
    };

    // 输出
    class stdOutAppender: public Appender{
    public:
        // 需要什么，需要现场记录
        void log(Logger::ptr, LoggerEvent::ptr, LogLevel::Level)override;
        loggerAppenderPakage toYamlString()override{
            // 由于是读配置。进行读配置的加锁
            std::shared_lock<std::shared_mutex> lock(mutex_Appender);
            loggerAppenderPakage lad;
            lad.type = 1;
            lad.level = m_level;
            if (formatter.get()){
                lad.formatter = formatter->getPattern();
            }
            return lad;
        }
    private:
        // 也是，何必进行其记录呢直接实现啊
    };

    // 继承的有锁
    class fileAppender: public Appender{
    public:

        loggerAppenderPakage toYamlString()override{

            std::shared_lock<std::shared_mutex> lock(mutex_Appender);
            // YAML::Node node;
            loggerAppenderPakage lad;
            lad.type = 2;
            lad.file = getfilePath();
            lad.level = m_level;
            if(formatter.get()){
                lad.formatter = formatter->getPattern();
            }
            return lad;
        }

        fileAppender(const std::string & file_):filePath(file_){
            // 寻找最后一个/然后进行分割
            // 这里的C++17的文件操作属实棒。
            try
            {
                int pos;
                for(size_t i=0;i<filePath.size();i++){
                    if(filePath[i] =='/') pos = i;
                }
                std::string directory_ = std::filesystem::absolute(filePath.substr(0,pos));
                if(!std::filesystem::exists(directory_)){
                    std::filesystem::create_directory(directory_);
                }
                file.open(filePath);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }
        }
        void log(Logger::ptr, LoggerEvent::ptr, LogLevel::Level) override;
        void reopen(){
            if(file.is_open()){
                file.close();
            }
            file.open(filePath);
        }
        std::string getfilePath()const{return filePath;}
    private:
        std::string filePath;
        std::ofstream file;
    };


    // 格式小样
    class FiberIDFormatterItem:public Formatter::formatterItem{
    public:
        FiberIDFormatterItem(){}
        using ptr = std::shared_ptr<FiberIDFormatterItem>;
        void format(std::ostream& os, Logger::ptr logger,LoggerEvent::ptr event, LogLevel::Level level)override{
            os<<event->getfiberid();
        }
    };
    class DateTimeFormatterItem:public Formatter::formatterItem{
    public:
        DateTimeFormatterItem(){}
        using ptr = std::shared_ptr<DateTimeFormatterItem>;
        void format(std::ostream& os, Logger::ptr logger,LoggerEvent::ptr event, LogLevel::Level level)override{
            // 对时间进行构造
            std::time_t current_t = event->gettime();

            auto result = std::put_time(std::localtime(&current_t), logger->getDateformatter().c_str());
            os<< result;
        }
    };
    class FileNameFormatterItem:public Formatter::formatterItem{
    public:
        FileNameFormatterItem(){}
        using ptr = std::shared_ptr<FileNameFormatterItem>;
        // 所在文件的地点
        void format(std::ostream& os, Logger::ptr logger,LoggerEvent::ptr event, LogLevel::Level level)override{
            os<<event->getfileName();
        }
    };
    class LineFormatterItem:public Formatter::formatterItem{
    public:
        LineFormatterItem(){}
        using ptr = std::shared_ptr<LineFormatterItem>;
        void format(std::ostream& os, Logger::ptr logger,LoggerEvent::ptr event, LogLevel::Level level)override{
            os<<event->getline();
        }
    };

    class NewLineFormatterItem:public Formatter::formatterItem{
    public:
        NewLineFormatterItem(){}
        using ptr = std::shared_ptr<NewLineFormatterItem>;
        void format(std::ostream& os, Logger::ptr logger,LoggerEvent::ptr event, LogLevel::Level level)override{
            os<<'\n';
        }
    }; // 输出换行

    class TabFormatterItem:public Formatter::formatterItem{
    public:
        TabFormatterItem(){}
        using ptr = std::shared_ptr<TabFormatterItem>;
        void format(std::ostream& os, Logger::ptr logger,LoggerEvent::ptr event, LogLevel::Level level)override{
            os<<'\t';
        }
    }; // 输出tab
    // 存在的是
    class ThreadNameFormatterItem:public Formatter::formatterItem{
    public:
        ThreadNameFormatterItem(){}
        using ptr = std::shared_ptr<ThreadNameFormatterItem>;
        void format(std::ostream& os, Logger::ptr logger,LoggerEvent::ptr event, LogLevel::Level level)override{
            os<<event->getthreadname();
        }
    }; // 线程号

    class LoggerNameFormatterItem:public Formatter::formatterItem{
    public:
        LoggerNameFormatterItem(){}
        using ptr = std::shared_ptr<LoggerNameFormatterItem>;
        void format(std::ostream& os, Logger::ptr logger,LoggerEvent::ptr event, LogLevel::Level level)override{
            os<<event->getLogger()->getloggername();
        }
    }; // 日志名字

    class funcNameFormatterItem:public Formatter::formatterItem{
    public:
        funcNameFormatterItem(){}
        using ptr = std::shared_ptr<funcNameFormatterItem>;
        void format(std::ostream& os, Logger::ptr logger,LoggerEvent::ptr event, LogLevel::Level level)override{
            os<<event->getFunName();
        }
    }; // 现场函数名字


    class manymsgFormatterItem:public Formatter::formatterItem{
    public:
        manymsgFormatterItem(){}
        using ptr = std::shared_ptr<manymsgFormatterItem>;
        void format(std::ostream& os, Logger::ptr logger,LoggerEvent::ptr event, LogLevel::Level level)override{
            // [MyClass.cpp:42 (void MyClass::myFunction())] [DEBUG] - Log message...
            os<<'['<<event->getfileName()<<":"<<event->getline()<<" ("<<event->getFunName()<<")]"<<"["<<level<<"] - "<<event->getContent();
        }
    }; 

    class LevelFormatterItem:public Formatter::formatterItem{
    public:
        LevelFormatterItem(){}
        using ptr = std::shared_ptr<LevelFormatterItem>;
        void format(std::ostream& os, Logger::ptr logger,LoggerEvent::ptr event, LogLevel::Level level)override{
            os<<LogLevel::ToString(level);
        }
    };
    class ElapseFormatterItem:public Formatter::formatterItem{
    public:
        ElapseFormatterItem(){}
        using ptr = std::shared_ptr<ElapseFormatterItem>;
        
        void format(std::ostream& os, Logger::ptr logger,LoggerEvent::ptr event, LogLevel::Level level)override{
            // std::time_t diff = event->gettime() - event->getelapse();
            double diff = difftime(event->gettime(), event->getelapse());
            os<<diff<<"s";
        }
    };

    class ThreadIDFormatterItem:public Formatter::formatterItem{
    public:
        ThreadIDFormatterItem(){}
        using ptr = std::shared_ptr<ThreadIDFormatterItem>;
        void format(std::ostream& os, Logger::ptr logger,LoggerEvent::ptr event, LogLevel::Level level)override{
            os<<event->getthreadid();
        }
    };
    class MessageFormatterItem:public Formatter::formatterItem{
    public:
        MessageFormatterItem(){}
        using ptr = std::shared_ptr<MessageFormatterItem>;
        void format(std::ostream& os, Logger::ptr logger,LoggerEvent::ptr event, LogLevel::Level level)override{
            os<<event->getContent();
        }
    };    

    class LoggerManager{
    public:
        // 这里进行的是
        LoggerManager(){
            m_root.reset(new Logger);
            m_root->addAppender(sylar::Appender::ptr(new stdOutAppender));
            maps[m_root->getloggername()] = m_root;
            init();
        }
        void init(){}
        Logger::ptr GetLogger(const std::string& name);
        
        std::string toYamlstring(){
            // 对每个日志器进行yaml的收集
            YAML::Node node;
            for (const auto&it:maps)
            {
                node[it.first] = it.second->toYamlstring();
                
            }
            std::stringstream ss;
            ss<<node;
            return ss.str();
        }


        Logger::ptr getRoot()const {
            return m_root;
        }
        std::unordered_map<std::string, Logger::ptr> getmaps()const{return maps;}
    private:
        std::unordered_map<std::string, Logger::ptr> maps;
        Logger::ptr m_root;
    };


    class LoggerWarp{
    public:
        LoggerWarp(LoggerEvent::ptr event){
            m_event = event;
        }
        
        LoggerEvent::ptr getEvent() const{
            return m_event;
        }
        std::ostream& getSS(){return os;}
        ~LoggerWarp(){

            m_event->inputMessage(os.str());
            // 进行m_event的loger的检查
            sylar::Logger::ptr loggers = m_event->getLogger();            
            m_event->getLogger()->log(m_event->getLevel(), m_event);
            
        }
    private:
        LoggerEvent::ptr m_event;
        std::stringstream  os;
    };
} // namespace sylar

#endif