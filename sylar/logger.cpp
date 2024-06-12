

#include"sylar/logger.hpp"


// 日志级别与字符串之间的转化
sylar::LogLevel::Level sylar::LogLevel::fromString(const std::string& str){
    #define XX(level, v)\
            if (str == #v){\
                return LogLevel::Level::level;\
            }
            XX(DEBUG,DEBUG);
            XX(INFO,INFO);
            XX(WARN,WARN);
            XX(ERROR,ERROR);
            XX(FATAL, FATAL);

            XX(DEBUG,debug);
            XX(INFO,info);
            XX(WARN,warn);
            XX(ERROR,error);
            XX(FATAL, fatal);
            return LogLevel::Level::UNKNOW;
    #undef XX
}

std::string sylar::LogLevel::ToString(const LogLevel::Level &level)
{
    switch (level)
    {
    case LogLevel::Level::DEBUG:
        return "DEBUG";
        break;
    case LogLevel::Level::INFO:
        return "INFO";
        break;
    case LogLevel::Level::WARN:
        return "WARN";
        break;
    case LogLevel::Level::FATAL:
        return "FATAL";
        break;
    case LogLevel::Level::ERROR:
        return "ERROR";
    default:
        return "UNKNOW";
        break;
    }
}

// 日志器的构造：输入日志器的名称
sylar::Logger::Logger(std::string name):LoggerName(name){
    m_formatter = sylar::Formatter::ptr(new sylar::Formatter);
    // 初始化日志器的level
    
}
// 输出地的添加
void sylar::Logger::addAppender(std::shared_ptr<Appender> appender){
    // 写锁
    // 在这里进行判断
    if(appender->getFormatter().get() == nullptr){
        // 跟随日志
        appender->setFormatter(this->getmFormatter());
    }
    std::unique_lock<std::shared_mutex> lock(mutex_);
    appenderVec.push_back(appender);
}

// 输出地的删除
void sylar::Logger::delAppender(std::shared_ptr<Appender> appender){
    for(auto it=appenderVec.begin();it!=appenderVec.end();it++){
        if ((*it) == appender){
            std::unique_lock<std::shared_mutex> lock(mutex_);
            appenderVec.erase(it);
            break;
        }
    }
}

// 对日志器进行日志的结果输出
void sylar::Logger::log(LogLevel::Level level, LoggerEvent::ptr event){
    // 这里还要判断一下，如果说level小于log设置的级别，那就别输出了
    if(level<LoggerLevelLogger) return;
    // 对输出地进行遍历
    auto self = shared_from_this();

    for(auto&it:appenderVec){
        // std::cout<<it.get()<<std::endl;
        // it->log(self, event, level); // 将数据传入输出地这里的level是日志输入口的级别
        it->log(self, event, level); // 将数据传入输出地这里的level是日志输入口的级别
    }
}

// 日志器级别的输出
void sylar::Logger::debug(LoggerEvent::ptr event){
    this->log(LogLevel::Level::DEBUG, event);
}

void sylar::Logger::info(LoggerEvent::ptr event){

    this->log(LogLevel::Level::INFO, event);
}
void sylar::Logger::warn(LoggerEvent::ptr event){
    // if(event->getlevel()>=LogLevel::WARN)
    this->log(LogLevel::Level::WARN, event);
}
void sylar::Logger::error(LoggerEvent::ptr event){
    // if(event->getlevel()>=LogLevel::ERROR)
    this->log(LogLevel::Level::ERROR, event);
}
void sylar::Logger::fatal(LoggerEvent::ptr event){
    // if(event->getlevel()>=LogLevel::FATAL)
    this->log(LogLevel::Level::FATAL, event);
}

std::string sylar::Logger::getDateformatter(){
    // std::shared_lock<std::shared_mutex> lock(mutex_);
    return m_formatter->gettimeformatter();
    }



// 日志管理器
sylar::Logger::ptr sylar::LoggerManager::GetLogger(const std::string& name){
    auto it = maps.find(name);
    if (it!=maps.end()){
        return maps[name];
    }
    Logger::ptr root = maps["root"];
    // 否则就创建:我知道了，如果说不存在就创建他，但是他的结果是和root日志是一样的。
    Logger::ptr logger(new Logger(name));

    logger->setLoggerLevel(root->getLevel());
    logger->setFormatter(logger->getmFormatter());
    logger->addAppender(sylar::Appender::ptr(new sylar::stdOutAppender));
    // 然后所有的配置均按照默认的root进行操作
    maps[name] = logger;
    logger->m_root = logger; // root就是自己这种的。
    // std::cout<<"进行创建logger"<<std::endl;
    return logger;
}



// 格式器
sylar::Formatter::Formatter(const std::string& pattern):m_pattern(pattern){
    parsePattern(); // 进行格式的解析
}

// 这里也就是说将小样对应的东西进行对应的小样函数的开辟进行工作 
std::shared_ptr<sylar::Formatter::formatterItem> sylar::Formatter::getItemfunction(const char& itemC){
        switch (itemC)
        {
            // 时间
        case 'd':return std::shared_ptr<Formatter::formatterItem>(new DateTimeFormatterItem);
            break;
            // 级别
        case 'p':return std::shared_ptr<Formatter::formatterItem>(new LevelFormatterItem);
            break;
            // 程序启动时间
        case 'r':return std::shared_ptr<Formatter::formatterItem>(new ElapseFormatterItem);
            break;
            // 线程id
        case 't':return std::shared_ptr<Formatter::formatterItem>(new ThreadIDFormatterItem);
            break;
            // 即使多种信息组合:组合技
        case 'l':return std::shared_ptr<Formatter::formatterItem>(new manymsgFormatterItem);
            break;
            // 日志器的名字
        case 'c':return std::shared_ptr<Formatter::formatterItem>(new LoggerNameFormatterItem);
            break;
            // 函数名字
        case 'M':return std::shared_ptr<Formatter::formatterItem>(new funcNameFormatterItem);
            break;
            // 所在文件名字
        case 'F':return std::shared_ptr<Formatter::formatterItem>(new FileNameFormatterItem);
            break;
            // 所在行的名字
        case 'L':return std::shared_ptr<Formatter::formatterItem>(new LineFormatterItem);
            break;
            // 信息
        case 'm':return std::shared_ptr<Formatter::formatterItem>(new MessageFormatterItem);
            break;
            // 新行
        case 'n':return std::shared_ptr<Formatter::formatterItem>(new NewLineFormatterItem);
            break;
        case 'f': return std::shared_ptr<Formatter::formatterItem>(new FiberIDFormatterItem);
        case 'T': return std::shared_ptr<Formatter::formatterItem>(new ThreadNameFormatterItem);
        default:
            return nullptr;
        }
}

void sylar::Formatter::parsePattern(){
    /*
    // 算法的原理
    // 如果是遇到%标记的东西就使用格式小样的类输出
    // 如果是nullptr的话，就输出temp
    */
    size_t i = 0;
    while(i<m_pattern.size()){
        if(m_pattern[i]!='%'){
            std::string temp="";
            temp.push_back(m_pattern[i]);
            logmsg.push_back(std::make_pair(temp,nullptr));
            i++;
        }else{
            // 遇到了%打头
            if(i+1>=m_pattern.size()){
                // 遇到%打头但是，出现了%结尾的情况。处理办法，仍然就是将其结果保存。
                logmsg.push_back(std::make_pair("%",nullptr));
                i++;
            }else{
                std::string temp;
                temp.push_back(m_pattern[i]);
                temp.push_back(m_pattern[i+1]);
                logmsg.push_back(std::make_pair(temp,getItemfunction(m_pattern[i+1])));

                if(m_pattern[i+1] == 'd' and i+2<m_pattern.size() and m_pattern[i+2] == '{'){
                    size_t pos = m_pattern.find("}");// {1234}
                    timeformater = m_pattern.substr(i+3,pos-i-3);
                    i = pos+1;
                }else{
                    i+=2; 
                }
            }
        }
    }
}

// 格式器对信息进行组装，然后进行输出
std::string sylar::Formatter::log(std::shared_ptr<Logger>  logger, LoggerEvent::ptr event, LogLevel::Level level){
    // 按照格式组装字符串然后返回
    std::string ans;
    std::stringstream ss;

    for(const auto& it:logmsg){    
        if (it.second)
        {
            it.second->format(ss,logger, event, level);
        }else{
            ss<<it.first;
        }
    }
    return ss.str();
} // 组装成字符串之后进行返回

// 输出地的初始化
void sylar::stdOutAppender::log(Logger::ptr logger, LoggerEvent::ptr event, LogLevel::Level level){
    std::unique_lock<std::shared_mutex> lock(mutex_Appender);
    if(level>=m_level){
        std::cout<<formatter->log(logger, event, level);
    }
}


void sylar::fileAppender::log(Logger::ptr logger, LoggerEvent::ptr event, LogLevel::Level level){
    
    std::unique_lock<std::shared_mutex> lock(mutex_Appender);
    // 加上写锁
    if(level>=m_level){
        // 这里就是说判断一下，如果存在文件就进行文件的输入信息，
        // 如果不存在，就创建文件
        file<<formatter->log(logger, event, level);
        file.flush();
    }
}