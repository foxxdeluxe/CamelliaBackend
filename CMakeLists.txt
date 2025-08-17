cmake_minimum_required(VERSION 3.30)
project(CamelliaBackend)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

set(EXPORT_CSHARP_LIB_DIR "${CMAKE_BINARY_DIR}/export/csharp/lib")
set(EXPORT_CSHARP_GLUE_DIR "${CMAKE_BINARY_DIR}/export/csharp/glue")

include(FetchContent)

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

# main C++ library
add_library(
        CamelliaBackendCore
        STATIC
        variant.cpp
        helper/algorithm_helper.cpp
        helper/scripting_helper.cpp
        helper/text_layout_helper.cpp
        manager.cpp
        live/play/activity.cpp
        live/play/actor.cpp
        live/play/dialog.cpp
        live/play/scene.cpp
        live/play/stage.cpp
        live/action/action.cpp
        live/action/action_timeline.cpp
        attribute_registry.cpp
        data/stage_data.cpp
)

# Link libraries in correct order (dependencies matter)
target_link_libraries(
        CamelliaBackendCore PRIVATE
        # QuickJS-ng
        qjs
        # xxHash
        xxHash::xxhash
        # modified Skia
        unofficial::skia::skia
        unofficial::skia::modules::skparagraph
)

# Install the static library
install(TARGETS CamelliaBackendCore
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
install(FILES live/action/action_timeline.h DESTINATION include/camellia/live/action)
install(FILES live/action/action.h DESTINATION include/camellia/live/action)
install(FILES live/play/activity.h DESTINATION include/camellia/live/play)
install(FILES live/play/actor.h DESTINATION include/camellia/live/play)
install(FILES live/play/dialog.h DESTINATION include/camellia/live/play)
install(FILES live/play/scene.h DESTINATION include/camellia/live/play)
install(FILES live/play/stage.h DESTINATION include/camellia/live/play)
install(FILES ${constexpr-xxh3_SOURCE_DIR}/constexpr-xxh3.h DESTINATION include/camellia/third_party)


# Testing (placed after dependencies and include dirs so tests inherit them)
add_subdirectory(test)