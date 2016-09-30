#ifndef SINGLETON_HPP_
#define SINGLETON_HPP_

namespace xXx {

template <typename TYPE> class Singleton {
  public:
    static TYPE &getInstance();

  protected:
    Singleton() = default;
    ~Singleton() = default;

  private:
    static TYPE _instance;
};

template <typename TYPE> TYPE Singleton<TYPE>::_instance;

template <typename TYPE> TYPE &Singleton<TYPE>::getInstance() {
    return (_instance);
}

} /* namespace xXx */

#endif /* SINGLETON_HPP_ */
