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

# Use the same dependencies as the main project
# Skia is already configured in parent scope via vcpkg

include_directories(
  ${gtest_SOURCE_DIR}/include 
  ${CamelliaBackend_SOURCE_DIR}
)

enable_testing()

add_executable(test_run value_test.cpp
        scripting_test.cpp
        stage_test.cpp
        text_layout_test.cpp
        serialization_test.cpp)

# Link the same libraries as CamelliaBackendCore
target_link_libraries(
        test_run PRIVATE
        CamelliaBackendStatic
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_run)