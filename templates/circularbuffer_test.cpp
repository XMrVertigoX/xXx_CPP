#include <stdint.h>
#include <stdlib.h>

#include "../thirdparty/Catch/single_include/catch.hpp"

#include "circularbuffer.hpp"

TEST_CASE("", "[CircularBuffer]") {
    int numberOfElements = 0;

    xXx::CircularBuffer<int> buffer(numberOfElements);

    REQUIRE(buffer.itemsAvailable() == 0);
    REQUIRE(buffer.slotsAvailable() == 0);
}

TEST_CASE("", "[CircularBuffer]") {
    int numberOfElements = 7;

    xXx::CircularBuffer<int> buffer(numberOfElements);

    REQUIRE(buffer.itemsAvailable() == 0);
    REQUIRE(buffer.slotsAvailable() == numberOfElements);

    for (int i = 0; i < numberOfElements; i++) {
        bool successfullyPushed = buffer.push(i);
        CHECK(successfullyPushed);
    }

    REQUIRE(buffer.itemsAvailable() == numberOfElements);
    REQUIRE(buffer.slotsAvailable() == 0);

    for (int i = 0; i < numberOfElements; i++) {
        int tmp;
        bool successfullyPopped = buffer.pop(tmp);
        CHECK(successfullyPopped);
        CHECK(tmp == i);
    }

    REQUIRE(buffer.itemsAvailable() == 0);
    REQUIRE(buffer.slotsAvailable() == numberOfElements);
}

TEST_CASE("", "[CircularBuffer]") {
    const int numberOfElements = 23;

    xXx::CircularBuffer<int> buffer(numberOfElements);

    CHECK(buffer.itemsAvailable() == 0);
    CHECK(buffer.slotsAvailable() == numberOfElements);

    for (int i = 0; i < 13; i++) {
        bool successfullyPushed = buffer.push(i);
        CHECK(successfullyPushed);
    }

    CHECK(buffer.itemsAvailable() == 13);
    CHECK(buffer.slotsAvailable() == numberOfElements - 13);

    for (int i = 0; i < 13; i++) {
        int tmp;
        bool successfullyPopped = buffer.pop(tmp);
        CHECK(successfullyPopped);
        CHECK(tmp == i);
    }

    CHECK(buffer.itemsAvailable() == 0);
    CHECK(buffer.slotsAvailable() == numberOfElements);

    for (int i = 0; i < numberOfElements; i++) {
        bool successfullyPushed = buffer.push(i);
        CHECK(successfullyPushed == true);
    }

    CHECK(buffer.itemsAvailable() == numberOfElements);
    CHECK(buffer.slotsAvailable() == 0);

    for (int i = 0; i < numberOfElements; i++) {
        int tmp;
        bool successfullyPopped = buffer.pop(tmp);
        CHECK(successfullyPopped);
        CHECK(tmp == i);
    }

    CHECK(buffer.itemsAvailable() == 0);
    CHECK(buffer.slotsAvailable() == numberOfElements);
}
