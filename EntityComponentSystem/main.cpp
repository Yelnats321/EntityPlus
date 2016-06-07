#include "metafunctions.h"

int main() {
	using namespace entityplus::meta;
	static_assert(tuple_has_type<
				  int,
				  std::tuple<float, double, int>
	>::value == true, "");

	static_assert(tuple_has_type<
				  int,
				  std::tuple<float, double>
	>::value == false , ""); 

	static_assert(is_tuple_unique<
				  std::tuple<float, double, int>
	>::value == true, "");
	static_assert(is_tuple_unique<
				  std::tuple<float, double, float>
	>::value == false, "");
	static_assert(is_tuple_unique<
				  std::tuple<int, float, double, float>
	>::value == false, "");

	/*static_assert(std::is_same<
				  detail::tuple_concat_t<
				  std::tuple<int, float>,
				  std::tuple<double, long>
				  >,
				  std::tuple<int, float, double, long>
	>::value == true, "");

	static_assert(detail::are_tuples_unique<
				  std::tuple<int, float>,
				  std::tuple<double, long>
	>::value == false, "");

	static_assert(detail::are_tuples_unique<
				  std::tuple<int, float>,
				  std::tuple<float, long>
	>::value == true, "");

	static_assert(detail::are_tuples_unique<
				  std::tuple<int>,
				  std::tuple<float, long>
	>::value == false, "");*/
}