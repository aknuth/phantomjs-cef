# Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

cmake_minimum_required(VERSION 2.8.12.1)

if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/cef")
  message(FATAL_ERROR "The `cef` subfolder is missing. Download CEF from https://cefbuilds.com/ and extract its contents to `${CMAKE_CURRENT_SOURCE_DIR}/cef`.")
endif()

# Add cef as a subdirectory.
add_subdirectory(cef)

# Add cef variables to the current scope.
get_directory_property(_CEF_VARS DIRECTORY cef VARIABLES)
foreach(_CEF_VAR ${_CEF_VARS})
  if (NOT _CEF_VAR MATCHES "^CMAKE_CURRENT_")
    get_directory_property(${_CEF_VAR} DIRECTORY cef DEFINITION ${_CEF_VAR})
  endif()
endforeach()

# Init project variables for PhantomJS-cef.
project(PhantomJS-cef)
include_directories(${PROJECT_SOURCE_DIR}/cef)

find_package(Qt5 NO_MODULE REQUIRED COMPONENTS Gui)

#
# Source files.
#
set(CMAKE_AUTOMOC ON)

# phantomjs sources.
set(PHANTOMJS_SRCS
        app.cpp
        handler.cpp
        print_handler.cpp
        main.cpp
        debug.cpp
        fs.cpp
        crashdump.cpp
        )

if(OS_LINUX)
    LIST(APPEND PHANTOMJS_SRCS
            keyevents_linux.cpp
            )
endif()


set(CEF_RESOURCE_DIR        "project_cef-prefix/src/project_cef/Resources")
set(CEF_BINARY_DIR          "project_cef-prefix/src/project_cef/Release")
#set(CEF_BINARY_DIR_DEBUG    "${CMAKE_CURRENT_SOURCE_DIR}/Debug")
#set(CEF_BINARY_DIR_RELEASE  "${CMAKE_CURRENT_SOURCE_DIR}/Release")

# CEF library paths.
#set(CEF_LIB_DEBUG   "${CEF_BINARY_DIR_DEBUG}/libcef.so")
#set(CEF_LIB_RELEASE "${CEF_BINARY_DIR_RELEASE}/libcef.so")

# List of CEF binary files.
set(CEF_BINARY_FILES
  libcef.so
  natives_blob.bin
  snapshot_blob.bin
  )

# List of CEF resource files.
set(CEF_RESOURCE_FILES
  cef.pak
  cef_100_percent.pak
  cef_200_percent.pak
  cef_extensions.pak
  devtools_resources.pak
  icudtl.dat
  locales
)


########################################################################################################################
set(CEF_LIBTYPE                 SHARED)
set(CEF_COMPILER_FLAGS          "-fno-strict-aliasing -fPIC -fstack-protector -funwind-tables -fvisibility=hidden --param=ssp-buffer-size=4 -pipe -pthread -Wall -Werror -Wno-missing-field-initializers -Wno-unused-parameter")
set(CEF_C_COMPILER_FLAGS        "-std=C99")
set(CEF_CXX_COMPILER_FLAGS      "-fno-exceptions -fno-rtti -fno-threadsafe-statics -fvisibility-inlines-hidden -std=gnu++11 -Wsign-compare -fpermissive -fexceptions")
set(CEF_COMPILER_FLAGS_DEBUG    "-O0 -g")
set(CEF_COMPILER_FLAGS_RELEASE  "-O2 -fdata-sections -ffunction-sections -fno-ident -DNDEBUG -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2")
set(CEF_LINKER_FLAGS            "-fPIC -pthread -Wl,--disable-new-dtags -Wl,--fatal-warnings -Wl,-rpath,. -Wl,-z,noexecstack -Wl,-z,now -Wl,-z,relro")
set(CEF_LINKER_FLAGS_RELEASE    "-Wl,-O1 -Wl,--as-needed -Wl,--gc-sections")

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

# -Wno-unused-local-typedefs      = Don't warn about unused local typedefs
CHECK_C_COMPILER_FLAG(-Wno-unused-local-typedefs COMPILER_SUPPORTS_NO_UNUSED_LOCAL_TYPEDEFS)
if(COMPILER_SUPPORTS_NO_UNUSED_LOCAL_TYPEDEFS)
    set(CEF_C_COMPILER_FLAGS      "${CEF_C_COMPILER_FLAGS} -Wno-unused-local-typedefs")
endif()

# -Wno-literal-suffix             = Don't warn about invalid suffixes on literals
CHECK_CXX_COMPILER_FLAG(-Wno-literal-suffix COMPILER_SUPPORTS_NO_LITERAL_SUFFIX)
if(COMPILER_SUPPORTS_NO_LITERAL_SUFFIX)
    set(CEF_CXX_COMPILER_FLAGS    "${CEF_CXX_COMPILER_FLAGS} -Wno-literal-suffix")
endif()

# -Wno-narrowing                  = Don't warn about type narrowing
CHECK_CXX_COMPILER_FLAG(-Wno-narrowing COMPILER_SUPPORTS_NO_NARROWING)
if(COMPILER_SUPPORTS_NO_NARROWING)
    set(CEF_CXX_COMPILER_FLAGS    "${CEF_CXX_COMPILER_FLAGS} -Wno-narrowing")
endif()

if(PROJECT_ARCH STREQUAL "x86_64")
    # 64-bit architecture.
    set(CEF_COMPILER_FLAGS        "${CEF_COMPILER_FLAGS} -m64 -march=x86-64")
    set(CEF_LINKER_FLAGS          "${CEF_LINKER_FLAGS} -m64")
elseif(PROJECT_ARCH STREQUAL "x86")
    # 32-bit architecture.
    set(CEF_COMPILER_FLAGS        "${CEF_COMPILER_FLAGS} -msse2 -mfpmath=sse -mmmx -m32")
    set(CEF_LINKER_FLAGS          "${CEF_LINKER_FLAGS} -m32")
endif()

# Allow the Large File Support (LFS) interface to replace the old interface.
add_definitions(-D_FILE_OFFSET_BITS=64)

set(CMAKE_C_FLAGS                     "${CEF_COMPILER_FLAGS} ${CEF_C_COMPILER_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG               "${CEF_COMPILER_FLAGS_DEBUG} ${CEF_C_COMPILER_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_RELEASE             "${CEF_COMPILER_FLAGS_RELEASE} ${CEF_C_COMPILER_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS                   "${CEF_COMPILER_FLAGS} ${CEF_CXX_COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG             "${CEF_COMPILER_FLAGS_DEBUG} ${CEF_CXX_COMPILER_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE           "${CEF_COMPILER_FLAGS_RELEASE} ${CEF_CXX_COMPILER_FLAGS_RELEASE}")
set(CMAKE_EXE_LINKER_FLAGS            "${CEF_LINKER_FLAGS} ${CEF_EXE_LINKER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG      "${CEF_LINKER_FLAGS_DEBUG} ${CEF_EXE_LINKER_FLAGS_DEBUG}")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE    "${CEF_LINKER_FLAGS_RELEASE} ${CEF_EXE_LINKER_FLAGS_RELEASE}")
set(CMAKE_SHARED_LINKER_FLAGS         "${CEF_LINKER_FLAGS} ${CEF_SHARED_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG   "${CEF_LINKER_FLAGS_DEBUG} ${CEF_SHARED_LINKER_FLAGS_DEBUG}")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CEF_LINKER_FLAGS_RELEASE} ${CEF_SHARED_LINKER_FLAGS_RELEASE}")

########################################################################################################################

#include_directories(../include)

qt5_add_resources(PHANTOMJS_SRCS resources.qrc)

#
# Shared configuration.
#

# Target executable names.
set(CEF_TARGET "phantomjs")
if(OS_MACOSX)
    set(CEF_HELPER_TARGET "phantomjs Helper")
endif()

# Logical target used to link the libcef library.
#ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")

# Determine the target output directory.
#SET_CEF_TARGET_OUT_DIR()
SET(BUILD_SHARED_LIBRARIES OFF)
#SET(CMAKE_EXE_LINKER_FLAGS "-static")
# Executable target.
add_executable(${CEF_TARGET} ${PHANTOMJS_SRCS})
add_dependencies(${CEF_TARGET} libcef_dll_wrapper)
message(STATUS "${_qt5Core_install_prefix}")
target_link_libraries(${CEF_TARGET}
  libcef_lib
  libcef_dll_wrapper
  ${CEF_STANDARD_LIBS}
  Qt5::Gui
)

if (OS_WINDOWS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -EHsc")
    if(MSVC14)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wv:18")
    endif()
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
      set(d "")
    else() # Debug build
      set(d "d")
    endif()
    if (NOT EXISTS "${_qt5Core_install_prefix}/lib/qtharfbuzzng${d}.lib")
      message(FATAL_ERROR "PhantomJS only supports building on Windows against a static Qt configured with -static -static-runtime!")
    endif()
    target_link_libraries(${CEF_TARGET}
      "${_qt5Core_install_prefix}/lib/qtharfbuzzng${d}.lib"
      "${_qt5Core_install_prefix}/lib/qtpcre${d}.lib"
      "${_qt5Core_install_prefix}/lib/qtfreetype${d}.lib"
      "${_qt5Core_install_prefix}/lib/Qt5PlatformSupport${d}.lib"
      "${_qt5Core_install_prefix}/plugins/platforms/qwindows${d}.lib"
      "Ws2_32.lib"
      "Imm32.lib"
      "Winmm.lib"
    )
endif()

add_library(libcef_dll_wrapper STATIC IMPORTED)
set_property(TARGET libcef_dll_wrapper PROPERTY IMPORTED_LOCATION ${install_dir}/src/project_cef-build/libcef_dll/libcef_dll_wrapper.a)
add_library(libcef STATIC IMPORTED)
set_property(TARGET libcef PROPERTY IMPORTED_LOCATION ${install_dir}/src/project_cef/Release/libcef.so)
add_dependencies(${CEF_TARGET} project_cef)

include_directories(${install_dir}/src/project_cef)
#add_subdirectory(../cef_binary_3.2526.1332.gf7d9457_linux64/libcef_dll)
#add_library(libcef_dll_wrapper STATIC IMPORTED)
#set_property(TARGET libcef_dll_wrapper PROPERTY IMPORTED_LOCATION /home/aknuth/Downloads/cef_binary_3.2556.1356.g9bc8da1_linux64/libcef_dll/libcef_dll_wrapper.a)
#add_library(libcef STATIC IMPORTED)
#set_property(TARGET libcef PROPERTY IMPORTED_LOCATION /home/aknuth/cef_binary_3.2526.1332.gf7d9457_linux64/Release/libcef.so)
#add_library(libcef STATIC IMPORTED)
#set_property(TARGET libcef PROPERTY IMPORTED_LOCATION /home/aknuth/Downloads/cef_binary_3.2556.1356.g9bc8da1_linux64/Release/libcef.so)

message(STATUS "${_qt5Core_install_prefix}")
target_link_libraries(${CEF_TARGET}
        libcef_dll_wrapper
        libcef
        Qt5::Gui
        Qt5::PrintSupport
        )


# Set rpath so that libraries can be placed next to the executable.
#set_target_properties(${CEF_TARGET} PROPERTIES INSTALL_RPATH "$ORIGIN")
#set_target_properties(${CEF_TARGET} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
set_target_properties(${CEF_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/rel)

# Copy binary and resource files to the target output directory.
COPY_FILES("${CEF_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" rel)
COPY_FILES("${CEF_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" rel)
#COPY_FILES("${CEF_TARGET}" "locales" "${CEF_RESOURCE_DIR}" rel)
#file(COPY ${CEF_BINARY_FILES} DESTINATION ${CMAKE_BINARY_DIR}/rel)
#file(COPY ${CEF_RESOURCE_FILES} DESTINATION ${CMAKE_BINARY_DIR}/rel)
#file(COPY ${CEF_RESOURCE_DIR}/locales DESTINATION rel)

#
# Linux configuration.
#

#COPY_FILES("${CEF_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
#COPY_FILES("${CEF_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")
