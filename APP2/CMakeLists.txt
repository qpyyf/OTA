cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()
#set(CMAKE_BUILD_TYPE "Debug")
# Set the project name
set(CMAKE_PROJECT_NAME APP2)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

#配置浮点数打印支持
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -u _printf_float")
# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME}
        CanFestival/Src/canfestival.c
        CanFestival/Inc/canfestival.h
        CanFestival/Src/TestSlave.c
        CanFestival/Inc/TestSlave.h
        Core/Src/flash.c
        Core/Src/MyCRC.cpp
        Core/Inc/MyCRC.h


)

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
        Core/Src/FreeRTOS_Demo.c
        Core/Src/mqtt.c
        Core/Inc/mqtt.h
        Core/Src/cJSON.c
        Core/Src/i2c_soft.c
        Core/Src/aht20.c
        Core/Src/ina226.c
        Core/Src/oled.c
        Core/Src/iwdg.c
        Core/Src/SD3078.c
        Core/Src/fifo.c
        Core/Src/wifi4g.c
        #################modbus##########################
        Core/Src/modbus.c
        freeModBus/ascii/mbascii.c
        freeModBus/functions/mbfunccoils.c
        freeModBus/functions/mbfuncdiag.c
        freeModBus/functions/mbfuncdisc.c
        freeModBus/functions/mbfuncholding.c
        freeModBus/functions/mbfuncinput.c
        freeModBus/functions/mbfuncother.c
        freeModBus/functions/mbutils.c
        freeModBus/rtu/mbcrc.c
        freeModBus/rtu/mbrtu.c
        freeModBus/tcp/mbtcp.c
        freeModBus/mb.c
        freeModBus/port.c
        freeModBus/portevent.c
        freeModBus/portserial.c
        freeModBus/porttimer.c
        ######################modbus_END######################
        ######################canopen######################
        CanFestival/Src/dcf.c
        CanFestival/Src/emcy.c
        CanFestival/Src/lifegrd.c
        CanFestival/Src/lss.c
        CanFestival/Src/nmtMaster.c
        CanFestival/Src/nmtSlave.c
        CanFestival/Src/objacces.c
        CanFestival/Src/pdo.c
        CanFestival/Src/sdo.c
        CanFestival/Src/states.c
        CanFestival/Src/sync.c
        CanFestival/Src/timer.c
        ######################canopen_END######################
        ##########################FREERTOS####################################
        # Add FreeRTOS sources
        # Add FreeRTOS demo source
        Core/Src/FreeRTOS_Demo.c
        # Add FreeRTOS portable source
        FreeRTOS/protable/heap_4.c
        FreeRTOS/protable/port.c

        # Add FreeRTOS source source
        FreeRTOS/source/croutine.c
        FreeRTOS/source/event_groups.c
        FreeRTOS/source/list.c
        FreeRTOS/source/queue.c
        FreeRTOS/source/stream_buffer.c
        FreeRTOS/source/tasks.c
        FreeRTOS/source/timers.c
        ######################FREERTOS_END#######################


)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
        ####################modbus####################
        freeModBus/ascii
        freeModBus/rtu
        freeModBus/include
        freeModBus
        ##################canopen##################
        CanFestival/Inc/stm32
        CanFestival/Inc
        #${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/main.c
        ######################canopen_END######################
        ###############FreeRTOS######################
        # Add FreeRTOS include paths
        FreeRTOS/protable
        include
        Core/Inc


)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
