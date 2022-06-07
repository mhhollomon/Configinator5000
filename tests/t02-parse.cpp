#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <configinator5000.hpp>

#include <string>

using namespace std::literals::string_literals;

TEST_CASE("compiles") {
    Configinator5000::Config cfg;
    CHECK(true);

    std::string input = R"DELIM( port = 7777; )DELIM"s;

    bool b = cfg.parse(input);

    CHECK(b == true);
}

