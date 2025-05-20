#############################################################################
#Copyright (c) 2025,
#
#Distributed under the terms of the MIT license.
#
#The full license is in the file LICENSE, distributed with this software.
#############################################################################

cmake_minimum_required(VERSION 3.4.3)
project(xeus-gab)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

set(XEUS_GAB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Versionning
# ===========

file(STRINGS "${XEUS_GAB_INCLUDE_DIR}/xeus-gab/xeus_gab_config.hpp" version_defines
     REGEX "#define XEUS_GAB_VERSION_(MAJOR|MINOR|PATCH)")
foreach (ver ${version_defines})
    if (ver MATCHES "#define XEUS_GAB_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
        set(XEUS_GAB_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif ()
endforeach ()
set(${PROJECT_NAME}_VERSION
    ${XEUS_GAB_VERSION_MAJOR}.${XEUS_GAB_VERSION_MINOR}.${XEUS_GAB_VERSION_PATCH})
message(STATUS "Building xeus-gab v${${PROJECT_NAME}_VERSION}")

# Configuration
# =============

include(GNUInstallDirs)

if (NOT DEFINED XEUS_GAB_KERNELSPEC_PATH)
    set(XEUS_GAB_KERNELSPEC_PATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/")
endif ()

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xgab/kernel.json.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xgab/kernel.json"
)

# Build options
# =============

option(XEUS_GAB_BUILD_STATIC "Build xeus-gab static library" ON)
OPTION(XEUS_GAB_BUILD_SHARED "Split xgab build into executable and library" ON)
OPTION(XEUS_GAB_BUILD_EXECUTABLE "Build the xgab executable" ON)

OPTION(XEUS_GAB_USE_SHARED_XEUS "Link xgab  with the xeus shared library (instead of the static library)" ON)
OPTION(XEUS_GAB_USE_SHARED_XEUS_GAB "Link xgab  with the xeus-gab shared library (instead of the static library)" OFF)

# Dependencies
# ============

set(xeus_REQUIRED_VERSION 5.0.0)

find_package(xeus ${xeus_REQUIRED_VERSION} REQUIRED)

# Flags
# =====
include(CheckCXXCompilerFlag)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251 /wd4141")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4018 /wd4267 /wd4715 /wd4146 /wd4129")
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-parameter -Wextra -Wreorder")
    

    CHECK_CXX_COMPILER_FLAG("-std=c++17" HAS_CPP_17_FLAG)
    if (HAS_CPP_17_FLAG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    else ()
        message(FATAL_ERROR "Unsupported compiler -- xeus requires C++17 support!")
    endif ()
endif ()



# Source files
# ============

set(XEUS_GAB_HEADERS
    include/xeus-gab/xeus_gab_config.hpp
    include/xeus-gab/xinterpreter.hpp
)

set(XEUS_GAB_SRC
    src/xinterpreter.cpp
)

set(XEUS_GAB_MAIN_SRC
    src/main.cpp
)


# Targets and link - Macros
# =========================

include(CheckCXXCompilerFlag)

string(TOUPPER "${CMAKE_BUILD_TYPE}" U_CMAKE_BUILD_TYPE)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib; ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

macro(xeus_gab_set_common_options target_name)
    if (MSVC)
        target_compile_options(${target_name} PUBLIC /wd4251 /wd4141)
        target_compile_options(${target_name} PUBLIC /wd4018 /wd4267 /wd4715 /wd4146 /wd4129)
    endif ()

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
        CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
        CMAKE_CXX_COMPILER_ID MATCHES "Intel")

        target_compile_options(${target_name} PUBLIC -Wunused-parameter -Wextra -Wreorder)

        # C++17 flag
        CHECK_CXX_COMPILER_FLAG("-std=c++17" HAS_CPP_17_FLAG)
        if (HAS_CPP_17_FLAG)
            target_compile_features(${target_name} PRIVATE cxx_std_17)
        else ()
            message(FATAL_ERROR "Unsupported compiler -- xeus-gab requires C++17 support!")
        endif ()
    endif ()

    if (APPLE)
        set_target_properties(${target_name} PROPERTIES
            MACOSX_RPATH ON
        )
    else ()
        set_target_properties(${target_name} PROPERTIES
            BUILD_WITH_INSTALL_RPATH 1
            SKIP_BUILD_RPATH FALSE
        )
    endif ()

    set_target_properties(${target_name} PROPERTIES
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )
endmacro()

# Common macro kernels (xgab)
macro(xeus_gab_set_kernel_options target_name)
    if (XEUS_GAB_USE_SHARED_XEUS_GAB)
        target_link_libraries(${target_name} PRIVATE xeus-gab)
        if(CMAKE_DL_LIBS)
            target_link_libraries(${target_name} PRIVATE ${CMAKE_DL_LIBS} util)
        endif()
    else ()
        target_link_libraries(${target_name} PRIVATE xeus-gab-static)
    endif()

    find_package(Threads)
        target_link_libraries(${target_name} PRIVATE ${CMAKE_THREAD_LIBS_INIT})
    
endmacro()

# Common macro for shared and static library
macro(xeus_gab_create_target target_name linkage output_name)
    string(TOUPPER "${linkage}" linkage_upper)

    if (NOT ${linkage_upper} MATCHES "^(SHARED|STATIC)$")
        message(FATAL_ERROR "Invalid library linkage: ${linkage}")
    endif ()

    add_library(${target_name} ${linkage_upper} ${XEUS_GAB_SRC} ${XEUS_GAB_HEADERS})
    xeus_gab_set_common_options(${target_name})

    set_target_properties(${target_name} PROPERTIES
                          PUBLIC_HEADER "${XEUS_GAB_HEADERS}"
                          PREFIX ""
                          VERSION ${${PROJECT_NAME}_VERSION}
                          SOVERSION ${XEUS_GAB_VERSION_MAJOR}
                          OUTPUT_NAME "lib${output_name}")

    target_compile_definitions(${target_name} PUBLIC "XEUS_GAB_EXPORTS")

    target_compile_features(${target_name} PRIVATE cxx_std_17)

    target_include_directories(${target_name}
                               PUBLIC
                               $<BUILD_INTERFACE:${XEUS_GAB_INCLUDE_DIR}>
                               $<INSTALL_INTERFACE:include>)

    if (XEUS_GAB_USE_SHARED_XEUS)
        set(XEUS_GAB_XEUS_TARGET xeus)
    else ()
        set(XEUS_GAB_XEUS_TARGET xeus-static)
    endif ()

    target_link_libraries(${target_name} PUBLIC ${XEUS_GAB_XEUS_TARGET})
    if (WIN32 OR CYGWIN)
        #
    elseif (APPLE)
        target_link_libraries(${target_name} PRIVATE "-undefined dynamic_lookup")
    endif ()
    find_package(Threads) # TODO: add Threads as a dependence of xeus-static?
        target_link_libraries(${target_name} PRIVATE ${CMAKE_THREAD_LIBS_INIT})
    
endmacro()

# xeus-gab
# ===========

set(XEUS_GAB_TARGETS "")

if (XEUS_GAB_BUILD_SHARED)
    # Build libraries
    xeus_gab_create_target(xeus-gab SHARED xeus-gab)
    list(APPEND XEUS_GAB_TARGETS xeus-gab)
endif ()

if (XEUS_GAB_BUILD_STATIC)
    # On Windows, a static library should use a different output name
    # to avoid the conflict with the import library of a shared one.
    if (CMAKE_HOST_WIN32)
        xeus_gab_create_target(xeus-gab-static STATIC xeus-gab-static)
    else ()
        xeus_gab_create_target(xeus-gab-static STATIC xeus-gab)
    endif ()
    if(EMSCRIPTEN)
        target_compile_options(xeus-gab-static PRIVATE -fPIC)
    endif()
    list(APPEND XEUS_GAB_TARGETS xeus-gab-static)
endif ()

# xgab
# =======
if (XEUS_GAB_BUILD_EXECUTABLE)
    find_package(xeus-zmq 1.0.2 REQUIRED)
    add_executable(xgab ${XEUS_GAB_MAIN_SRC})
    target_compile_features(xgab PRIVATE cxx_std_17)
    xeus_gab_set_common_options(xgab)
    xeus_gab_set_kernel_options(xgab)
    target_link_libraries(xgab PRIVATE xeus-zmq)
    target_link_libraries(xgab PRIVATE cgab -rdynamic)
endif()

# Installation
# ============
include(CMakePackageConfigHelpers)

set(XEUS_GAB_CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE STRING "install path for xeus-gabConfig.cmake")

# Install xeus-gab and xeus-gab-static
if (XEUS_GAB_BUILD_SHARED)
    install(TARGETS ${XEUS_GAB_TARGETS}
            EXPORT ${PROJECT_NAME}-targets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xeus-gab)

    # Makes the project importable from the build directory
    export(EXPORT ${PROJECT_NAME}-targets
           FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")
endif ()

# Install xgab
if (XEUS_GAB_BUILD_EXECUTABLE)
    install(TARGETS xgab
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if (XEUS_GAB_BUILD_EXECUTABLE OR EMSCRIPTEN)
    # Configuration and data directories for jupyter and xeus-gab
    set(XJUPYTER_DATA_DIR "share/jupyter"    CACHE STRING "Jupyter data directory")

    # Install xgab Jupyter kernelspec
    set(KERNELSPEC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels)
    install(DIRECTORY ${KERNELSPEC_DIR}
            DESTINATION ${XJUPYTER_DATA_DIR}
            PATTERN "*.in" EXCLUDE)


    # Extra path for installing Jupyter kernelspec
    if (XEXTRA_JUPYTER_DATA_DIR)
        install(DIRECTORY ${KERNELSPEC_DIR}
                DESTINATION ${XEXTRA_JUPYTER_DATA_DIR}
                PATTERN "*.in" EXCLUDE)
    endif ()
endif ()

# Configure 'xeus-gabConfig.cmake' for a build tree
set(XEUS_GAB_CONFIG_CODE "####### Expanded from \@XEUS_GAB_CONFIG_CODE\@ #######\n")
set(XEUS_GAB_CONFIG_CODE "${XEUS_GAB_CONFIG_CODE}set(CMAKE_MODULE_PATH \"${CMAKE_CURRENT_SOURCE_DIR}/cmake;\${CMAKE_MODULE_PATH}\")\n")
set(XEUS_GAB_CONFIG_CODE "${XEUS_GAB_CONFIG_CODE}##################################################")
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${PROJECT_BINARY_DIR})

# Configure 'xeus-gabConfig.cmake.in for an install tree
set(XEUS_GAB_CONFIG_CODE "")
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${XEUS_GAB_CMAKECONFIG_INSTALL_DIR})

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
                                 VERSION ${${PROJECT_NAME}_VERSION}
                                 COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
              DESTINATION ${XEUS_GAB_CMAKECONFIG_INSTALL_DIR})

if (XEUS_GAB_BUILD_SHARED)
    install(EXPORT ${PROJECT_NAME}-targets
            FILE ${PROJECT_NAME}Targets.cmake
            DESTINATION ${XEUS_GAB_CMAKECONFIG_INSTALL_DIR})
endif ()


