cmake_minimum_required(VERSION 3.20)
project(USBCredProvider)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Windows-specific settings
if(WIN32 OR MINGW)
    add_definitions(-DUNICODE -D_UNICODE -DWIN32 -D_WIN32)

    # Add MinGW-w64 include paths for better IDE support
    if(MINGW)
        include_directories(SYSTEM
            /usr/x86_64-w64-mingw32/include
            /usr/share/mingw-w64/include
        )
    endif()
endif()

# Main USB Credential Provider DLL
add_library(USBCredProvider SHARED
    src/dll.cpp
    src/dll.h
    src/helpers.cpp
    src/helpers.h
    src/common.h
    src/resource.h
    src/guid.cpp
    src/guid.h
    src/usb_detector.cpp
    src/usb_detector.h
    src/config.cpp
    src/config.h
    src/USBCredential.cpp
    src/USBCredential.h
    src/USBCredentialProvider.cpp
    src/USBCredentialProvider.h
    src/USBCredProvider.def
)

# Link required Windows libraries
target_link_libraries(USBCredProvider
    advapi32
    ole32
    oleaut32
    credui
    setupapi
    uuid
    shlwapi
    winhttp
    cfgmgr32
)

# Set DLL properties
set_target_properties(USBCredProvider PROPERTIES
    OUTPUT_NAME "USBCredProvider"
    PREFIX ""
    SUFFIX ".dll"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LINK_FLAGS "/DEF:${CMAKE_CURRENT_SOURCE_DIR}/src/USBCredProvider.def"
)

# USB Device Lister Tool
add_executable(list_usb_devices
    tools/list_usb_devices.cpp
    src/usb_detector.cpp
    src/usb_detector.h
)

target_link_libraries(list_usb_devices
    setupapi
    cfgmgr32
)

set_target_properties(list_usb_devices PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tools"
)
