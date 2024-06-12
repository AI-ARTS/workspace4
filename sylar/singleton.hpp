
#ifndef __SINGLETON__
#define __SINGLETON__

namespace sylar
{
    // 这是懒汉模式，需要时再创建
    template<class T, class X=void, int N=0>
    class Singleton{
    public:
        static T* GetInstance(){
            static T v;
            return &v;
        }
    };

    template<class T, class X=void, int N=0>
    class SingletonPtr{
        static std::shared_ptr<T> GetInstance(){
            static std::shared_ptr<T> v(new T);
            return v;
        }
    };
} // namespace sylar

#endif