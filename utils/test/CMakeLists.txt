add_executable(utils_unittest
	RsolverUtilsTest.cpp
	RsolverVisionTest.cpp
	RsolverIntegrationTest.cpp
	RsolverControlTest.cpp)
target_link_libraries(utils_unittest gtest_main gmock_main rsolverutils)
add_test(NAME RsolverUtilsTest COMMAND utils_unittest)