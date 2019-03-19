#ifndef __GPIO_H__
#define __GPIO_H__

#include <em_device.h>
#include "utils.h"

<<<<<<< HEAD
// LED MACROS
#define LED_ON()        PERI_REG_BIT_SET(&GPIO->P[0].DOUT) = BIT(0)
#define LED_OFF()       PERI_REG_BIT_CLEAR(&GPIO->P[0].DOUT) = BIT(0)
=======
#define LED_ON()        PERI_REG_BIT_SET(&(GPIO->P[0].DOUT)) = BIT(0)
#define LED_OFF()       PERI_REG_BIT_CLEAR(&(GPIO->P[0].DOUT)) = BIT(0)
>>>>>>> 5068d27c15023745c96403bc4165dbdcc6e7099a
#define LED_TOGGLE()    GPIO->P[0].DOUTTGL = BIT(0);

// PN532 MACROS
#define PN532_RESET()       //PERI_REG_BIT_CLEAR(&(GPIO->P[0].DOUT)) = BIT(2)
#define PN532_UNRESET()     //PERI_REG_BIT_SET(&(GPIO->P[0].DOUT)) = BIT(2)
#define PN532_SELECT()      PERI_REG_BIT_CLEAR(&(GPIO->P[4].DOUT)) = BIT(13)
#define PN532_UNSELECT()    PERI_REG_BIT_SET(&(GPIO->P[4].DOUT)) = BIT(13)
#define PN532_READY()       PERI_REG_BIT(&(GPIO->P[0].DIN, 2)

void gpio_init();

#endif  // __GPIO_H__
