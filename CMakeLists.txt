cmake_minimum_required(VERSION 3.30)
project(CamelliaBackend VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

set(EXPORT_CSHARP_LIB_DIR "${CMAKE_BINARY_DIR}/export/csharp/lib")
set(EXPORT_CSHARP_GLUE_DIR "${CMAKE_BINARY_DIR}/export/csharp/glue")

include(FetchContent)
include(GenerateExportHeader)
include(CheckIPOSupported)
include(GNUInstallDirs)

check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_OUTPUT)

if(IPO_SUPPORTED)
    message(STATUS "IPO supported. Enabling IPO.")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
else()
    message(STATUS "IPO not supported: \n${IPO_OUTPUT}")
endif()

# QuickJS-ng
find_package(qjs CONFIG REQUIRED)

# xxHash
find_package(xxHash CONFIG REQUIRED)

# modified Skia
find_package(unofficial-skia CONFIG REQUIRED)

# Set up include directories
include_directories(
    ${PROJECT_SOURCE_DIR}
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

# Set up proper target properties for static library
set_target_properties(CamelliaBackendStatic PROPERTIES
    OUTPUT_NAME camellia-backend
    PUBLIC_HEADER ""
)

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

# Set up proper target properties for shared library
set_target_properties(CamelliaBackendShared PROPERTIES
    OUTPUT_NAME camellia-backend
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER ""
)

# Install all targets with export sets
install(TARGETS CamelliaBackendStatic CamelliaBackendShared CamelliaBackendDeps
        EXPORT CamelliaBackendTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install all headers while preserving directory structure
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/camellia
        FILES_MATCHING PATTERN "*.h"
        PATTERN "build*" EXCLUDE
        PATTERN "cmake-build*" EXCLUDE
        PATTERN "_deps" EXCLUDE
        PATTERN "test" EXCLUDE
        PATTERN ".git*" EXCLUDE
        PATTERN ".cache*" EXCLUDE
        PATTERN ".idea*" EXCLUDE
        PATTERN ".vscode*" EXCLUDE
        PATTERN "vcpkg_installed*" EXCLUDE
)

# Install the export header
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/camelliabackendshared_export.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/camellia
)

# Install constexpr-xxh3 header
find_path(CONSTEXPR_XXH3_INCLUDE_DIR constexpr-xxh3.h REQUIRED)
install(FILES ${CONSTEXPR_XXH3_INCLUDE_DIR}/constexpr-xxh3.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/camellia
)

# Install constexpr-xxh3 license file
find_path(CONSTEXPR_XXH3_LICENSE_DIR copyright PATHS ${_VCPKG_INSTALLED_DIR}/*/share/constexpr-xxh3 REQUIRED)
install(FILES ${CONSTEXPR_XXH3_LICENSE_DIR}/copyright
        DESTINATION ${CMAKE_INSTALL_DATADIR}/camellia/licenses
        RENAME constexpr-xxh3-copyright
)

# Install the targets export file
install(EXPORT CamelliaBackendTargets
        FILE CamelliaBackendTargets.cmake
        NAMESPACE CamelliaBackend::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CamelliaBackend
)

# Create and install config files
include(CMakePackageConfigHelpers)

# Generate the config file
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CamelliaBackendConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/CamelliaBackendConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CamelliaBackend
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR
)

# Generate the version file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/CamelliaBackendConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Install the config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/CamelliaBackendConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/CamelliaBackendConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CamelliaBackend
)

# Testing (placed after dependencies and include dirs so tests inherit them)
add_subdirectory(test)