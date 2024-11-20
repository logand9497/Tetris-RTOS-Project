// multimod_dac.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for DAC functions

/************************************Includes***************************************/

#include "../multimod_dac.h"
#include "../multimod_PCA9555.h"

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/i2c.h"
#include "driverlib/ssi.h"
#include "driverlib/timer.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/adc.h"

/************************************Includes***************************************/

#define AUDIO_PCA9555_ADDR 0x22

/********************************Public Functions***********************************/

// MutimodDAC_Init
// Initializes ports & modules for the DAC
// Return: void
void MutimodDAC_Init(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);

    PCA9555_SetPinDir(I2C0_BASE, AUDIO_PCA9555_ADDR, 0xF0);
    PCA9555_SetOutput(I2C0_BASE, AUDIO_PCA9555_ADDR, (1 << 0) | (1 << 3) | (1 << 4));

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);

    GPIOPinConfigure(GPIO_PB4_SSI2CLK);
    GPIOPinConfigure(GPIO_PB6_SSI2RX);
    GPIOPinConfigure(GPIO_PB7_SSI2TX);

    GPIOPinTypeSSI(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_7);

    SSIConfigSetExpClk(SSI2_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 20000000, 12);

    SSIEnable(SSI2_BASE);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);

    uint32_t timer_period;

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);

    timer_period = SysCtlClockGet() / DAC_SAMPLE_FREQUENCY_HZ;
    TimerLoadSet(TIMER1_BASE, TIMER_A, timer_period - 1);

    // Register the interrupt handler
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER1_BASE, TIMER_A);
}

// MutimodDAC_Write
// Performs a write to a DAC register
// Return: void
void MutimodDAC_Write(uint32_t reg_address, uint32_t data) {
    uint32_t packet = ((reg_address << 19) & 0x1F) | ((DAC_WRITE_CMD << 17) & 0x03) | ((data << 0) & 0xFFFF);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);

    SSIDataPut(SSI2_BASE, packet >> 12);
    while (SSIBusy(SSI2_BASE));

    SSIDataPut(SSI2_BASE, packet);
    while (SSIBusy(SSI2_BASE));

    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
}

// MutimodDAC_Read
// Performs a read from a DAC register
// Return: void
uint32_t MutimodDAC_Read(uint32_t reg_address) {
    uint32_t packet = ((reg_address << 19) & 0x1F) | ((DAC_READ_CMD << 17) & 0x03);
    uint32_t data;

    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);

    SSIDataPut(SSI2_BASE, packet >> 12);
    while (SSIBusy(SSI2_BASE));

    SSIDataPut(SSI2_BASE, 0); // send 16 dummy bits
    while (SSIBusy(SSI2_BASE));

    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);

    SSIDataGet(SSI2_BASE, &data);

    return data & 0xFFFF; // extract the 16-bit data
}

/********************************Public Functions***********************************/
