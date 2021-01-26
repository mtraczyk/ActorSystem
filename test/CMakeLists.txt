include_directories(../../../mt418401)

add_executable(test_empty test_empty.c)
add_test(test_empty test_empty)

set_tests_properties(test_empty PROPERTIES TIMEOUT 1)
