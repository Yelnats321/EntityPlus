#pragma once
#include <stdexcept>

namespace entityplus {

struct bad_entity_exception : std::logic_error {
	using std::logic_error::logic_error;
};

struct invalid_component : std::logic_error {
	using std::logic_error::logic_error;
};

}