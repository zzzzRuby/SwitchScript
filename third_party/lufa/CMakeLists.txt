cmake_minimum_required(VERSION 3.21)
project(LUFA C)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "AVR")
    set(LUFA_ARCH AVR8)
endif()

if (NOT DEFINED LUFA_ARCH)
    message(FATAL_ERROR "LUFA is not supported")
endif()

set(LUFA_F_CPU ${CMAKE_AVR_F_CPU})
if (NOT DEFINED LUFA_F_USB)
    set(LUFA_F_USB ${LUFA_F_CPU})
endif()

if (NOT DEFINED LUFA_BOARD)
    set(LUFA_BOARD "NONE")
endif()

set(LUFA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lufa/LUFA)

set(LUFAUsbCommon_SOURCES
    ${LUFA_PATH}/Drivers/USB/Core/${LUFA_ARCH}/USBController_${LUFA_ARCH}.c
    ${LUFA_PATH}/Drivers/USB/Core/${LUFA_ARCH}/USBInterrupt_${LUFA_ARCH}.c
    ${LUFA_PATH}/Drivers/USB/Core/ConfigDescriptors.c
    ${LUFA_PATH}/Drivers/USB/Core/Events.c
    ${LUFA_PATH}/Drivers/USB/Core/USBTask.c
    ${LUFA_PATH}/Drivers/USB/Class/Common/HIDParser.c)

set(LUFAUsbHost_SOURCES
    ${LUFA_PATH}/Drivers/USB/Core/${LUFA_ARCH}/Host_${LUFA_ARCH}.c
    ${LUFA_PATH}/Drivers/USB/Core/${LUFA_ARCH}/Pipe_${LUFA_ARCH}.c
    ${LUFA_PATH}/Drivers/USB/Core/${LUFA_ARCH}/PipeStream_${LUFA_ARCH}.c
    ${LUFA_PATH}/Drivers/USB/Core/HostStandardReq.c)

set(LUFAUsbDevice_SOURCES
    ${LUFA_PATH}/Drivers/USB/Core/${LUFA_ARCH}/Device_${LUFA_ARCH}.c
    ${LUFA_PATH}/Drivers/USB/Core/${LUFA_ARCH}/Endpoint_${LUFA_ARCH}.c
    ${LUFA_PATH}/Drivers/USB/Core/${LUFA_ARCH}/EndpointStream_${LUFA_ARCH}.c
    ${LUFA_PATH}/Drivers/USB/Core/DeviceStandardReq.c)

set(LUFAUsbClassDevice_SOURCES
    ${LUFA_PATH}/Drivers/USB/Class/Device/AudioClassDevice.c
    ${LUFA_PATH}/Drivers/USB/Class/Device/CDCClassDevice.c
    ${LUFA_PATH}/Drivers/USB/Class/Device/HIDClassDevice.c
    ${LUFA_PATH}/Drivers/USB/Class/Device/MassStorageClassDevice.c
    ${LUFA_PATH}/Drivers/USB/Class/Device/MIDIClassDevice.c
    ${LUFA_PATH}/Drivers/USB/Class/Device/PrinterClassDevice.c
    ${LUFA_PATH}/Drivers/USB/Class/Device/RNDISClassDevice.c)
    
set(LUFAUsbClassHost_SOURCES
    ${LUFA_PATH}/Drivers/USB/Class/Host/AndroidAccessoryClassHost.c
    ${LUFA_PATH}/Drivers/USB/Class/Host/AudioClassHost.c
    ${LUFA_PATH}/Drivers/USB/Class/Host/CDCClassHost.c
    ${LUFA_PATH}/Drivers/USB/Class/Host/HIDClassHost.c
    ${LUFA_PATH}/Drivers/USB/Class/Host/MassStorageClassHost.c
    ${LUFA_PATH}/Drivers/USB/Class/Host/MIDIClassHost.c
    ${LUFA_PATH}/Drivers/USB/Class/Host/PrinterClassHost.c
    ${LUFA_PATH}/Drivers/USB/Class/Host/RNDISClassHost.c
    ${LUFA_PATH}/Drivers/USB/Class/Host/StillImageClassHost.c)

set(LUFATemperature_SOURCES ${LUFA_PATH}/Drivers/Board/Temperature.c)

set(LUFASerial_SOURCES ${LUFA_PATH}/Drivers/Peripheral/${LUFA_ARCH}/Serial_${LUFA_ARCH}.c)

set(LUFATwi_SOURCES ${LUFA_PATH}/Drivers/Peripheral/${LUFA_ARCH}/TWI_${LUFA_ARCH}.c)

if ("${LUFA_ARCH}" STREQUAL "UC3")
    enable_language(ASM)

    set(LUFAPlatform_SOURCES
        ${LUFA_PATH}/Platform/UC3/Exception.S
        ${LUFA_PATH}/Platform/UC3/InterruptManagement.c)
else()
    set(LUFAPlatform_SOURCES )
endif()

macro(lufa_library target)
    add_library(${target} INTERFACE)
    target_sources(${target} INTERFACE ${ARGN})
    target_include_directories(${target} INTERFACE ${LUFA_PATH}/..)
    target_compile_definitions(${target} INTERFACE ARCH=ARCH_${LUFA_ARCH} BOARD=BOARD_${LUFA_BOARD} F_USB=${LUFA_F_USB}UL)
endmacro()

lufa_library(LUFAUsbCommon ${LUFAUsbCommon_SOURCES})
lufa_library(LUFAUsbHost ${LUFAUsbHost_SOURCES} ${LUFAUsbCommon_SOURCES})
lufa_library(LUFAUsbDevice ${LUFAUsbDevice_SOURCES} ${LUFAUsbCommon_SOURCES})
lufa_library(LUFAUsbClassDevice ${LUFAUsbClassDevice_SOURCES})
lufa_library(LUFAUsbClassHost ${LUFAUsbClassHost_SOURCES})
lufa_library(LUFAUsb ${LUFAUsbDevice_SOURCES} ${LUFAUsbHost_SOURCES} ${LUFAUsbCommon_SOURCES})
lufa_library(LUFAUsbClass ${LUFAUsbClassDevice_SOURCES} ${LUFAUsbClassHost_SOURCES})
lufa_library(LUFATemperature ${LUFATemperature_SOURCES})
lufa_library(LUFASerial ${LUFASerial_SOURCES})
lufa_library(LUFATwi ${LUFATwi_SOURCES})
lufa_library(LUFAPlatform ${LUFAPlatform_SOURCES})
lufa_library(LUFA ${LUFAUsbDevice_SOURCES} ${LUFAUsbHost_SOURCES} ${LUFAUsbCommon_SOURCES}
    ${LUFAUsbClassDevice_SOURCES} ${LUFAUsbClassHost_SOURCES}
    ${LUFATemperature_SOURCES} ${LUFASerial_SOURCES} ${LUFATwi_SOURCES} ${LUFAPlatform_SOURCES})