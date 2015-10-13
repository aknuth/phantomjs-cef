# Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

#
# Source files.
#

# phantomjs sources.
set(PHANTOMJS_SRCS
  app.cpp
  handler.cpp
  print_handler.cpp
  )
set(PHANTOMJS_SRCS_LINUX
  phantomjs_linux.cpp
  handler_linux.cpp
  )
set(PHANTOMJS_SRCS_MACOSX
  phantomjs_mac.mm
  handler_mac.mm
  )
set(PHANTOMJS_SRCS_WINDOWS
  phantomjs.exe.manifest
  phantomjs.rc
  phantomjs_win.cpp
  resource.h
  handler_win.cpp
  )
APPEND_PLATFORM_SOURCES(PHANTOMJS_SRCS)
source_group(phantomjs FILES ${PHANTOMJS_SRCS})

set(PHANTOMJS_RES_SRCS_WINDOWS
  res/phantomjs.ico
  res/small.ico
  )
APPEND_PLATFORM_SOURCES(PHANTOMJS_RES_SRCS)
source_group(phantomjs\\\\res FILES ${PHANTOMJS_RES_SRCS})

set(PHANTOMJS_SRCS
  ${PHANTOMJS_SRCS}
  ${PHANTOMJS_RES_SRCS}
  )

# phantomjs helper sources.
set(PHANTOMJS_HELPER_SRCS_MACOSX
  process_helper_mac.cpp
  )
APPEND_PLATFORM_SOURCES(PHANTOMJS_HELPER_SRCS)
source_group(phantomjs FILES ${PHANTOMJS_HELPER_SRCS})

# phantomjs resources.
set(PHANTOMJS_RESOURCES_MAC_SRCS_MACOSX
  mac/Info.plist
  mac/phantomjs.icns
  )
APPEND_PLATFORM_SOURCES(PHANTOMJS_RESOURCES_MAC_SRCS)
source_group(phantomjs\\\\mac FILES ${PHANTOMJS_RESOURCES_MAC_SRCS})

set(PHANTOMJS_RESOURCES_MAC_ENGLISH_LPROJ_SRCS_MACOSX
  mac/English.lproj/InfoPlist.strings
  mac/English.lproj/MainMenu.xib
  )
APPEND_PLATFORM_SOURCES(PHANTOMJS_RESOURCES_MAC_ENGLISH_LPROJ_SRCS)
source_group(phantomjs\\\\mac\\\\English.lproj FILES ${PHANTOMJS_RESOURCES_MAC_ENGLISH_LPROJ_SRCS})

set(PHANTOMJS_RESOURCES_SRCS
  ${PHANTOMJS_RESOURCES_MAC_SRCS}
  ${PHANTOMJS_RESOURCES_MAC_ENGLISH_LPROJ_SRCS}
  )


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


#
# Linux configuration.
#

if(OS_LINUX)
  # Executable target.
  add_executable(${CEF_TARGET} ${PHANTOMJS_SRCS})
  add_dependencies(${CEF_TARGET} libcef_dll_wrapper)
  target_link_libraries(${CEF_TARGET} libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})

  # Set rpath so that libraries can be placed next to the executable.
  set_target_properties(${CEF_TARGET} PROPERTIES INSTALL_RPATH "$ORIGIN")
  set_target_properties(${CEF_TARGET} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
  set_target_properties(${CEF_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CEF_TARGET_OUT_DIR})

  # Copy binary and resource files to the target output directory.
  COPY_FILES("${CEF_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
  COPY_FILES("${CEF_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")

  # Set SUID permissions on the chrome-sandbox target.
  SET_LINUX_SUID_PERMISSIONS("${CEF_TARGET}" "${CEF_TARGET_OUT_DIR}/chrome-sandbox")
endif()


#
# Mac OS X configuration.
#

if(OS_MACOSX)
  # Output paths for the app bundles.
  set(CEF_APP "${CEF_TARGET_OUT_DIR}/${CEF_TARGET}.app")
  set(CEF_HELPER_APP "${CEF_TARGET_OUT_DIR}/${CEF_HELPER_TARGET}.app")

  # Variable referenced from Info.plist files.
  set(PRODUCT_NAME "${CEF_TARGET}")

  # Helper executable target.
  add_executable(${CEF_HELPER_TARGET} MACOSX_BUNDLE ${PHANTOMJS_HELPER_SRCS})
  add_dependencies(${CEF_HELPER_TARGET} libcef_dll_wrapper)
  target_link_libraries(${CEF_HELPER_TARGET} libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})
  set_target_properties(${CEF_HELPER_TARGET} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/mac/helper-Info.plist
    )

  # Fix the framework link in the helper executable.
  FIX_MACOSX_HELPER_FRAMEWORK_LINK(${CEF_HELPER_TARGET} ${CEF_HELPER_APP})

  # Main executable target.
  add_executable(${CEF_TARGET} MACOSX_BUNDLE ${PHANTOMJS_RESOURCES_SRCS} ${PHANTOMJS_SRCS})
  add_dependencies(${CEF_TARGET} libcef_dll_wrapper "${CEF_HELPER_TARGET}")
  target_link_libraries(${CEF_TARGET} libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})
  set_target_properties(${CEF_TARGET} PROPERTIES
    RESOURCE "${PHANTOMJS_RESOURCES_SRCS}"
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/mac/Info.plist
    )

  # Copy files into the main app bundle.
  add_custom_command(
    TARGET ${CEF_TARGET}
    POST_BUILD
    # Copy the helper app bundle into the Frameworks directory.
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CEF_HELPER_APP}"
            "${CEF_APP}/Contents/Frameworks/${CEF_HELPER_TARGET}.app"
    # Copy the CEF framework into the Frameworks directory.
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CEF_BINARY_DIR}/Chromium Embedded Framework.framework"
            "${CEF_APP}/Contents/Frameworks/Chromium Embedded Framework.framework"
    VERBATIM
    )

  # Fix the framework link in the main executable.
  FIX_MACOSX_MAIN_FRAMEWORK_LINK(${CEF_TARGET} ${CEF_APP})

  # Make the other helper app bundles.
  MAKE_MACOSX_HELPERS(${CEF_TARGET} ${CEF_APP})

  if(NOT ${CMAKE_GENERATOR} STREQUAL "Xcode")
    # Manually process and copy over resource files.
    # The Xcode generator handles this via the set_target_properties RESOURCE directive.
    set(PREFIXES "mac/")  # Remove these prefixes from input file paths.
    COPY_MACOSX_RESOURCES("${PHANTOMJS_RESOURCES_SRCS}" "${PREFIXES}" "${CEF_TARGET}" "${CMAKE_CURRENT_SOURCE_DIR}" "${CEF_APP}")
  endif()
endif()


#
# Windows configuration.
#

if(OS_WINDOWS)
  # Executable target.
  add_executable(${CEF_TARGET} WIN32 ${PHANTOMJS_SRCS})
  add_dependencies(${CEF_TARGET} libcef_dll_wrapper)
  target_link_libraries(${CEF_TARGET} libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})

  if(USE_SANDBOX)
    # Logical target used to link the cef_sandbox library.
    ADD_LOGICAL_TARGET("cef_sandbox_lib" "${CEF_SANDBOX_LIB_DEBUG}" "${CEF_SANDBOX_LIB_RELEASE}")
    target_link_libraries(${CEF_TARGET} cef_sandbox_lib ${CEF_SANDBOX_STANDARD_LIBS})
  endif()

  # Add the custom manifest files to the executable.
  ADD_WINDOWS_MANIFEST("${CMAKE_CURRENT_SOURCE_DIR}" "${CEF_TARGET}")

  # Copy binary and resource files to the target output directory.
  COPY_FILES("${CEF_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
  COPY_FILES("${CEF_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")
endif()
