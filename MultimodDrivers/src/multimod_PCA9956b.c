// multimod_PCA9956b.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for PCA9956b functions

/************************************Includes***************************************/

#include "../multimod_PCA9956b.h"
#include "../multimod_PCA9555.h"


#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>

#include <stdint.h>
#include "../multimod_i2c.h"

/************************************Includes***************************************/

/********************************Public Functions***********************************/

// PCA9956b_Init
// Initializes the PCA9956b, initializes the relevant output enable pins
// Return: void
void PCA9956b_Init() {
    I2C_Init(I2C_A_BASE);

    // set reset pin to known value (1)
    PCA9555_SetPinDir(I2C_A_BASE, LED_PCA9555_GPIO_ADDR, ~(0x0001));
    PCA9555_SetOutput(I2C_A_BASE, LED_PCA9555_GPIO_ADDR, ~(0x0001));
    SysCtlDelay(10000);
    PCA9555_SetOutput(I2C_A_BASE, LED_PCA9555_GPIO_ADDR, ~(0x0000));

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, OE_PIN);
    GPIOPinWrite(GPIO_PORTB_BASE, OE_PIN, 0x00);

    PCA9956b_SetAllOff();
    return;
}

// PCA9956b_SetAllMax
// Writes to the IREFALL, PWMALL registers, sets LEDs to maximum.
// Return: void
void PCA9956b_SetAllMax() {
    uint8_t data[] = {(~(AI_BIT) & IREFALL), 0xFF};
    I2C_WriteMultiple(I2C_A_BASE, PCA9956B_ADDR, data, 2);

    data[0] = (~(AI_BIT) & PWMALL);
    I2C_WriteMultiple(I2C_A_BASE, PCA9956B_ADDR, data, 2);
}

// PCA9956b_SetAllOff
// Writes to the IREFALL, PWMALL registers, sets LEDs to off.
// Return: void
void PCA9956b_SetAllOff() {
    uint8_t data[] = {(~(AI_BIT) & IREFALL), 0x00};
    I2C_WriteMultiple(I2C_A_BASE, PCA9956B_ADDR, data, 2);

    data[0] = (~(AI_BIT) & PWMALL);
    I2C_WriteMultiple(I2C_A_BASE, PCA9956B_ADDR, data, 2);
}

// PCA9956b_EnableOutput
// Sets output enable pin to true.
// Return: void
void PCA9956b_EnableOutput() {
    GPIOPinWrite(GPIO_PORTB_BASE, OE_PIN, 0x00);
}

// PCA9956b_EnableOutput
// Sets output enable pin to false.
// Return: void
void PCA9956b_DisableOutput() {
    GPIOPinWrite(GPIO_PORTB_BASE, OE_PIN, 0xFF);
}

void PCA9556b_SetLED(uint8_t LED, uint8_t PWM, uint8_t IREF) {
    uint8_t data[] = {(~(AI_BIT) & PWM0 + LED), PWM};
    I2C_WriteMultiple(I2C_A_BASE, PCA9956B_ADDR, data, 2);

    data[0] = (~(AI_BIT) & IREF0 + LED);
    data[1] = IREF;
    I2C_WriteMultiple(I2C_A_BASE, PCA9956B_ADDR, data, 2);
}

uint8_t PCA9956b_GetChipID() {
    I2C_WriteSingle(I2C_A_BASE, PCA9956B_ADDR, 0x01);
    return I2C_ReadSingle(I2C_A_BASE, PCA9956B_ADDR);
}

/********************************Public Functions***********************************/
