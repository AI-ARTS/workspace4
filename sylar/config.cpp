#include"config.hpp"

namespace sylar{
    config::ConfigVarMap config::s_datas;

    ConfigVarBase::ptr config::LookupBase(const std::string& name){

        auto it = config::s_datas.find(name);
        return it == config::s_datas.end()? nullptr: it->second;
    }
    void config::visit(std::function<void(ConfigVarBase::ptr)> cb){
        std::shared_lock<std::shared_mutex> lock(GetMutex()); 
        for(auto&it:config::s_datas){
            cb(it.second);
        }
    }
}