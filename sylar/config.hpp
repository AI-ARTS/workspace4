
#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include<memory>
#include<string>
#include<boost/lexical_cast.hpp>
#include"sylar/logger.hpp"
#include<yaml-cpp/yaml.h>
#include<list>
#include<map>
#include<set>
#include<unordered_map>
#include<unordered_set>

namespace sylar{
    
    // 默认读取yaml的配置结果的代码
    class LogAppenderDefine{
    public:
        int type = 1; // 1 std, 2 file
        LogLevel::Level level = LogLevel::UNKNOW;
        std::string formatter;
        std::string file;
        bool operator==(const LogAppenderDefine& oth)const{
            return type == oth.type&&
                    level == oth.level&&
                    formatter == oth.formatter&&
                    file == oth.file;
        }
    };

    // 这是log器的结构
    class logDefine{
    public:
        std::string name;
        LogLevel::Level level = LogLevel::UNKNOW;
        std::string formatter;
        // 输出地的结果
        std::vector<LogAppenderDefine> appenders;

        bool operator==(const logDefine& oth) const{
            return name == oth.name&&
                    level == oth.level&&
                    formatter == oth.formatter&&
                    appenders == appenders;
        }
        bool operator<(const logDefine& oth)const{
            return name<oth.name;
        }
    };

    class ConfigVarBase{
    public:
        using ptr = std::shared_ptr<ConfigVarBase>;
        // 记录信息的名字和描述
        ConfigVarBase(const std::string& name,
                    const std::string& description = "")
                    :m_name(name),
                    m_description(description)
                    {
                        // 统一转换成小写
                        std::transform(m_name.begin(),
                            m_name.end(),
                            m_name.begin(),
                            ::tolower
                        );
                    }
        virtual ~ConfigVarBase(){}
        virtual std::string getName() const=0;// {return m_name;}
        virtual std::string getDescription()const=0;//{return m_description;}

        virtual std::string toString()=0;//{return "";}
        virtual bool fromString(const std::string& val)=0;//{ return true;}
        // 拿到信息的类型名字
        virtual std::string getTypename()const=0;//{return "";}
        protected:
            std::string m_name;
            std::string m_description;
    };


    // 转换的范型编程
    template<class F, class T>
    class LexicalCast{
        public:
        T operator()(const F& v){
            return boost::lexical_cast<T>(v);
        }
    };

    // 将字符串转换成vec
    template<class T>
    class LexicalCast<std::string, std::vector<T>>{
        public:
        std::vector<T> operator()(const std::string& v){
            YAML::Node node = YAML::Load(v);
            typename std::vector<T> ans;
            std::stringstream ss;
            for (size_t i = 0; i < node.size(); i++)
            {
                ss.str(""); // 这里清空一下
                ss<<node[i];
                // 学到了如何使用这个函数。学到了模板特化和typename的用法。
                ans.emplace_back(boost::lexical_cast<T>(ss.str()));
            }
            return ans;
        }
    };

    // 将字符串转换成list
    template<class T>
    class LexicalCast<std::string, std::list<T>>{
        public:
        std::list<T> operator()(const std::string& v){
            YAML::Node node = YAML::Load(v);
            typename std::list<T> ans;
            std::stringstream ss;
            for (size_t i = 0; i < node.size(); i++)
            {
                ss.str(""); // 这里清空一下
                ss<<node[i];
                // 学到了如何使用这个函数。学到了模板特化和typename的用法。
                ans.emplace_back(boost::lexical_cast<T>(ss.str()));
            }
            return ans;
        }
    };

    // 将字符串转成set
    template<class T>
    class LexicalCast<std::string, std::set<T>>{
    public:
        std::set<T> operator()(const std::string& str){
            typename std::set<T> ans;
            YAML::Node node = YAML::Load(str);
            if(node.IsSequence()){
                std::stringstream ss;
                for(size_t i = 0;i<node.size();i++){
                    ans.insert(boost::lexical_cast<T>(node[i]));
                }
            }
            return ans;
        }
    };

    // 将字符串转成unordered_set
    template<class T>
    class LexicalCast<std::string, std::unordered_set<T>>{
    public:
        std::unordered_set<T> operator()(const std::string& str){
            typename std::unordered_set<T> ans;
            YAML::Node node = YAML::Load(str);
            if(node.IsSequence()){
                std::stringstream ss;
                for(size_t i = 0;i<node.size();i++){
                    ans.insert(boost::lexical_cast<T>(node[i]));
                }
            }
            return ans;
        }
    };

    // 将字符串转成unordered_map
    template<class T>
    class LexicalCast<std::string, std::unordered_map<std::string, T>>{
    public:
        std::unordered_map<std::string, T> operator()(const std::string& str){
            YAML::Node node = YAML::Load(str);
            std::unordered_map<std::string, T> ans;
            std::stringstream ss;
            for(auto it = node.begin();it!=node.end();it++){
                ss.str("");
                ss<<it->first;
                ans[it->first.Scalar()] = boost::lexical_cast<T>(it->second);
            }
            return ans;
        }
    };
    
    // 将字符串转成map
    template<class T>
    class LexicalCast<std::string, std::map<std::string, T>>{
    public:
        std::map<std::string, T> operator()(const std::string& str){
            YAML::Node node = YAML::Load(str);
            std::map<std::string, T> ans;
            std::stringstream ss;
            for(auto it = node.begin();it!=node.end();it++){
                ss.str("");
                ss<<it->first;
                ans[it->first.Scalar()] = boost::lexical_cast<T>(it->second);
            }
            return ans;
        }
    };

    // 从vec转成字符串
    template<class T>
    class LexicalCast<std::vector<T>, std::string>{
    public:
        std::string operator()(const std::vector<T>& vec){
            YAML::Node node;
            for(auto&it:vec){
                node.push_back(YAML::Load(boost::lexical_cast<std::string>(it)));
            }
            std::stringstream ss;
            ss<<node;
            return ss.str();
        }
    };

    // 从list转换成string
    template<class T>
    class LexicalCast<std::list<T>, std::string>{
    public:
        std::string operator()(const std::list<T>& vec){
            YAML::Node node;
            for(auto&it:vec){
                node.push_back(YAML::Load(boost::lexical_cast<std::string>(it)));
            }
            std::stringstream ss;
            ss<<node;
            return ss.str();
        }
    };

    // 从set转成string
    template<class T>
    class LexicalCast<std::set<T>, std::string>{
    public:
        std::string operator()(const std::set<T>& vec){
            YAML::Node node;
            for(auto&it:vec){
                node.push_back(YAML::Load(boost::lexical_cast<std::string>(it)));
            }
            std::stringstream ss;
            ss<<node;
            return ss.str();
        }
    };

    // 从unordered_set转成string
    template<class T>
    class LexicalCast<std::unordered_set<T>, std::string>{
    public:
        std::string operator()(const std::unordered_set<T>& vec){
            YAML::Node node;
            for(auto&it:vec){
                node.push_back(YAML::Load(boost::lexical_cast<std::string>(it)));
            }
            std::stringstream ss;
            ss<<node;
            return ss.str();
        }
    };

    // 将map转成字符串
    template<class T>
    class LexicalCast<std::map<std::string, T>, std::string>{
    public:
        std::string operator()(const std::map<std::string, T>& maps){
            YAML::Node node(YAML::NodeType::Map);
            for(auto&it:maps){
                node[it.first] = boost::lexical_cast<T>(it.second);
            }
            std::stringstream ss;
            ss<<node;
            return ss.str();
        }
    };

    // 将unordered_map转成字符串
    template<class T>
    class LexicalCast<std::unordered_map<std::string, T>, std::string>{
    public:
        std::string operator()(const std::unordered_map<std::string, T>& maps){
            // 这里学到了
            YAML::Node node(YAML::NodeType::Map);
            for(auto&it:maps){
                node[it.first] = boost::lexical_cast<T>(it.second);
            }
            std::stringstream ss;
            ss<<node;
            return ss.str();
        }
    };

    // 根据不同的类型进行构建信息对象 信息对象包装器
    template<class T, class fromestr =LexicalCast<std::string, T>,
    class tostr = LexicalCast<T,std::string>>
    class configvar:public ConfigVarBase{
    public:
        using ptr = std::shared_ptr<configvar>;
        using value_change = std::function<void(const T& old_value, const T&new_value)>;
        configvar(
        const std::string& name,
        const T& default_value,
        const std::string& descriiption="" 
        ):ConfigVarBase(name, descriiption){
            std::unique_lock<std::shared_mutex> lock(mutex_);
            m_val = default_value;
        }
        ~configvar(){}
        // 将信息转换成字符串
        std::string toString()override{
            try
            {
                std::shared_lock<std::shared_mutex> lock(mutex_);
                return tostr()(m_val);
            }
            catch(const std::exception& e)
            {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())<<"configvar::tostring exception"
                << e.what()<<"convert:"<<typeid(m_val).name()<<"to string";
                // std::cerr << e.what() << '\n';
            }
            return "";
        }
        // 将字符串转化成信息，并返回转化的结果
        bool fromString(const std::string& val)override{
            // static uint64_t i = 0;
            try
            {
                setValue(fromestr()(val));
                // 从字符串进行构建
                // {
                //     std::shared_lock<std::shared_mutex> lock(mutex_);
                //     auto new_m_val = fromestr()(val);
                // }
                // // SYLAR_LOG_INFO_ROOT<<m_val;
                // // 这里需要进行参数的修改修改函数
                // auto func = getFunction(0xF1E231);
                // if(func){
                //     func(m_val,new_m_val);
                //     m_val = new_m_val;
                // }
                return true;
            }
            catch(const std::exception& e)
            {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())<<"configvar::fromstring exception"
                <<e.what()<<"convert:string to"<<typeid(m_val).name();
            }
            return false;
        }
        T getValue()const{
            std::shared_lock<std::shared_mutex> lock(mutex_);
            return m_val;
            }
        std::string getName()const override{
            
            std::shared_lock<std::shared_mutex> lock(mutex_);
            // 对T变成字符串
            return m_name;
        }
        std::string getDescription()const override{
            std::shared_lock<std::shared_mutex> lock(mutex_);
            return m_description;
            }
        void setValue(const T& value){

            // 这里通过了预设函数进行函数信息对象参数的设置。
            {
            std::shared_lock<std::shared_mutex> lock(mutex_);
                if(value == m_val) return; 
                for(auto&it:function_maps){                    
                    try{
                        it.second(m_val, value);
                    }catch(const std::exception& e){
                        // 参数不对应接着查找
                        continue;
                    }
                }
            }
                // 记住一个信念：对于对象自己的信息，就自己的信息被修改的地方
                // 进行加锁， 其他地方不要管
            std::unique_lock<std::shared_mutex> lock(mutex_);
            m_val = value;
        }
        // 就该参数的函数api
        // 对监听函数的设置。我可以从外部进行信息对象的参数的设置。
        uint64_t addlisten(value_change func){
            static uint64_t key = 0;
            std::unique_lock<std::shared_mutex> lock(mutex_);
            key++;
            function_maps[key] = func;
            return key;
        }
        void dellisten(const uint64_t& key, value_change func){
            uint64_t keys = 0;
            {
                std::shared_lock<std::shared_mutex> lock(mutex_);
                for(auto& it:function_maps){
                    if(it.first == key){
                        keys = it.first;
                    }
                }
            }
            std::unique_lock<std::shared_mutex> lock(mutex_);
            function_maps.erase(keys);
        }
        value_change getFunction(const uint64_t& key)const{
            std::shared_lock<std::shared_mutex> lock(mutex_);
            auto it = function_maps.find(key);
            return it == function_maps.end()? nullptr:it->second;
        }
        std::string getTypename()const override{
            return typeid(T).name();
            }
        void clearFcuntionMaps(){
            std::unique_lock<std::shared_mutex> lock(mutex_);
            function_maps.clear();
            }
        private:
            T m_val;
            mutable std::shared_mutex mutex_;
            std::unordered_map<uint64_t, value_change> function_maps;
    };

    // 主要配置器
    class config{
    public:
        // 对配置信息进行记录
        using ConfigVarMap = std::map<std::string, ConfigVarBase::ptr>;
        using ptr = std::shared_ptr<config>;
        
        // 从保存的数据中找到信息对象
        static ConfigVarBase::ptr LookupBase(const std::string& name);
        // 将信息记录：静态
        template<class T>
        static typename configvar<T>::ptr Lookup(const std::string& name,
            const T& default_value , const std::string& description=""
        ){
            // 调用重载模板函数
            sylar::ConfigVarBase::ptr item;
           {
                std::shared_lock<std::shared_mutex> lock(GetMutex());
                item = LookupBase(name);
           }

            // SYLAR_LOG_DEBUG_ROOT<<name;
            // 检查参数是否存在:存在就返回 
            if(item){
                if(item->getTypename()!=typeid(T).name()){
                    SYLAR_LOG_ERROR_ROOT<<"param: "<<item->getName()<<" exist but type not "<<typeid(T).name()
                    <<" the real type is "<<item->getTypename()<<".";
                    return nullptr;
                }
            }
            typename configvar<T>::ptr temp;
            {
                std::shared_lock<std::shared_mutex> lock(GetMutex());
                temp = Lookup<T>(name);
            }

            if(temp){
                SYLAR_LOG_INFO(SYLAR_LOG_ROOT())<<"Look name"<<name<<"exists"<<" "<<temp->getValue();
                
                return temp;
            }
            // 检查名字是否合法
            if(name.find_first_not_of(
                "abcdefghigklmnopqrstuvwxyz._0123456789 "
            )!=std::string::npos){
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())<<"lookup name invalid"<<name;
                throw std::invalid_argument(name);
            }
            // 不存在且合法就记录在案
            typename configvar<T>::ptr v(new configvar<T>(name, default_value, description));
            {
                std::unique_lock<std::shared_mutex> lock(GetMutex());
                s_datas[name] = v;
            }
            return v;            
        }

        // 重载函数对信息进行查找:静态
        template<class T>
        static typename configvar<T>::ptr Lookup(const std::string& name){
            auto it = s_datas.find(name);
            if(it == s_datas.end()){
                return nullptr;
            }
            // 确保返回的是指针
            return typename configvar<T>::ptr(std::dynamic_pointer_cast<configvar<T>>(it->second));
        }

        // yaml 配置解析实现
        void LoadFileFromYaml(const std::string& filename){
            // 拿到的时候是yaml转的字符串需要从字符串转成类型。
            root = YAML::LoadFile(filename);
            ListAllMember("", root, all_node);
            // ListAllMember("system", root["system"], all_node);
            // std::cout<<root.Type();
            for(auto&i:all_node){
                std::string key = i.first;
                if(key.empty()) continue;
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                // 拿到系统启动之时所自行设定的数据。
                ConfigVarBase::ptr var = LookupBase(key);
                // 如果存在
                if(var){
                    if(i.second.IsScalar()){
                        // 将保存的节点转成所需要的类型。
                        var->fromString(i.second.Scalar());
                    }else{
                        std::stringstream ss;
                        ss<<i.second;
                        var->fromString(ss.str());
                    }
                }
            }
        }

        static void ListAllMember(const std::string& prefix,
            const YAML::Node& node,
        // 这里的const学到了
            std::list<std::pair<std::string, const YAML::Node>>& output
            ){
                if(prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._01234566789")!=
                    std::string::npos
                ){
                    SYLAR_LOG_ERROR_ROOT <<" Config invaild name: "<< prefix <<" : "<<"";
                    return;
                }
                // SYLAR_LOG_INFO_ROOT<<"node type "<<node.Type();

                output.emplace_back(prefix, node);// 保存的是结点，键是yaml到达的路径。

                if(node.IsMap()){
                    // 如果是map，那么就递归的将其取出。并且保存到后面
                    for (auto it = node.begin();it != node.end();it++){
                        std::string temp = prefix.empty()?it->first.Scalar():prefix+"."+it->first.Scalar();
                        ListAllMember(temp, it->second, output);
                    }
                }
            }

        std::list<std::pair<std::string, const YAML::Node>> getAllNode()const{
            return all_node;
        }
        
        // YAML::Node getYamlRoot()const{return root;}
        static void visit(std::function<void(ConfigVarBase::ptr)> cb);
        private:
            static ConfigVarMap getSdata(){
                // 为什么这样呢，因为静态全局变量和方法的初始化顺序是不固定的，
                // 因此，当statci方法从s_data中拿数据的时候，此时如果s_datas
                // 如果没有初始化的时候，将导致段错误。
                return s_datas;}
            // 配置数据存放的地方
            static ConfigVarMap s_datas;// 需要进行外部初始化
            std::list<std::pair<std::string, const YAML::Node>> all_node;
            YAML::Node root;

            // 类模板调用的时候才创建
            static std::shared_mutex& GetMutex(){
                static std::shared_mutex mutex_;
                return mutex_;
            }
    };

    // 对logfine进行全特化
    template<>
    class LexicalCast<std::set<sylar::logDefine>, std::string>{
    public:
        std::string operator()(const std::set<sylar::logDefine>& set_val){
            YAML::Node node;
            for(auto&logger_:set_val){
                std::string name = logger_.name;
                std::string level = sylar::LogLevel::ToString(logger_.level);
                std::string formatter = logger_.formatter;
                // std::string file = logger_.file;

                node["logs"]["name"] = name;
                node["logs"]["level"] = level;
                node["logs"]["formatter"] = formatter;
                
                // 对appenders进行遍历
                for(const auto&it:logger_.appenders){
                    YAML::Node appeners_;
                    if (it.type == 1){
                        appeners_["type"] = "stdOutAppender";
                    }else if (it.type == 2)
                    {
                        appeners_["type"] = "fileAppender";
                    }
                    if(!it.file.empty()){
                        appeners_["file"] = it.file;
                    }
                    // 进行信息的整合
                    if(!it.formatter.empty()){
                        appeners_["formatter"] = it.formatter;
                    }
                    node["appenders"].push_back(appeners_);
                }
            }
            std::stringstream ss;
            ss<<node;
            return ss.str();
        }
    };

    template<>
    class LexicalCast<std::string, std::set<logDefine>>{
    public:
        std::set<logDefine> operator()(const std::string& val){
            YAML::Node logNode = YAML::Load(val);
            std::set<sylar::logDefine> sets;

            for(size_t i = 0; i<logNode.size();i++){
                sylar::logDefine ld;
                ld.name = logNode[i]["name"].as<std::string>();
                ld.level = sylar::LogLevel::fromString(
                    logNode[i]["level"].as<std::string>()
                );
                if(logNode[i]["formatter"].IsDefined()){
                    // 如果说这个日志器的formatter存在就设置
                    ld.formatter = logNode[i]["formatter"].as<std::string>();
                }
                // 对appenders进行设置
                for(const auto &a:logNode[i]["appender"]){
                    // 由于这里的节点是map因此需要进行打印输出
                    sylar::LogAppenderDefine lad;
                    // SYLAR_LOG_INFO_ROOT<<"从Yaml中拿到的数据: "<<(a["type"].as<std::string>() == "stdOutAppender");
                    if(a["type"].as<std::string>() == "stdOutAppender"){
                        lad.type = 1;
                        // 对a进行遍历将信息掏出来
                        for(const auto&it :a){
                            if (it.first.Scalar() == "level"){
                                lad.level = LogLevel::fromString(it.second.Scalar());
                            }else if(it.first.Scalar() == "formatter"){
                                lad.formatter = it.second.Scalar();
                            }
                        }
                        // SYLAR_LOG_INFO_ROOT<<"读不到"<<lad.type;
                        ld.appenders.push_back(lad);
                    }else{
                        lad.type = 2;
                        for(const auto& it:a){
                            if(it.first.Scalar() == "level"){
                                lad.level = LogLevel::fromString(it.second.Scalar());
                            }else if(it.first.Scalar() == "formatter"){
                                lad.formatter = it.second.Scalar();
                                // SYLAR_LOG_INFO_ROOT<<"捕获文件输出地的格式: "<<lad.formatter;
                            }else if(it.first.Scalar() == "file"){
                                lad.file = it.second.Scalar();
                                // SYLAR_LOG_INFO_ROOT<<"捕获输出位置文件地址："<<lad.file;
                            }
                        }
                        if(lad.file.empty()){
                            std::string logfile = "../logfile/"+ld.name+".txt";
                            lad.file = logfile;
                        }
                        ld.appenders.push_back(lad);
                    }
                }
                sets.insert(ld);
            }

            // // 解析完毕进行信息的整体打印
            // SYLAR_LOG_INFO_ROOT<<"==================解析=============";
            // for(auto&it:sets){
            //     SYLAR_LOG_INFO_ROOT<<"log name: "<<it.name;
            //     SYLAR_LOG_INFO_ROOT<<"log level: "<<sylar::LogLevel::ToString(it.level);
            //     SYLAR_LOG_INFO_ROOT<<"log formatter: "<<it.formatter;
            //     for(auto& appender:it.appenders){
            //         SYLAR_LOG_INFO_ROOT<<"\t"<<"appender type: "<<appender.type;
            //         SYLAR_LOG_INFO_ROOT<<"\t"<<"appender file: "<<appender.file;
            //         SYLAR_LOG_INFO_ROOT<<"\t"<<"level: "<<sylar::LogLevel::ToString(appender.level);
            //         SYLAR_LOG_INFO_ROOT<<"\t"<<"appender formatter: "<<appender.formatter;
            //     }
            // }
            return sets;
        }
    };
    
}
// 惊讶这个配置var和base这两个类，直接就将数据给隐藏起来了。

#endif