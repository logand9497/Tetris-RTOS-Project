// multimod_PCA9555b.h
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// GPIO Expander Header File

#ifndef MULTIMOD_PCA9555_H_
#define MULTIMOD_PCA9555_H_

/************************************Includes***************************************/

#include <stdint.h>
#include <stdbool.h>

#include <inc/hw_memmap.h>
#include <inc/hw_gpio.h>

#include <driverlib/i2c.h>
#include <driverlib/gpio.h>

/************************************Includes***************************************/

/*************************************Defines***************************************/

#define PCA9555_INPUT_ADDR  0x00
#define PCA9555_OUTPUT_ADDR 0x02
#define PCA9555_POLINV_ADDR 0x04
#define PCA9555_CONFIG_ADDR 0x06

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

uint16_t PCA9555_GetInput(uint32_t mod, uint8_t addr);
void PCA9555_SetPinDir(uint32_t mod, uint8_t addr, uint16_t pins);
void PCA9555_SetPinPol(uint32_t mod, uint8_t addr, uint16_t pins);
void PCA9555_SetOutput(uint32_t mod, uint8_t addr, uint16_t pins);

uint16_t PCA9555_ReadReg(uint32_t mod, uint8_t gpio_module_addr, uint8_t addr);


/********************************Public Functions***********************************/

/*******************************Private Variables***********************************/
/*******************************Private Variables***********************************/

/*******************************Private Functions***********************************/
/*******************************Private Functions***********************************/

#endif /* MULTIMOD_PCA9555_H_ */


