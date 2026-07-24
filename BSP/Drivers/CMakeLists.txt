cmake_minimum_required(VERSION 3.22)
# Enable CMake support for ASM and C languages
enable_language(C ASM)
# STM32CubeMX generated symbols (macros)
set(MX_Defines_Syms 
	USE_HAL_DRIVER 
	STM32WB55xx
    $<$<CONFIG:Debug>:DEBUG>
)

# STM32CubeMX generated include paths
set(MX_Include_Dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Inc/Legacy
    ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS/Device/ST/STM32WBxx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS/Include
)

# STM32 HAL/LL Drivers
set(STM32_Drivers_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_adc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_adc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_comp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_cortex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_crc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_crc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_cryp_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_cryp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_dma_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_exti.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_flash_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_flash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_hsem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_i2c_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_ipcc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_irda.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_iwdg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_lcd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_lptim.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_msp_template.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_pcd_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_pcd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_pka.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_pwr_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_pwr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_qspi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_rcc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_rcc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_rng.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_rtc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_rtc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_sai_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_sai.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_smartcard_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_smartcard.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_smbus_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_smbus.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_spi_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_tim_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_tim.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_timebase_rtc_alarm_template.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_timebase_rtc_wakeup_template.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_timebase_tim_template.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_tsc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_uart_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_uart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_usart_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_usart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_wwdg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_adc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_comp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_crc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_crs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_exti.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_lptim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_lpuart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_pka.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_pwr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_rcc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_rng.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_rtc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_usart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_usb.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_utils.c
)

# Project static libraries
set(WBXX_DFP_LIBS 
    STM32_Drivers
    ${TOOLCHAIN_LINK_LIBRARIES}
	
)

# Interface library for includes and symbols
add_library(stm32wbxx_dfp INTERFACE)
target_include_directories(stm32wbxx_dfp INTERFACE ${MX_Include_Dirs} ${STM32WBXX_DFP_APP_INCLUDE_DIRS})
target_compile_definitions(stm32wbxx_dfp INTERFACE ${MX_Defines_Syms})

# Create STM32_Drivers static library
add_library(STM32_Drivers OBJECT)
target_sources(STM32_Drivers PRIVATE ${STM32_Drivers_Src})
target_link_libraries(STM32_Drivers PUBLIC stm32wbxx_dfp)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${STM32WBXX_DFP_SRCS})
target_link_libraries(${CMAKE_PROJECT_NAME} ${WBXX_DFP_LIBS})

# Add the map file to the list of files to be removed with 'clean' target
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES ${CMAKE_PROJECT_NAME}.map)

# Validate that the DFP code is compatible with C standard
if((CMAKE_C_STANDARD EQUAL 90) OR (CMAKE_C_STANDARD EQUAL 99))
    message(ERROR "Generated code requires C11 or higher")
endif()
