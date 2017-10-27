//          Copyright Elnar Dakeshov 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <catch.hpp>
#include <entityplus/metafunctions.h>
#include <entityplus/typelist.h>

using namespace entityplus::meta;

static_assert(!typelist_has_type_v<char, typelist<>>);
static_assert(typelist_has_type_v<float, typelist<float, int, double>>);
static_assert(typelist_has_type_v<int, typelist<float, int, double>>);
static_assert(typelist_has_type_v<double, typelist<float, int, double>>);
static_assert(!typelist_has_type_v<char, typelist<float, int, double>>);
static_assert(typelist_has_type_v<double, typelist<double, int, double>>);

static_assert(is_typelist_unique_v<typelist<float, int, double>>);
static_assert(!is_typelist_unique_v<typelist<float, double, int, double>>);
static_assert(!is_typelist_unique_v<typelist<double, float, double, int, double>>);
static_assert(is_typelist_unique_v<typelist<>>);

static_assert(typelist_index_v<int, typelist<int, float, double>> == 0);
static_assert(typelist_index_v<float, typelist<int, float, double>> == 1);
static_assert(typelist_index_v<double, typelist<int, float, double>> == 2);

static_assert(std::is_same<typelist_concat_t<typelist<float, int>, typelist<double, char>>,
                           typelist<float, int, double, char>>{});
static_assert(
    std::is_same<typelist_concat_t<typelist<float, int>, typelist<>>, typelist<float, int>>{});
static_assert(
    std::is_same<typelist_concat_t<typelist<>, typelist<float, int>>, typelist<float, int>>{});
static_assert(std::is_same<typelist_concat_t<typelist<>, typelist<>>, typelist<>>{});

static_assert(std::is_same<
              typelist_intersection_t<typelist<int, float, double>, typelist<double, float, char>>,
              typelist<float, double>>{});
static_assert(
    std::is_same<typelist_intersection_t<typelist<int, float, double>, typelist<>>, typelist<>>{});
static_assert(
    std::is_same<typelist_intersection_t<typelist<>, typelist<int, float, double>>, typelist<>>{});

TEST_CASE("type_bitset", "[metafunctions]") {
    using types = typelist<struct A, struct B, struct C>;

    SECTION("operator[] and get") {
        type_bitset<types> tba, tbb;
        tba[0] = true;
        get<A>(tbb) = true;
        REQUIRE((tba == tbb));
    }
    SECTION("operators") {
        type_bitset<types> tba, tbb;
        get<B>(tba) = true;
        REQUIRE((tba != tbb));
        REQUIRE(((tba & tbb) == type_bitset<types>{}));
        REQUIRE(((tba & tba) == tba));
    }
}
