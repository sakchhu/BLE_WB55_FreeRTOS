# ---- FreeRTOS ---- START #

# FreeRTOS Config
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM
INTERFACE
    App/include/Config/FreeRTOS
)
target_compile_definitions(freertos_config
  INTERFACE
    projCOVERAGE_TEST=0
)
target_compile_definitions(freertos_config INTERFACE ${definitions})
target_compile_options(freertos_config INTERFACE ${options})

set(FREERTOS_HEAP "4" CACHE STRING "" FORCE)
if (CMAKE_CROSSCOMPILING)
  set(FREERTOS_PORT "GCC_ARM_CM4F" CACHE STRING "" FORCE)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/External)
include(External/ST/stMiddleWareFilePaths.cmake)
# ---- FreeRTOS ---- END #

# ---- stm32wbxx_dfp, WBXX_DFP_LIBS ---- START #

# stm32wbxx_dfp required system files
set(STM32WBXX_DFP_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/App/ble/hw_ipcc.c
    ${CMAKE_CURRENT_LIST_DIR}/App/ble/app_ble.c
    ${CMAKE_CURRENT_LIST_DIR}/App/ble/custom_app.c
    ${CMAKE_CURRENT_LIST_DIR}/App/ble/custom_stm.c
    ${CMAKE_CURRENT_LIST_DIR}/App/src/app_freertos.c
    ${CMAKE_CURRENT_LIST_DIR}/App/src/app_adapter.c
    ${CMAKE_CURRENT_LIST_DIR}/App/src/app_debug.c
    ${CMAKE_CURRENT_LIST_DIR}/App/src/app_entry.c
    ${CMAKE_CURRENT_LIST_DIR}/App/src/app_freertos.c
    ${CMAKE_CURRENT_LIST_DIR}/App/src/main.c
    ${CMAKE_CURRENT_LIST_DIR}/App/src/stm32_dfp/stm32wbxx_it.c
    ${CMAKE_CURRENT_LIST_DIR}/App/src/stm32_dfp/stm32wbxx_hal_msp.c
    ${CMAKE_CURRENT_LIST_DIR}/App/src/stm32_dfp/stm32wbxx_hal_timebase_tim2.c
    ${CMAKE_CURRENT_LIST_DIR}/App/src/stm32_dfp/sysmem.c
    ${CMAKE_CURRENT_LIST_DIR}/App/src/stm32_dfp/syscalls.c
    ${CMAKE_CURRENT_LIST_DIR}/App/src/stm32_dfp/system_stm32wbxx.c
    ${CMAKE_CURRENT_LIST_DIR}/../startup_stm32wb55xx_cm4.s
)

set(STM32WBXX_DFP_APP_INCLUDE_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/App/include
    ${CMAKE_CURRENT_LIST_DIR}/ble
    ${CMAKE_CURRENT_LIST_DIR}/App/include/stm32_dfp
)

set(BLE_APP_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/App/ble
    ${CMAKE_CURRENT_SOURCE_DIR}/App/include
)

set(ST)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Drivers)
# ---- stm32wbxx_dfp ---- END #

# ---- BSP Application and Libraries ---- START #

# !TODO: Fix the dependency linking strategies!!!

add_library(STM32_TINY_LPM)
target_sources(STM32_TINY_LPM PRIVATE
    ${ST_TINY_LPM_SRCS}
)
target_include_directories(STM32_TINY_LPM PUBLIC
    ${ST_TINY_LPM_INCLUDE_DIRS}
)
target_link_libraries(STM32_TINY_LPM PUBLIC stm32wbxx_dfp)

# add_library(APP_BLE)
# target_sources(APP_BLE PRIVATE
#     # App/ble/app_ble.c
#     # App/ble/custom_app.c
#     # App/ble/custom_stm.c
# )
# target_include_directories(APP_BLE PUBLIC ${ST_STM32_WPAN_INCLUDE_DIRS} ${BLE_APP_INCLUDE_DIRS})
# target_link_libraries(APP_BLE PUBLIC stm32wbxx_dfp STM32_TINY_LPM freertos_kernel freertos_config)

add_library(STM32_WPAN)
target_sources(STM32_WPAN PUBLIC
    ${ST_STM32_WPAN_SRCS}
)
target_include_directories(STM32_WPAN PUBLIC
    ${ST_STM32_WPAN_INCLUDE_DIRS} ${BLE_APP_INCLUDE_DIRS}
)
target_link_libraries(STM32_WPAN PUBLIC stm32wbxx_dfp STM32_TINY_LPM freertos_kernel freertos_config)

add_library(RTK_RLS)
target_sources(RTK_RLS PUBLIC
    # App/src/app_freertos.c
    # App/src/app_adapter.c
    # PUBLIC
    # App/src/app_debug.c
    # App/src/app_entry.c
    # App/src/app_freertos.c
    # App/src/hw_timerserver.c
    # App/src/main.c
    # App/src/gpio.c
    # App/src/rf.c
    # App/src/rng.c
    # App/src/rtc.c

)
target_include_directories(RTK_RLS PUBLIC App/include)
target_link_libraries(RTK_RLS PUBLIC stm32wbxx_dfp STM32_WPAN STM32_TINY_LPM freertos_kernel freertos_config)
target_link_libraries(${CMAKE_PROJECT_NAME} RTK_RLS)
# add_library()

# ---- BSP Application and Libraries ---- END #
