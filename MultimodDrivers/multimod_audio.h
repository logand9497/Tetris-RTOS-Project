// multimod_joystick.h
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Declarations for multimod joystick functions

#ifndef MULTIMOD_AUDIO_H_
#define MULTIMOD_AUDIO_H_

/************************************Includes***************************************/

#include <stdint.h>
#include <stdbool.h>

/************************************Includes***************************************/

/*************************************Defines***************************************/

#define AUDIO_PCA9555_GPIO_ADDR     0x22

#define DAC_WRITE_CMD               0x03
#define DAC_READ_CMD                0x00
#define DAC_CS                      GPIO_PIN_1

#define WAVEFORM_AMPLITUDE 2047 // half of 4095 (12 bit DAC)
#define WAVEFORM_OFFSET 2048 // middle of 4095 (12 bit DAC)

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

void AudioInput_Init(void);
void AudioOutput_Init(void);
void AudioConversionTimer_Enable(uint32_t frequency);
void AudioConversionTimer_Disable();
void AudioDAC_Write(uint32_t address, uint32_t data);
uint32_t AudioDAC_Read(uint32_t address);

void AudioDAC_Select(void);
void AudioDAC_Deselect(void);

void Audio_PCA9555_Write(uint8_t reg, uint8_t data);
uint8_t Audio_PCA9555_Read(uint8_t reg);

/********************************Public Functions***********************************/

/*******************************Private Variables***********************************/
/*******************************Private Variables***********************************/

/*******************************Private Functions***********************************/
/*******************************Private Functions***********************************/

#endif /* MULTIMOD_AUDIO_H_ */



