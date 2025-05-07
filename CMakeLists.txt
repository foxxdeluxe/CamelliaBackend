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
        live/play/timeline_evaluator.cpp
        live/play/stage.cpp
        live/action/action.cpp
        live/action/action_timeline.cpp
        attribute_registry.cpp
        live/exception/uninitialized_exception.cpp
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

        COMMAND ${CMAKE_COMMAND} -E remove_directory ${EXPORT_LIB_DIR} 
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${EXPORT_INCLUDE_DIR} 

        COMMAND ${CMAKE_COMMAND} -E make_directory ${EXPORT_LIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:CamelliaBackendCore> ${EXPORT_LIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:qjs> ${EXPORT_LIB_DIR}

        COMMAND ${CMAKE_COMMAND} -E make_directory ${EXPORT_INCLUDE_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/camellia.h ${EXPORT_INCLUDE_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/camellia_typedef.h ${EXPORT_INCLUDE_DIR}

        COMMAND ${CMAKE_COMMAND} -E make_directory ${EXPORT_INCLUDE_DIR}/quickjs
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/quickjs/quickjs.h ${EXPORT_INCLUDE_DIR}/quickjs/
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