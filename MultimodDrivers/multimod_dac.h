// multimod_dac.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Declarations for multimod dac functions

#ifndef MULTIMOD_DAC_H_
#define MULTIMOD_DAC_H_

/************************************Includes***************************************/

#include <stdint.h>
#include <stdbool.h>

/************************************Includes***************************************/

/*************************************Defines***************************************/

#define DAC_WRITE_CMD 0x03
#define DAC_READ_CMD 0x00
#define DAC_OUT_REG 0x00
#define DAC_SAMPLE_FREQUENCY_HZ 200
#define DAC_INTERRUPT INT_TIMER1A

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

void MutimodDAC_Init(void);
void MutimodDAC_Write(uint32_t reg_address, uint32_t data);
uint32_t MutimodDAC_Read(uint32_t reg_address);

/********************************Public Functions***********************************/

/*******************************Private Variables***********************************/
/*******************************Private Variables***********************************/

/*******************************Private Functions***********************************/
/*******************************Private Functions***********************************/

#endif /* MULTIMOD_DAC_H_ */
