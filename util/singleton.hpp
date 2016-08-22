#ifndef SINGLETON_HPP_
#define SINGLETON_HPP_

template <typename TYPE>
class Singleton {
   public:
    static TYPE& getInstance();

   protected:
    Singleton() = default;
    ~Singleton() = default;
};

template <typename TYPE>
TYPE& Singleton<TYPE>::getInstance() {
    static TYPE instance;
    return (instance);
}

#endif  // SINGLETON_HPP_
