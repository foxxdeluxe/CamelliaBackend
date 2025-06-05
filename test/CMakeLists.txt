project(test)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${CamelliaBackend_SOURCE_DIR} ${CamelliaBackend_SOURCE_DIR}/include)

enable_testing()

add_executable(test_run value_test.cpp
        scripting_test.cpp
        stage_test.cpp)
target_link_libraries(test_run CamelliaBackendCore)
target_link_libraries(test_run GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_run)