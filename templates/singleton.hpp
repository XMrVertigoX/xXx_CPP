#ifndef SINGLETON_HPP_
#define SINGLETON_HPP_

namespace xXx {

template <typename TYPE>
class Singleton {
   private:
    static TYPE instance;

    Singleton(const Singleton &other) = default;
    Singleton &operator=(const Singleton &other) = default;

    Singleton(Singleton &&other) = default;
    Singleton &operator=(Singleton &&other) = default;

   protected:
    ~Singleton() = default;
    Singleton()  = default;

   public:
    static TYPE &getInstance();
};

template <typename TYPE>
TYPE Singleton<TYPE>::instance;

template <typename TYPE>
TYPE &Singleton<TYPE>::getInstance() {
    return (instance);
}

} /* namespace xXx */

#endif /* SINGLETON_HPP_ */
