#pragma once

#ifndef ENTITYPLUS_NO_EXCEPTIONS
#include <stdexcept>
#endif

namespace entityplus {

#ifdef ENTITYPLUS_NO_EXCEPTIONS

enum class error_code_t {
	BAD_ENTITY,
	INVALID_COMPONENT
};

using error_callback_t = void(error_code_t);

#else

struct bad_entity_exception : std::logic_error {
	using std::logic_error::logic_error;
};

template <class Component>
struct invalid_component : std::logic_error {
	using std::logic_error::logic_error;
};

#endif

}