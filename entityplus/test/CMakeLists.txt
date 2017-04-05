set(Catch_DIR "Catch lib directory" CACHE PATH "Path to Catch")
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${Catch_DIR})

file(GLOB SOURCES "*.cpp")

add_executable(Tests ${SOURCES})
target_link_libraries(Tests Catch EntityPlus)

enable_testing()
add_test(NAME EntityPlusTests COMMAND Tests)
