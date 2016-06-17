#include "metafunctions.h"
#include "Entity.h"

int main() {
	using namespace entityplus;
	using namespace entityplus::meta;
	//using type = entityplus::ComponentList<int, int>::type;
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

	using MyComponentList = ComponentList<int, float>;
	using MyTagList = TagList<>;
	EntityManager<MyComponentList, MyTagList> em;
}