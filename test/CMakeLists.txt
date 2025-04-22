project(test)
add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CamelliaBackend_SOURCE_DIR})

add_executable(test_run value_test.cpp
        scripting_test.cpp)
target_link_libraries(test_run CamelliaBackendCore)
target_link_libraries(test_run gtest gtest_main)