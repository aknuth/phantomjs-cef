# Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

find_package(Qt5 NO_MODULE REQUIRED COMPONENTS Gui PrintSupport)

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
	
if (OS_LINUX)
  message(ERET)
    LIST(APPEND PHANTOMJS_SRCS
            keyevents_linux.cpp
            )
endif()

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
ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")

# Determine the target output directory.
SET_CEF_TARGET_OUT_DIR()

# Executable target.
add_executable(${CEF_TARGET} ${PHANTOMJS_SRCS})
add_dependencies(${CEF_TARGET} libcef_dll_wrapper)
message(STATUS "${_qt5Core_install_prefix}")
target_link_libraries(${CEF_TARGET}
        libcef_lib
        libcef_dll_wrapper
        ${CEF_STANDARD_LIBS}
        Qt5::Gui
        Qt5::PrintSupport
        )

if (OS_WINDOWS)
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
            )
endif()

if(OS_WINDOWS AND USE_SANDBOX)
    # Logical target used to link the cef_sandbox library.
    ADD_LOGICAL_TARGET("cef_sandbox_lib" "${CEF_SANDBOX_LIB_DEBUG}" "${CEF_SANDBOX_LIB_RELEASE}")
    target_link_libraries(${CEF_TARGET} cef_sandbox_lib ${CEF_SANDBOX_STANDARD_LIBS})
endif()

# Set rpath so that libraries can be placed next to the executable.
set_target_properties(${CEF_TARGET} PROPERTIES INSTALL_RPATH "$ORIGIN")
set_target_properties(${CEF_TARGET} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
set_target_properties(${CEF_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CEF_TARGET_OUT_DIR})

# Copy binary and resource files to the target output directory.
COPY_FILES("${CEF_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
COPY_FILES("${CEF_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")

#
# Linux configuration.
#

if(OS_LINUX)
    # Set SUID permissions on the chrome-sandbox target.
    #SET_LINUX_SUID_PERMISSIONS("${CEF_TARGET}" "${CEF_TARGET_OUT_DIR}/chrome-sandbox")
endif()
