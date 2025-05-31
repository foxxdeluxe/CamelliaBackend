cmake_minimum_required(VERSION 3.30)
project(CamelliaBackend)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

set(EXPORT_LIB_DIR "${CMAKE_BINARY_DIR}/export/lib")
set(EXPORT_INCLUDE_DIR "${CMAKE_BINARY_DIR}/export/include")
set(EXPORT_CSHARP_LIB_DIR "${CMAKE_BINARY_DIR}/export/csharp/lib")
set(EXPORT_CSHARP_GLUE_DIR "${CMAKE_BINARY_DIR}/export/csharp/glue")

add_subdirectory(quickjs)
add_subdirectory(test)

include_directories(${PROJECT_SOURCE_DIR})

# Collect all header files for the library export
set(CAMELLIA_HEADERS
    camellia_typedef.h
    variant.h
    attribute_registry.h
    manager.h
    data/stage_data.h
    helper/algorithm_helper.h
    helper/scripting_helper.h
    live/action/action_timeline.h
    live/action/action.h
    live/play/activity.h
    live/play/actor.h
    live/play/dialog.h
    live/play/scene.h
    live/play/stage.h
)

# Create a generated header file for library export
configure_file(
    "${PROJECT_SOURCE_DIR}/camellia.h"
    "${EXPORT_INCLUDE_DIR}/camellia.h"
    COPYONLY
)

# Create export directories at configure time
file(MAKE_DIRECTORY ${EXPORT_LIB_DIR})
file(MAKE_DIRECTORY ${EXPORT_INCLUDE_DIR})
file(MAKE_DIRECTORY ${EXPORT_INCLUDE_DIR}/data)
file(MAKE_DIRECTORY ${EXPORT_INCLUDE_DIR}/helper)
file(MAKE_DIRECTORY ${EXPORT_INCLUDE_DIR}/live/action)
file(MAKE_DIRECTORY ${EXPORT_INCLUDE_DIR}/live/play)
file(MAKE_DIRECTORY ${EXPORT_INCLUDE_DIR}/quickjs)

# Copy all header files using configure_file
configure_file(${PROJECT_SOURCE_DIR}/camellia_typedef.h ${EXPORT_INCLUDE_DIR}/camellia_typedef.h COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/variant.h ${EXPORT_INCLUDE_DIR}/variant.h COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/attribute_registry.h ${EXPORT_INCLUDE_DIR}/attribute_registry.h COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/manager.h ${EXPORT_INCLUDE_DIR}/manager.h COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/data/stage_data.h ${EXPORT_INCLUDE_DIR}/data/stage_data.h COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/helper/algorithm_helper.h ${EXPORT_INCLUDE_DIR}/helper/algorithm_helper.h COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/helper/scripting_helper.h ${EXPORT_INCLUDE_DIR}/helper/scripting_helper.h COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/live/action/action_timeline.h ${EXPORT_INCLUDE_DIR}/live/action/action_timeline.h COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/live/action/action.h ${EXPORT_INCLUDE_DIR}/live/action/action.h COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/live/play/activity.h ${EXPORT_INCLUDE_DIR}/live/play/activity.h COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/live/play/actor.h ${EXPORT_INCLUDE_DIR}/live/play/actor.h COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/live/play/dialog.h ${EXPORT_INCLUDE_DIR}/live/play/dialog.h COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/live/play/scene.h ${EXPORT_INCLUDE_DIR}/live/play/scene.h COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/live/play/stage.h ${EXPORT_INCLUDE_DIR}/live/play/stage.h COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/quickjs/quickjs.h ${EXPORT_INCLUDE_DIR}/quickjs/quickjs.h COPYONLY)

# main C++ library
add_library(
        CamelliaBackendCore
        STATIC
        variant.cpp
        helper/algorithm_helper.cpp
        helper/scripting_helper.cpp
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
        xxHash/xxhash.c
        xxHash/xxh_x86dispatch.c
)

target_link_libraries(
        CamelliaBackendCore
        PRIVATE qjs
)

add_custom_command(
        TARGET CamelliaBackendCore POST_BUILD
        COMMENT "Copying core build outputs..."
        
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:CamelliaBackendCore> ${EXPORT_LIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:qjs> ${EXPORT_LIB_DIR}
)

# swig
find_package(SWIG REQUIRED)
include(UseSWIG)

# get_target_property(CAMELLIA_BACKEND_CORE_SOURCES CamelliaBackendCore SOURCES)
set(SWIG_INTERFACE_FILES
        interface/csharp/root.i
)

set_property(SOURCE ${SWIG_INTERFACE_FILES} PROPERTY CPLUSPLUS ON)
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