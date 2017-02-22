//          Copyright Elnar Dakeshov 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <catch.hpp>

#include "../container.h"

TEST_CASE("simple set", "[flat_set]") {
	entityplus::flat_set<int> set;
	REQUIRE(set.size() == 0);
	REQUIRE(set.empty());
	auto ret = set.emplace(3);
	REQUIRE(ret.second);
	REQUIRE(*ret.first == 3);
	*ret.first = 4;
	REQUIRE(set.size() == 1);
	REQUIRE(set.find(3) == set.end());
	REQUIRE(set.find(4) == set.begin());
	REQUIRE(set.erase(3) == 0);
	REQUIRE(set.size() == 1);
	REQUIRE(set.find(3) == set.end());
	REQUIRE(set.find(4) == set.begin());
	REQUIRE(set.erase(4) == 1);
	REQUIRE(set.find(3) == set.end());
	REQUIRE(set.find(4) == set.end());
	REQUIRE(set.size() == 0);
}

TEST_CASE("set ordering invariant", "[flat_set]") {
	entityplus::flat_set<int> set;
	set.emplace(2);
	set.emplace(3);
	set.emplace(4);
	int count = 2;
	for (auto i : set) {
		REQUIRE(i == count++);
	}
	REQUIRE(count == 5);
	count = 1;
	set.emplace(1);
	for (auto i : set) {
		REQUIRE(i == count++);
	}
	std::vector<int> cv = {1,2,3,4};
	REQUIRE(std::equal(set.begin(), set.end(), cv.begin(), cv.end()));
	REQUIRE(count == 5);

	REQUIRE(!set.emplace(2).second);
	set.erase(2);
	std::vector<int> test = {1,3,4};
	REQUIRE(std::equal(set.begin(), set.end(), test.begin(), test.end()));
	count = 1;
	set.emplace(2);
	for (auto i : set) {
		REQUIRE(i == count++);
	}
}

TEST_CASE("simple map", "[flat_map]") {
	entityplus::flat_map<int, float> map;
	REQUIRE(map.size() == 0);
	REQUIRE(map.empty());
	auto ret = map.emplace(3, 3.14f);
	REQUIRE(ret.second);
	REQUIRE(ret.first->first == 3);
	REQUIRE(ret.first->second == 3.14f);
	ret.first->second = 4;
	REQUIRE(map.size() == 1);
	REQUIRE(map.find(4) == map.end());
	REQUIRE(map.find(3) == map.begin());
	REQUIRE(map.erase(4) == 0);
	REQUIRE(map.size() == 1);
	REQUIRE(map.find(4) == map.end());
	REQUIRE(map.find(3) == map.begin());
	REQUIRE(map.erase(3) == 1);
	REQUIRE(map.find(3) == map.end());
	REQUIRE(map.find(4) == map.end());
	REQUIRE(map.size() == 0);
}

TEST_CASE("map ordering invariant", "[flat_map]") {
	entityplus::flat_map<int, float> map;
	map.emplace(2, 1);
	map.emplace(3, 1);
	map.emplace(4, 1);
	int count = 2;
	for (auto i : map) {
		REQUIRE(i.first == count++);
	}
	REQUIRE(count == 5);
	count = 1;
	map.emplace(1,5);
	for (auto i : map) {
		REQUIRE(i.first == count++);
	}
	REQUIRE(count == 5);

	REQUIRE(!map.emplace(2, 3).second);
	map.erase(2);
	std::vector<std::pair<int, float>> test = {std::make_pair(1, 5.f),
		std::make_pair(3, 1.f),
		std::make_pair(4, 1.f)};
	REQUIRE(std::equal(map.begin(), map.end(), test.begin(), test.end()));
	count = 1;
	map.emplace(2, 3);
	for (auto i : map) {
		REQUIRE(i.first == count++);
	}
}
