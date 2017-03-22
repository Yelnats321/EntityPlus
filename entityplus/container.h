//          Copyright Elnar Dakeshov 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)


#pragma once

#include <vector>
#include <algorithm>

namespace entityplus {

template <typename Key, typename Compare = std::less<Key>,
	typename Allocator = std::allocator<Key>>
class flat_set : private std::vector<Key, Allocator> {
	Compare comp;
public:
	using container_type = std::vector<Key, Allocator>;
	using key_type = Key;
	using value_type = Key;
	using key_compare = Compare;
	using value_compare = Compare;
	using typename container_type::size_type;
	using typename container_type::difference_type;
	using typename container_type::allocator_type;
	using typename container_type::reference;
	using typename container_type::const_reference;
	using typename container_type::pointer;
	using typename container_type::const_pointer;
	using typename container_type::iterator;
	using typename container_type::const_iterator;
	using typename container_type::reverse_iterator;
	using typename container_type::const_reverse_iterator;

	using container_type::begin;
	using container_type::cbegin;

	using container_type::rbegin;
	using container_type::crbegin;

	using container_type::end;
	using container_type::cend;

	using container_type::rend;
	using container_type::crend;

	using container_type::empty;
	using container_type::size;
	using container_type::max_size;

	template <typename... Args>
	std::pair<iterator, bool> emplace(Args&&... args) {
		container_type::emplace_back(std::forward<Args>(args)...);
		auto lower = std::lower_bound(begin(), end(), container_type::back(), comp);
		if (*lower == container_type::back() && lower != end() - 1) {
			container_type::pop_back();
			return{lower, false};
		}
		if (lower == end() - 1) {
			return{end() - 1, true};
		}
		return{std::rotate(rbegin(), rbegin() + 1, reverse_iterator{lower}).base(), true};
	}

	iterator find(const key_type &key) {
		auto lower = std::lower_bound(begin(), end(), key, comp);
		if (lower != end() && *lower == key) return lower;
		return end();
	}
	const_iterator find(const key_type &key) const {
		auto lower = std::lower_bound(begin(), end(), key, comp);
		if (lower != end() && *lower == key) return lower;
		return end();
	}

	iterator erase(const_iterator pos) {
		return container_type::erase(pos);
	}
	size_type erase(const key_type &key) {
		auto itr = find(key);
		if (itr == end()) return 0;
		erase(itr);
		return 1;
	}
	
	static flat_set from_sorted_underlying(container_type &&other) {
		flat_set set;
		static_cast<container_type &>(set) = other;
		return set;
	}
};

template <typename Key, typename T, typename Compare = std::less<Key>,
	typename Allocator = std::allocator<std::pair<Key, T>>>
class flat_map: private std::vector<std::pair<Key, T>, Allocator> {
public:
	using key_type = Key;
	using mapped_type = T;
	using value_type = std::pair<Key, T>;
	using container_type = std::vector<value_type, Allocator>;
	using key_compare = Compare;

	struct value_compare {
	protected:
		key_compare keyComp;
	public:
		value_compare(key_compare kc): keyComp(kc) {}
		bool operator()(const value_type &lhs, const value_type &rhs) const {
			return keyComp(lhs.first, rhs.first);
		}
	};

private:
	key_compare keyComp;
	value_compare valComp{keyComp};
public:
	using typename container_type::size_type;
	using typename container_type::difference_type;
	using typename container_type::allocator_type;
	using typename container_type::reference;
	using typename container_type::const_reference;
	using typename container_type::pointer;
	using typename container_type::const_pointer;
	using typename container_type::iterator;
	using typename container_type::const_iterator;
	using typename container_type::reverse_iterator;
	using typename container_type::const_reverse_iterator;

	using container_type::begin;
	using container_type::cbegin;

	using container_type::rbegin;
	using container_type::crbegin;

	using container_type::end;
	using container_type::cend;

	using container_type::rend;
	using container_type::crend;

	using container_type::empty;
	using container_type::size;
	using container_type::max_size;

	template <typename... Args>
	std::pair<iterator, bool> emplace(Args&&... args) {
		container_type::emplace_back(std::forward<Args>(args)...);
		auto lower = std::lower_bound(begin(), end(), container_type::back(), valComp);
		if (lower->first == container_type::back().first && lower != end() - 1) {
			container_type::pop_back();
			return{lower, false};
		}
		if (lower == end() - 1) {
			return{end() - 1, true};
		}
		return{std::rotate(rbegin(), rbegin() + 1, reverse_iterator{lower}).base(), true};

	}

	iterator find(const key_type &key) {
		auto lower = std::lower_bound(begin(), end(), key,
									  [&](const value_type &val, const key_type &key) {
			return keyComp(val.first, key);
		});
		if (lower != end() && lower->first == key) return lower;
		return end();
	}
	const_iterator find(const key_type &key) const {
		auto lower = std::lower_bound(begin(), end(), key, 
									  [&](const value_type &val, const key_type &key) {
			return keyComp(val.first, key);
		});
		if (lower != end() && lower->first == key) return lower;
		return end();
	}

	iterator erase(const_iterator pos) {
		return container_type::erase(pos);
	}
	size_type erase(const key_type &key) {
		auto itr = find(key);
		if (itr == end()) return 0;
		erase(itr);
		return 1;
	}
};

}
