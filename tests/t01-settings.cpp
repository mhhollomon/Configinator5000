#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <configinator5000.hpp>

#include <string>

using namespace std::literals::string_literals;

TEST_CASE("settings") {

    Configinator5000::Setting s{3};
    CHECK(s.is_integer());

    int i = s.get<int>();
    CHECK(i == 3);
    double f = s.get<double>();
    CHECK(f == 3.0);


    CHECK_THROWS(s.get<std::string>());

}

