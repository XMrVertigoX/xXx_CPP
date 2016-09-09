#ifndef SINGLETON_HPP_
#define SINGLETON_HPP_

template <typename TYPE>
class Singleton {
   public:
    static TYPE& getInstance();

   protected:
    Singleton() = default;
    ~Singleton() = default;

   private:
    static TYPE _instance;
};

template <typename TYPE>
TYPE Singleton<TYPE>::_instance;

template <typename TYPE>
TYPE& Singleton<TYPE>::getInstance() {
    return (_instance);
}

#endif  // SINGLETON_HPP_
