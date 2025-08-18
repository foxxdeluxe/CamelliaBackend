cmake_minimum_required(VERSION 3.30)
project(CamelliaBackend)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

set(EXPORT_CSHARP_LIB_DIR "${CMAKE_BINARY_DIR}/export/csharp/lib")
set(EXPORT_CSHARP_GLUE_DIR "${CMAKE_BINARY_DIR}/export/csharp/glue")

include(FetchContent)
include(GenerateExportHeader)
include(CheckIPOSupported)

check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_OUTPUT)

if(IPO_SUPPORTED)
    message(STATUS "IPO supported. Enabling IPO.")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
else()
    message(STATUS "IPO not supported: \n${IPO_OUTPUT}")
endif()

# QuickJS-ng
FetchContent_Declare(
    quickjs-ng
    GIT_REPOSITORY https://github.com/quickjs-ng/quickjs.git
    GIT_TAG master
)
FetchContent_MakeAvailable(quickjs-ng)

# constexpr-xxh3 (header-only)
FetchContent_Declare(
    constexpr-xxh3
    GIT_REPOSITORY https://github.com/chys87/constexpr-xxh3.git
    GIT_TAG main
)
FetchContent_MakeAvailable(constexpr-xxh3)

# xxHash
find_package(xxHash CONFIG REQUIRED)

# modified Skia
find_package(unofficial-skia CONFIG REQUIRED)


# Set up include directories
include_directories(
    ${PROJECT_SOURCE_DIR} 
    # QuickJS-ng
    ${quickjs-ng_SOURCE_DIR}
    # constexpr-xxh3
    ${constexpr-xxh3_SOURCE_DIR}
)

add_library(
    CamelliaBackendDeps INTERFACE
)

# Link libraries in correct order (dependencies matter)
target_link_libraries(
    CamelliaBackendDeps INTERFACE
    # QuickJS-ng
    qjs
    # xxHash
    xxHash::xxhash
    # modified Skia
    unofficial::skia::modules::skparagraph
)

# main C++ library
add_library(
        CamelliaBackendObject
        OBJECT
        variant.cpp
        helper/algorithm_helper.cpp
        helper/scripting_helper.cpp
        helper/text_layout_helper.cpp
        manager.cpp
        node/activity.cpp
        node/actor.cpp
        node/dialog.cpp
        node/scene.cpp
        node/stage.cpp
        node/action/action.cpp
        node/action/action_timeline.cpp
        attribute_registry.cpp
        data/stage_data.cpp
)

# Link dependencies to CamelliaBackendObject for compilation
target_link_libraries(
        CamelliaBackendObject PRIVATE
        CamelliaBackendDeps
)

add_library(
        CamelliaBackendStatic
        STATIC
        $<TARGET_OBJECTS:CamelliaBackendObject>
)

# Link the same dependencies as CamelliaBackendObject to CamelliaBackendStatic
target_link_libraries(
        CamelliaBackendStatic PUBLIC
        CamelliaBackendDeps
)

# Install the static library
install(TARGETS CamelliaBackendStatic
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)

# Install header files with directory structure
install(FILES camellia.h DESTINATION include/camellia)
install(FILES camellia_typedef.h DESTINATION include/camellia)
install(FILES variant.h DESTINATION include/camellia)
install(FILES attribute_registry.h DESTINATION include/camellia)
install(FILES manager.h DESTINATION include/camellia)
install(FILES data/stage_data.h DESTINATION include/camellia/data)
install(FILES helper/algorithm_helper.h DESTINATION include/camellia/helper)
install(FILES helper/scripting_helper.h DESTINATION include/camellia/helper)
install(FILES helper/text_layout_helper.h DESTINATION include/camellia/helper)
install(FILES node/action/action_timeline.h DESTINATION include/camellia/node/action)
install(FILES node/action/action.h DESTINATION include/camellia/node/action)
install(FILES node/activity.h DESTINATION include/camellia/node)
install(FILES node/actor.h DESTINATION include/camellia/node)
install(FILES node/dialog.h DESTINATION include/camellia/node)
install(FILES node/scene.h DESTINATION include/camellia/node)
install(FILES node/stage.h DESTINATION include/camellia/node)
install(FILES ${constexpr-xxh3_SOURCE_DIR}/constexpr-xxh3.h DESTINATION include/camellia/third_party)

add_library(
    CamelliaBackendShared
    SHARED
    $<TARGET_OBJECTS:CamelliaBackendObject>
)

target_link_libraries(
        CamelliaBackendShared PRIVATE
        CamelliaBackendDeps
)

# On ELF platforms, drop unused DT_NEEDED entries to prune transitive deps
if(UNIX AND NOT APPLE)
    target_link_options(CamelliaBackendShared PRIVATE "-Wl,--as-needed")
endif()

generate_export_header(CamelliaBackendShared)

# Testing (placed after dependencies and include dirs so tests inherit them)
add_subdirectory(test)