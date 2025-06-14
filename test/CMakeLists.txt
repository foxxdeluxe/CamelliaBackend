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

# Add dependencies needed by CamelliaBackendCore
find_package(qjs REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Freetype REQUIRED)
find_package(ICU REQUIRED COMPONENTS uc)
pkg_check_modules(HARFBUZZ REQUIRED harfbuzz)

# Set Skia paths (inherited from parent)
set(SKIA_ROOT_DIR "$ENV{SkiaParagraph_DIR}")
set(SKIA_LIBRARY_DIR "${SKIA_ROOT_DIR}/out/Static")
set(SKIA_LIBRARY "${SKIA_LIBRARY_DIR}/libskia.a")
set(SKIA_PARAGRAPH_LIBRARY "${SKIA_LIBRARY_DIR}/libskparagraph.a")
set(SKIA_SHAPER_LIBRARY "${SKIA_LIBRARY_DIR}/libskshaper.a")
set(SKIA_UNICODE_CORE_LIBRARY "${SKIA_LIBRARY_DIR}/libskunicode_core.a")
set(SKIA_UNICODE_ICU_LIBRARY "${SKIA_LIBRARY_DIR}/libskunicode_icu.a")
set(SKIA_SKCMS_LIBRARY "${SKIA_LIBRARY_DIR}/libskcms.a")

include_directories(
  ${gtest_SOURCE_DIR}/include 
  ${CamelliaBackend_SOURCE_DIR}
)

enable_testing()

add_executable(test_run value_test.cpp
        scripting_test.cpp
        stage_test.cpp
        text_layout_test.cpp)

# Link the same libraries as CamelliaBackendCore
target_link_libraries(
        test_run PRIVATE
        CamelliaBackendCore
        GTest::gtest_main
        # QuickJS-ng
        qjs
        # Skia
        ${SKIA_PARAGRAPH_LIBRARY}
        ${SKIA_SHAPER_LIBRARY}
        ${SKIA_UNICODE_ICU_LIBRARY}
        ${SKIA_UNICODE_CORE_LIBRARY}
        ${SKIA_LIBRARY}
        ${SKIA_SKCMS_LIBRARY}
        # Skia dependencies
        ${HARFBUZZ_LIBRARIES}
        Freetype::Freetype
        ICU::uc
)

include(GoogleTest)
gtest_discover_tests(test_run)