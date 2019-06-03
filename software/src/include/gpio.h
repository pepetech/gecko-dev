#ifndef __GPIO_H__
#define __GPIO_H__

#include <em_device.h>
#include "cmu.h"
#include "systick.h"
#include "utils.h"
#include "nvic.h"
#include "rfm69.h"
#include "ft6x36.h"

// PN532 MACROS
#define PN532_RESET()       //PERI_REG_BIT_CLEAR(&(GPIO->P[0].DOUT)) = BIT(2)
#define PN532_UNRESET()     //PERI_REG_BIT_SET(&(GPIO->P[0].DOUT)) = BIT(2)
#define PN532_SELECT()      PERI_REG_BIT_CLEAR(&(GPIO->P[4].DOUT)) = BIT(9)
#define PN532_UNSELECT()    PERI_REG_BIT_SET(&(GPIO->P[4].DOUT)) = BIT(9)
#define PN532_READY()       1//PERI_REG_BIT(&(GPIO->P[0].DIN, 2)

// AS5048A MACROS
#define AS5048A_SELECT()      PERI_REG_BIT_CLEAR(&(GPIO->P[4].DOUT)) = BIT(9)
#define AS5048A_UNSELECT()    PERI_REG_BIT_SET(&(GPIO->P[4].DOUT)) = BIT(9)

// LED MACROS
#define LED_HIGH()          PERI_REG_BIT_SET(&(GPIO->P[1].DOUT)) = BIT(0)
#define LED_LOW()           PERI_REG_BIT_CLEAR(&(GPIO->P[1].DOUT)) = BIT(0)
#define LED_TOGGLE()        GPIO->P[1].DOUTTGL = BIT(0);

// RFM69 MACROS
#define RFM69_UNRESET()     PERI_REG_BIT_CLEAR(&(GPIO->P[0].DOUT)) = BIT(4)
#define RFM69_RESET()       PERI_REG_BIT_SET(&(GPIO->P[0].DOUT)) = BIT(4)
#define RFM69_SELECT()      PERI_REG_BIT_CLEAR(&(GPIO->P[0].DOUT)) = BIT(3)
#define RFM69_UNSELECT()    PERI_REG_BIT_SET(&(GPIO->P[0].DOUT)) = BIT(3)
#define RFM69_IRQ()         PERI_REG_BIT(&(GPIO->P[0].DIN), 6)

// GSM MACROS
#define GSM_LED_ENABLE()    PERI_REG_BIT_SET(&(GPIO->P[0].DOUT)) = BIT(10)
#define GSM_LED_DISABLE()   PERI_REG_BIT_CLEAR(&(GPIO->P[0].DOUT)) = BIT(10)
#define GSM_PWR_KEY_SET()   PERI_REG_BIT_SET(&(GPIO->P[0].DOUT)) = BIT(9)
#define GSM_PWR_KEY_RSET()  PERI_REG_BIT_CLEAR(&(GPIO->P[0].DOUT)) = BIT(9)
#define GSM_DTR_ASSERT()    PERI_REG_BIT_CLEAR(&(GPIO->P[2].DOUT)) = BIT(5)
#define GSM_DTR_DEASSERT()  PERI_REG_BIT_SET(&(GPIO->P[2].DOUT)) = BIT(5)
#define GSM_RF_SYNC()       PERI_REG_BIT(&(GPIO->P[0].DIN), 8)
#define GSM_STATUS()        PERI_REG_BIT(&(GPIO->P[0].DIN), 7)
#define GSM_RING()          PERI_REG_BIT(&(GPIO->P[2].DIN), 4)

// TFT MACROS
#define ILI9488_RESET()     PERI_REG_BIT_CLEAR(&(GPIO->P[0].DOUT)) = BIT(12)
#define ILI9488_UNRESET()   PERI_REG_BIT_SET(&(GPIO->P[0].DOUT)) = BIT(12)
#define ILI9488_SELECT()    PERI_REG_BIT_CLEAR(&(GPIO->P[3].DOUT)) = BIT(3)
#define ILI9488_UNSELECT()  PERI_REG_BIT_SET(&(GPIO->P[3].DOUT)) = BIT(3)
#define ILI9488_IRQ()       PERI_REG_BIT(&(GPIO->P[0].DIN), 13)
#define ILI9488_SETUP_DAT() PERI_REG_BIT_SET(&(GPIO->P[0].DOUT)) = BIT(14)
#define ILI9488_SETUP_CMD() PERI_REG_BIT_CLEAR(&(GPIO->P[0].DOUT)) = BIT(14)
#define TFT_BL_ON()         PERI_REG_BIT_SET(&(GPIO->P[1].DOUT)) = BIT(10)
#define TFT_BL_OFF()        PERI_REG_BIT_CLEAR(&(GPIO->P[1].DOUT)) = BIT(10)

// WIFI COPROCESSOR MACROS
#define WIFI_SELECT()       PERI_REG_BIT_CLEAR(&(GPIO->P[2].DOUT)) = BIT(8)
#define WIFI_UNSELECT()     PERI_REG_BIT_SET(&(GPIO->P[2].DOUT)) = BIT(8)
#define WIFI_IRQ()          PERI_REG_BIT(&(GPIO->P[4].DIN), 3)
#define WIFI_RESET()        PERI_REG_BIT_CLEAR(&(GPIO->P[4].DOUT)) = BIT(4)
#define WIFI_UNRESET()      PERI_REG_BIT_SET(&(GPIO->P[4].DOUT)) = BIT(4)

// CCS811 MACROS
#define CCS811_RESET()      PERI_REG_BIT_CLEAR(&(GPIO->P[4].DOUT)) = BIT(11)
#define CCS811_UNRESET()    PERI_REG_BIT_SET(&(GPIO->P[4].DOUT)) = BIT(11)
#define CCS811_SLEEP()      PERI_REG_BIT_SET(&(GPIO->P[4].DOUT)) = BIT(10)
#define CCS811_WAKE()       PERI_REG_BIT_CLEAR(&(GPIO->P[4].DOUT)) = BIT(10)
#define CCS811_IRQ()        PERI_REG_BIT(&(GPIO->P[4].DIN), 9)

void gpio_init();

void play_sound(uint16_t usFrequency, uint32_t ulTime);

#endif  // __GPIO_H__
