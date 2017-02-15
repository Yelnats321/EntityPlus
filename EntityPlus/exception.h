#pragma once

#ifndef ENTITYPLUS_NO_EXCEPTIONS
#include <stdexcept>
#endif

namespace entityplus {

enum class error_code_t {
	BAD_ENTITY,
	INVALID_COMPONENT
};

#ifndef ENTITYPLUS_NO_EXCEPTIONS

struct bad_entity : std::logic_error {
	using std::logic_error::logic_error;
};

struct invalid_component : std::logic_error {
	using std::logic_error::logic_error;
};

#endif

}
