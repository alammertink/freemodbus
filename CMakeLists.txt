cmake_minimum_required(VERSION 3.16)

# FreeModbus Library CMake configuration
# This builds the core FreeModbus library and the STM32_GENERIC port

# Define the FreeModbus library target
add_library(freemodbus STATIC)

# Include directories
target_include_directories(freemodbus PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/modbus/include
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/STM32_GENERIC/include
)

# Core FreeModbus library source files
set(FREEMODBUS_CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/modbus/mb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/modbus/mbutils.c
    # Functions
    ${CMAKE_CURRENT_SOURCE_DIR}/modbus/functions/mbfunccoils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/modbus/functions/mbfuncdiag.c
    ${CMAKE_CURRENT_SOURCE_DIR}/modbus/functions/mbfuncdisc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/modbus/functions/mbfuncholding.c
    ${CMAKE_CURRENT_SOURCE_DIR}/modbus/functions/mbfuncinput.c
    ${CMAKE_CURRENT_SOURCE_DIR}/modbus/functions/mbfuncother.c
    # RTU mode
    ${CMAKE_CURRENT_SOURCE_DIR}/modbus/rtu/mbcrc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/modbus/rtu/mbrtu.c
    # ASCII mode (include for completeness)
    ${CMAKE_CURRENT_SOURCE_DIR}/modbus/ascii/mbascii.c
)

# STM32 Generic port files
set(PORT_STM32_GENERIC_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/STM32_GENERIC/port/portevent.c
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/STM32_GENERIC/port/portserial.c
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/STM32_GENERIC/port/porttimer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/STM32_GENERIC/port/portcritical.c
)

# Add all sources to the library target
target_sources(freemodbus PRIVATE
    ${FREEMODBUS_CORE_SOURCES}
    ${PORT_STM32_GENERIC_SOURCES}
)

# Enable position-independent code (if needed)
set_property(TARGET freemodbus PROPERTY POSITION_INDEPENDENT_CODE ON)

# Set compiler options
target_compile_options(freemodbus PRIVATE
    -Wall
    -Wextra
    -ffunction-sections
    -fdata-sections
)

# Export include directories for parent project
set(FREEMODBUS_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/modbus/include
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/STM32_GENERIC/include
    PARENT_SCOPE
)
