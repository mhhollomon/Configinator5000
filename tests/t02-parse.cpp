#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <configinator5000.hpp>

#include <string>

using namespace std::literals::string_literals;

TEST_CASE("compiles") {
    Configinator5000::Config cfg;
    CHECK(true);

    std::string input = R"DELIM( port 
= 7777; )DELIM"s;

    bool b = cfg.parse(input);

    CHECK(b == true);

    auto & s = cfg.get_settings();

    CHECK(s.count() == 1);
    CHECK(s.exists("port"));
    CHECK(s.is_group());
    CHECK(s.at("port").is_integer());
    CHECK(s.at("port").get<int>() == 7777);
}

TEST_CASE("string") {
    Configinator5000::Config cfg;
    CHECK(true);

    std::string input = R"DELIM( port = "hello"; )DELIM"s;

    bool b = cfg.parse(input);

    CHECK(b);

    auto & s = cfg.get_settings();

    CHECK(s.count() == 1);
    CHECK(s.exists("port"));
    CHECK(s.is_group());
    CHECK(s.at("port").is_string());
    CHECK(s.at("port").get<std::string>() == "hello"s);

    // Multi strings

    input = R"DELIM( port = "hel"
"lo"
)DELIM"s;

    b = cfg.parse(input);
    CHECK(b);

    auto & s2 = cfg.get_settings();

    CHECK(s2.count() == 1);
    CHECK(s2.exists("port"));
    CHECK(s2.at("port").is_string());
    CHECK(s2.at("port").get<std::string>() == "hello"s);

}

TEST_CASE("float") {
    Configinator5000::Config cfg;
    CHECK(true);

    std::string input = R"DELIM( port = 42.0; )DELIM"s;

    bool b = cfg.parse(input);

    CHECK(b);

    auto & s = cfg.get_settings();

    CHECK(s.count() == 1);
    CHECK(s.exists("port"));
    CHECK(s.is_group());
    CHECK(s.at("port").is_float());
    CHECK(s.at("port").get<double>() == 42.0);
}

TEST_CASE("bool") {
    Configinator5000::Config cfg;

    std::string input = R"DELIM( 
        b1 = TRUE;
        b2 : FALSE
        b3 = TruE,
        b4 : fALse
 )DELIM"s;

    bool b = cfg.parse(input);

    CHECK(b);

    auto & s = cfg.get_settings();

    CHECK(s.count() == 4);
    CHECK(s.exists("b1"));
    CHECK(s.at("b1").is_boolean());
    CHECK(s.at("b1").get<bool>() == true);
    CHECK(s.at("b2").is_boolean());
    CHECK(s.at("b2").get<bool>() == false);
    CHECK(s.at("b3").is_boolean());
    CHECK(s.at("b3").get<bool>() == true);
    CHECK(s.at("b4").is_boolean());
    CHECK(s.at("b4").get<bool>() == false);
}

TEST_CASE("Nested Group") {
    Configinator5000::Config cfg;

    std::string input = R"DELIM( a = {
    a1 = true;
    a2 = 3;
    }
b = {
    b1 = {
        bb1 : "foo",
    },
}
)DELIM"s;

    bool b = cfg.parse(input);

    CHECK(b);

    auto & s = cfg.get_settings();
    CHECK(s.count() == 2);
    CHECK(s.exists("a"));
    CHECK(s.exists("b"));

    auto &a = s.at("a");
    CHECK(a.count() == 2);
    CHECK(a.at("a1").get<bool>() == true);
    CHECK(a.at("a2").get<int>() == 3);

    
    CHECK(s.at("b").at("b1").at("bb1").get<std::string>() == "foo"s);

}
