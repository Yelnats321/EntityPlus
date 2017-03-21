//          Copyright Elnar Dakeshov 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <catch.hpp>
#include "../metafunctions.h"
#include "../typelist.h"

using namespace entityplus::meta;

static_assert(!and_all<std::true_type, std::true_type, std::false_type>{}, "");
static_assert(!and_all<std::true_type, std::false_type, std::true_type>{}, "");
static_assert(!and_all<std::false_type, std::true_type, std::true_type>{}, "");
static_assert(and_all<std::true_type, std::true_type, std::true_type>{}, "");
static_assert(!and_all<std::false_type>{}, "");
static_assert(and_all<std::true_type>{}, "");
static_assert(and_all<>{}, "");

static_assert(or_all<std::true_type, std::true_type, std::false_type>{}, "");
static_assert(or_all<std::true_type, std::false_type, std::true_type>{}, "");
static_assert(or_all<std::false_type, std::true_type, std::true_type>{}, "");
static_assert(!or_all<std::false_type, std::false_type, std::false_type>{}, "");
static_assert(or_all<std::true_type, std::true_type, std::true_type>{}, "");
static_assert(!or_all<std::false_type>{}, "");
static_assert(or_all<std::true_type>{}, "");
static_assert(!or_all<>{}, "");

static_assert(!typelist_has_type_v<char, typelist<>>, "");
static_assert(typelist_has_type_v<float, typelist<float, int, double>>, "");
static_assert(typelist_has_type_v<int, typelist<float, int, double>>, "");
static_assert(typelist_has_type_v<double, typelist<float, int, double>>, "");
static_assert(!typelist_has_type_v<char, typelist<float, int, double>>, "");
static_assert(typelist_has_type_v<double, typelist<double, int, double>>, "");

static_assert(is_typelist_unique_v<typelist<float, int, double>>, "");
static_assert(!is_typelist_unique_v<typelist<float, double, int, double>>, "");
static_assert(!is_typelist_unique_v<typelist<double, float, double, int, double>>, "");
static_assert(is_typelist_unique_v<typelist<>>, "");

static_assert(typelist_index_v<int, typelist<int, float, double>> == 0, "");
static_assert(typelist_index_v<float, typelist<int, float, double>> == 1, "");
static_assert(typelist_index_v<double, typelist<int, float, double>> == 2, "");

static_assert(std::is_same<
	typelist_concat_t<typelist<float, int>, typelist<double, char>>,
	typelist<float, int, double, char>
>{}, "");
static_assert(std::is_same<
	typelist_concat_t<typelist<float, int>, typelist<>>,
	typelist<float, int>
>{}, "");
static_assert(std::is_same<
	typelist_concat_t<typelist<>, typelist<float, int>>,
	typelist<float, int>
>{}, "");
static_assert(std::is_same<
	typelist_concat_t<typelist<>, typelist<>>,
	typelist<>
>{}, "");

static_assert(std::is_same<
	typelist_intersection_t<typelist<int, float, double>, typelist <double, float, char>>,
	typelist<float, double>
>{}, "");
static_assert(std::is_same<
	typelist_intersection_t<typelist<int, float, double>, typelist <>>,
	typelist<>
>{}, "");
static_assert(std::is_same<
	typelist_intersection_t<typelist<>, typelist <int, float, double>>,
	typelist<>
>{}, "");

TEST_CASE("for_each", "[metafunctions]") {
	const std::tuple<int, float, double> tupOriginal{2, 3.4f, 5.6};
	SECTION("triple with ref") {
		auto tup = tupOriginal;
		for_each(tup, [](auto &x, std::size_t i, auto) {
			x = x*3 + (int)i;
		});
		REQUIRE(std::get<int>(tupOriginal) * 3 == std::get<int>(tup));
		REQUIRE(std::get<float>(tupOriginal) * 3 + 1 == std::get<float>(tup));
		REQUIRE(std::get<double>(tupOriginal) * 3 + 2 == std::get<double>(tup));
	}
	SECTION("triple without ref") {
		auto tup = tupOriginal;
		for_each(tup, [](auto x, std::size_t, auto) {
			x *= 3;
		});
		REQUIRE(tup == tupOriginal);
	}
}

TEST_CASE("eval_if", "[metafunctions]") {
	REQUIRE(eval_if([](auto) { return 1; },
					fail_cond<std::true_type>([](auto) { return 2.3f; }),
					fail_cond<std::true_type>([](auto) { return std::tuple<int>{3}; }))
			== 1);
	REQUIRE(eval_if([](auto) { return 1; },
					fail_cond<std::false_type>([](auto) { return 2.3f; }),
					fail_cond<std::true_type>([](auto) { return std::tuple<int>{3}; }))
			== 2.3f);
	REQUIRE(eval_if([](auto) { return 1; },
					fail_cond<std::true_type>([](auto) { return 2.3f; }),
					fail_cond<std::false_type>([](auto) { return std::tuple<int>{3}; }))
			== std::tuple<int>{3});
	REQUIRE(eval_if([](auto) { return 1; },
					fail_cond<std::false_type>([](auto) { return 2.3f; }),
					fail_cond<std::false_type>([](auto) { return std::tuple<int>{3}; }))
			== 2.3f);
}

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
