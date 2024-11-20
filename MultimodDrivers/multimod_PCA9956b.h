// multimod_PCA9956b.h
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// LED driver header file

#ifndef MULTIMOD_PCA9956B_H_
#define MULTIMOD_PCA9956B_H_

/************************************Includes***************************************/

#include <stdint.h>
#include <stdbool.h>

#include <inc/hw_memmap.h>
#include <inc/hw_gpio.h>

#include <driverlib/i2c.h>
#include <driverlib/gpio.h>

/************************************Includes***************************************/

/*************************************Defines***************************************/

#define PCA9956B_ADDR   0x01
#define I2C_SCL_PIN     GPIO_PIN_6
#define I2C_SDA_PIN     GPIO_PIN_7
#define OE_PIN          GPIO_PIN_5
#define LEDOUT0         0x02
#define LEDOUT5         0x07
#define PWM0            0x0A
#define PWM11           0x15
#define IREF0           0x22
#define IREF11          0x2D
#define MODE2           0x01
#define EFLAG0          0x41
#define EFLAG2          0x43
#define PWMALL          0x3F
#define IREFALL         0x40

#define AI_BIT          0x80

// Connected PCA9555
#define LED_PCA9555_GPIO_ADDR   0x23
#define LED_PCA9555_RST_PIN     0x00

/*************************************Defines***************************************/

/******************************Data Type Definitions********************************/
/******************************Data Type Definitions********************************/

/****************************Data Structure Definitions*****************************/
/****************************Data Structure Definitions*****************************/

/***********************************Externs*****************************************/
/***********************************Externs*****************************************/

/********************************Public Variables***********************************/
/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

void PCA9956b_Init(void);
void PCA9956b_SetAllMax(void);
void PCA9956b_SetAllOff(void);
void PCA9956b_EnableOutput(void);
void PCA9956b_DisableOutput(void);
void PCA9556b_ReadAll();
void PCA9556b_SetLED(uint8_t LED, uint8_t PWM, uint8_t IREF);
uint8_t PCA9956b_GetChipID(void);

/********************************Public Functions***********************************/

/*******************************Private Variables***********************************/
/*******************************Private Variables***********************************/

/*******************************Private Functions***********************************/
/*******************************Private Functions***********************************/

#endif /* MULTIMOD_PCA9956B_H_ */


