cmake_minimum_required(VERSION 3.30)
project(CamelliaBackend)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

set(EXPORT_CSHARP_LIB_DIR "${CMAKE_BINARY_DIR}/export/csharp/lib")
set(EXPORT_CSHARP_GLUE_DIR "${CMAKE_BINARY_DIR}/export/csharp/glue")

# Testing
add_subdirectory(test)

include(FetchContent)

# QuickJS-ng
find_package(qjs REQUIRED)

# xxHash
FetchContent_Declare(
  xxHash
  GIT_REPOSITORY https://github.com/Cyan4973/xxHash.git
  GIT_TAG v0.8.3
)
FetchContent_MakeAvailable(xxHash)

# Set Skia paths directly
set(SKIA_ROOT_DIR "$ENV{SkiaParagraph_DIR}")
set(SKIA_LIBRARY_DIR "${SKIA_ROOT_DIR}/out/Static")
set(SKIA_INCLUDE_DIR "${SKIA_ROOT_DIR}/include")
set(SKIA_MODULES_DIR "${SKIA_ROOT_DIR}/modules")

# Set Skia library paths directly
set(SKIA_LIBRARY "${SKIA_LIBRARY_DIR}/libskia.a")
set(SKIA_PARAGRAPH_LIBRARY "${SKIA_LIBRARY_DIR}/libskparagraph.a")
set(SKIA_SHAPER_LIBRARY "${SKIA_LIBRARY_DIR}/libskshaper.a")
set(SKIA_UNICODE_CORE_LIBRARY "${SKIA_LIBRARY_DIR}/libskunicode_core.a")
set(SKIA_UNICODE_ICU_LIBRARY "${SKIA_LIBRARY_DIR}/libskunicode_icu.a")
set(SKIA_SKCMS_LIBRARY "${SKIA_LIBRARY_DIR}/libskcms.a")

# Verify that all Skia libraries exist
foreach(lib ${SKIA_LIBRARY} ${SKIA_PARAGRAPH_LIBRARY} ${SKIA_SHAPER_LIBRARY} 
            ${SKIA_UNICODE_CORE_LIBRARY} ${SKIA_UNICODE_ICU_LIBRARY} ${SKIA_SKCMS_LIBRARY})
    if(NOT EXISTS ${lib})
        message(FATAL_ERROR "Required Skia library not found: ${lib}")
    endif()
endforeach()

# Add system dependencies that Skia typically needs
find_package(PkgConfig REQUIRED)
find_package(Freetype REQUIRED)
find_package(ICU REQUIRED COMPONENTS uc)
# Use pkg-config for HarfBuzz since find_package may not work reliably
pkg_check_modules(HARFBUZZ REQUIRED harfbuzz)


# Set up include directories
include_directories(
    ${PROJECT_SOURCE_DIR} 
    # QuickJS-ng
    ${qjs_SOURCE_DIR}
    # xxHash
    ${xxhash_SOURCE_DIR}
    # Skia
    ${SKIA_ROOT_DIR}
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
        # xxHash (built as part of the project)
        ${xxhash_SOURCE_DIR}/xxhash.c
        ${xxhash_SOURCE_DIR}/xxh_x86dispatch.c
)

# Link libraries in correct order (dependencies matter)
target_link_libraries(
        CamelliaBackendCore PRIVATE
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

# swig
find_package(SWIG REQUIRED)
include(UseSWIG)

# get_target_property(CAMELLIA_BACKEND_CORE_SOURCES CamelliaBackendCore SOURCES)
set(SWIG_INTERFACE_FILES
        interface/csharp/root.i
)

set_property(SOURCE ${SWIG_INTERFACE_FILES} PROPERTY CPLUSPLUS ON)
# Add include directories for SWIG to find header files
set_property(SOURCE ${SWIG_INTERFACE_FILES} PROPERTY SWIG_FLAGS "-I${PROJECT_SOURCE_DIR}")
# set_property(SOURCE ${SWIG_INTERFACE_FILES} APPEND PROPERTY OBJECT_DEPENDS "${CAMELLIA_BACKEND_CORE_SOURCES};${PROJECT_SOURCE_DIR}")
swig_add_library(
        CamelliaBackendCSharp
        TYPE SHARED
        LANGUAGE CSharp
        OUTPUT_DIR csharp
        SOURCES ${SWIG_INTERFACE_FILES}
)
set_property(TARGET CamelliaBackendCSharp PROPERTY SWIG_COMPILE_DEFINITIONS SWIG_CSHARP_NO_STRING_HELPER SWIG_CSHARP_NO_EXCEPTION_HELPER)

target_link_libraries(
        CamelliaBackendCSharp
        PRIVATE CamelliaBackendCore)

add_custom_command(
        TARGET CamelliaBackendCSharp POST_BUILD
        COMMENT "Copying SWIG C# bindings & build outputs..."
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${EXPORT_CSHARP_LIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${EXPORT_CSHARP_GLUE_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${EXPORT_CSHARP_LIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${EXPORT_CSHARP_GLUE_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:CamelliaBackendCSharp> ${EXPORT_CSHARP_LIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/csharp ${EXPORT_CSHARP_GLUE_DIR}
        COMMAND ${CMAKE_COMMAND} -E rm ${EXPORT_CSHARP_GLUE_DIR}/rootCSHARP_wrap.cxx
        COMMAND ${CMAKE_COMMAND} -E rm ${EXPORT_CSHARP_GLUE_DIR}/rootCSHARP_wrap.h
)