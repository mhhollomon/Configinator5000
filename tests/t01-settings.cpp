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
    CHECK(s.is_numeric());

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
    s.set_value(11);
    CHECK(x.get<int>() == 11);

    // scalars return a count of 0
    CHECK(s.count() == 0);

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
    CHECK_FALSE(s.is_numeric());

    CHECK(s.get<bool>());

    CHECK_THROWS(s.get<int>());
    CHECK_THROWS(s.get<std::string>());
    CHECK_FALSE(s.exists("foo"));

    // not composite, this should fail
    CHECK_THROWS(s.add_child(1));
    
    // scalars return a count of 0
    CHECK(s.count() == 0);

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
    CHECK(s.is_numeric());

    CHECK_THROWS(s.get<bool>());
    CHECK_THROWS(s.get<int>());
    CHECK(s.get<double>() == 42.0);
    CHECK_THROWS(s.get<std::string>());
    CHECK_FALSE(s.exists("foo"));
    
    // not composite, this should fail
    CHECK_THROWS(s.add_child(1));
    
    // scalars return a count of 0
    CHECK(s.count() == 0);

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
    CHECK_FALSE(s.is_numeric());

    CHECK_THROWS(s.get<bool>());
    CHECK_THROWS(s.get<int>());
    CHECK_THROWS(s.get<double>());
    CHECK(s.get<std::string>() == "foo bar"s);
    CHECK_FALSE(s.exists("foo"));
    
    // not composite, this should fail
    CHECK_THROWS(s.add_child(1));
    
    // scalars return a count of 0
    CHECK(s.count() == 0);

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

TEST_CASE("Groups") {
    using ST = Configinator5000::Setting::setting_type;

    Configinator5000::Setting s{};

    s.make_group();
    CHECK(s.is_group());
    CHECK(s.is_composite());
    CHECK_FALSE(s.is_scalar());

    auto & new_s = s.add_child("foo", 2);
    CHECK(s.exists("foo"));
    CHECK(new_s.is_integer());
    CHECK(new_s.get<int>() == 2);
    new_s.set_value(5);

    CHECK_THROWS(s.add_child("foo", true));

    auto &sub_group = s.add_child("bar", ST::GROUP);
    CHECK(s.exists("bar"));
    CHECK(sub_group.is_group());

    CHECK(s.count() == 2);
    CHECK(sub_group.count() == 0);

    // Group children must be named
    CHECK_THROWS(s.add_child(true));
    
    // be sure the variant with just a string
    // Calls the non-group one.
    CHECK_THROWS(s.add_child("bogus"s));
    CHECK_THROWS(s.add_child("bogus"));

    CHECK(s.at(0).get<int>() == 5);

    CHECK(s.at("foo").get<int>() == 5);
    CHECK_THROWS(s.at("fargate"));

}

TEST_CASE("Lists") {
    using ST = Configinator5000::Setting::setting_type;

    Configinator5000::Setting s{ST::LIST};

    CHECK(s.is_list());
    CHECK(s.is_composite());
    CHECK_FALSE(s.is_scalar());

    CHECK_THROWS(s.at(0));

    s.add_child(true);
    s.add_child(1);

    CHECK(s.at(0).get<bool>() == true);
    CHECK(s.at(-2).get<bool>() == true);

    CHECK(s.at(1).get<int>() == 1);
    CHECK(s.at(-1).get<int>() == 1);

    CHECK(s.count() == 2);

    // non-group
    CHECK_FALSE(s.exists("foo"));
    CHECK_THROWS(s.at("bar"));
}
