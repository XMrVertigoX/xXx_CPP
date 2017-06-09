#include <stdint.h>
#include <stdlib.h>

#include "../thirdparty/Catch/single_include/catch.hpp"

#include "singleton.hpp"

class TestSingleton : public xXx::Singleton<TestSingleton> {
    friend class xXx::Singleton<TestSingleton>;

   private:
    ~TestSingleton() = default;
    TestSingleton()  = default;

    TestSingleton(const TestSingleton &other) = default;
    TestSingleton &operator=(const TestSingleton &other) = default;

    TestSingleton(TestSingleton &&other) = default;
    TestSingleton &operator=(TestSingleton &&other) = default;
};

TEST_CASE("", "[Singleton]") {
    TestSingleton &singletonA = TestSingleton::getInstance();
    TestSingleton &singletonB = TestSingleton::getInstance();

    REQUIRE(&singletonA == &singletonB);
}
