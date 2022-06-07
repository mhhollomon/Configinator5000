#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <configinator5000.hpp>

#include <string>

using namespace std::literals::string_literals;

TEST_CASE("integer") {

    Configinator5000::Setting s{3};
    CHECK(s.is_integer());
    CHECK_FALSE(s.is_boolean());
    CHECK_FALSE(s.is_float());
    CHECK_FALSE(s.is_string());
    CHECK_FALSE(s.is_group());
    CHECK_FALSE(s.is_array());
    CHECK_FALSE(s.is_list());
    CHECK(s.is_scalar());
    CHECK_FALSE(s.is_composite());

    int i = s.get<int>();
    CHECK(i == 3);
    double f = s.get<double>();
    CHECK(f == 3.0);


    CHECK_THROWS(s.get<std::string>());
    CHECK_THROWS(s.get<bool>());
    CHECK_FALSE(s.exists("foo"));

    auto & x = s.set_value(9);
    CHECK(s.is_integer());
    CHECK(s.get<int>() == 9);
    //
    // make sure we are pointing to the correct setting
    CHECK(x.get<int>() == 9);

}

TEST_CASE("bool") {

    Configinator5000::Setting s{true};
    CHECK_FALSE(s.is_integer());
    CHECK(s.is_boolean());
    CHECK_FALSE(s.is_float());
    CHECK_FALSE(s.is_string());
    CHECK_FALSE(s.is_group());
    CHECK_FALSE(s.is_array());
    CHECK_FALSE(s.is_list());
    CHECK(s.is_scalar());
    CHECK_FALSE(s.is_composite());

    CHECK(s.get<bool>());

    CHECK_THROWS(s.get<int>());
    CHECK_THROWS(s.get<std::string>());
    CHECK_FALSE(s.exists("foo"));

}

TEST_CASE("float") {

    Configinator5000::Setting s{42.0};
    CHECK_FALSE(s.is_integer());
    CHECK_FALSE(s.is_boolean());
    CHECK(s.is_float());
    CHECK_FALSE(s.is_string());
    CHECK_FALSE(s.is_group());
    CHECK_FALSE(s.is_array());
    CHECK_FALSE(s.is_list());
    CHECK(s.is_scalar());
    CHECK_FALSE(s.is_composite());

    CHECK_THROWS(s.get<bool>());
    CHECK_THROWS(s.get<int>());
    CHECK(s.get<double>() == 42.0);
    CHECK_THROWS(s.get<std::string>());
    CHECK_FALSE(s.exists("foo"));

}

TEST_CASE("std::string") {

    Configinator5000::Setting s{"foo bar"};
    CHECK_FALSE(s.is_integer());
    CHECK_FALSE(s.is_boolean());
    CHECK_FALSE(s.is_float());
    CHECK(s.is_string());
    CHECK_FALSE(s.is_group());
    CHECK_FALSE(s.is_array());
    CHECK_FALSE(s.is_list());
    CHECK(s.is_scalar());
    CHECK_FALSE(s.is_composite());

    CHECK_THROWS(s.get<bool>());
    CHECK_THROWS(s.get<int>());
    CHECK_THROWS(s.get<double>());
    CHECK(s.get<std::string>() == "foo bar"s);
    CHECK_FALSE(s.exists("foo"));

}

TEST_CASE("scalar conversions") {
    Configinator5000::Setting s{"foo bar"};

    CHECK(s.is_string());

    s.set_value(9);
    CHECK(s.is_integer());
    CHECK(s.get<long>() == 9);
    CHECK_FALSE(s.is_string());

    s.set_value(9.0);
    CHECK_FALSE(s.is_integer());
    CHECK(s.is_float());
    CHECK(s.get<double>() == 9.0);
    CHECK_THROWS(s.get<long>());

    s.set_value("blah");
    CHECK_FALSE(s.is_float());
    CHECK(s.is_string());
    CHECK(s.get<std::string>() == "blah"s);
    CHECK_THROWS(s.get<double>());
}

