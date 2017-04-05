file(GLOB SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" *.cpp)

foreach(src ${SOURCES})
	string(REPLACE ".cpp" "" srcName ${src})
	add_executable(${srcName} ${src})
	target_link_libraries(${srcName} EntityPlus)
endforeach(src ${SOURCES})
