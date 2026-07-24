#ifndef RTS_RLS_BOARD_PINS_REV1_4_SOLAR_H
#define RTS_RLS_BOARD_PINS_REV1_4_SOLAR_H

/* ---- Protocol GPIO Pin Definitions ---- */

#define I2C1_SCL_PIN           GPIO_PIN_6
#define I2C1_SDA_PIN           GPIO_PIN_7

#define SPI2_SCK_PIN           GPIO_PIN_13
#define SPI2_MISO_PIN          GPIO_PIN_14
#define SPI2_MOSI_PIN          GPIO_PIN_15

#define SPI1_CLK_PIN           GPIO_PIN_5
#define SPI1_MISO_PIN          GPIO_PIN_6
#define SPI1_MOSI_PIN          GPIO_PIN_7

#define SWD_GPIO_PORT          GPIOA
#define SWD_SWDIO_PIN          GPIO_PIN_13
#define SWD_SWCLK_PIN          GPIO_PIN_14

#define SWO_GPIO_PORT          GPIOB
#define SWO_PIN                GPIO_PIN_3

/* ---- GPIOA: A121 / USART1 ---- */

/* A121 */
#define A121_GPIO_PORT         GPIOA
#define A121_SPI_INST          SPI1
#define A121_EN_PIN            GPIO_PIN_2
#define A121_INT_PIN           GPIO_PIN_3
#define A121_NSS_PIN           GPIO_PIN_4 /* Active LOW */
#define A121_CLK_PIN           SPI1_CLK_PIN
#define A121_MISO_PIN          SPI1_MISO_PIN
#define A121_MOSI_PIN          SPI1_MOSI_PIN

/* USART1 */
#define USART1_GPIO_PORT       GPIOA
#define USART1_DTR_PIN         GPIO_PIN_8
#define USART1_TX_PIN          GPIO_PIN_9
#define USART1_RX_PIN          GPIO_PIN_10
#define USART1_CTS_PIN         GPIO_PIN_11
#define USART1_RTS_PIN         GPIO_PIN_12

/* ---- GPIOB: CHARGER / IMU / FLASH / LORA ---- */

/* Charger */
#define CHARGER_GPIO_PORT      GPIOB
#define CHARGER_EN_PIN         GPIO_PIN_0
#define CHARGER_INT_PIN        GPIO_PIN_1
#define CHARGER_QON_PIN        GPIO_PIN_2

#define CHARGER_I2C_INST       I2C1
#define CHARGER_SCL_PIN        I2C1_SCL_PIN
#define CHARGER_SDA_PIN        I2C1_SDA_PIN

/* IMU : LSM6DSV16X */
#define IMU_GPIO_PORT          GPIOB
#define IMU_I2C_INST           I2C1
#define IMU_SCL_PIN            I2C1_SCL_PIN
#define IMU_SDA_PIN            I2C1_SDA_PIN
#define IMU_INT1_PIN           GPIO_PIN_4
#define IMU_INT2_PIN           GPIO_PIN_5

/* Flash */
#define FLASH_GPIO_PORT        GPIOB
#define FLASH_SPI_INST         SPI2
#define FLASH_NSS_PIN          GPIO_PIN_9
#define FLASH_SCK_PIN          SPI2_SCK_PIN
#define FLASH_MISO_PIN         SPI2_MISO_PIN
#define FLASH_MOSI_PIN         SPI2_MOSI_PIN

/* LoRa */
#define LORA_GPIO_PORT         GPIOB
#define LORA_RST_PIN           GPIO_PIN_8
#define LORA_BUSY_PIN          GPIO_PIN_10
#define LORA_IRQ_PIN           GPIO_PIN_11

#define LORA_SPI_INST          SPI2
#define LORA_NSS_PIN           GPIO_PIN_12
#define LORA_SCK_PIN           SPI2_SCK_PIN
#define LORA_MISO_PIN          SPI2_MISO_PIN
#define LORA_MOSI_PIN          SPI2_MOSI_PIN

/* ---- GPIOC: LEDs / EXT_SERIAL_LPUART / Interface Connectors ---- */

/* LEDs */
#define LED_GPIO_PORT          GPIOC
#define LED_BLUE_GPIO_PIN      GPIO_PIN_4
#define LED_GREEN_GPIO_PIN     GPIO_PIN_5
#define LED_RED_GPIO_PIN       GPIO_PIN_6

/* External Serial Connection (LPUART) */
#define EXT_SERIAL_GPIO_PORT   GPIOC
#define EXT_SERIAL_LPUART_PORT LPUART1
#define EXT_SERIAL_RX_PIN      GPIO_PIN_0
#define EXT_SERIAL_TX_PIN      GPIO_PIN_1
#define EXT_SERIAL_TX_EN_PIN   GPIO_PIN_2
#define EXT_SERIAL_MUX_SEL_PIN GPIO_PIN_3

/* ---- Interface Connectors ---- */
#define BTYPE_GPIO_PORT        GPIOC
#define BTYPE_A_GPIO_PIN       GPIO_PIN_11
#define BTYPE_B_GPIO_PIN       GPIO_PIN_12

/* ---- GPIOE: User Button ---- */

/* User Button */
#define USER_BUTTON_GPIO_PORT  GPIOE
#define USER_BUTTON_GPIO_PIN   GPIO_PIN_4

#endif /* RTS_RLS_BOARD_PINS_REV1_4_SOLAR_H */