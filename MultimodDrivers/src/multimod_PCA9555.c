// multimod_OPT3001.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for OPT3001 functions

/************************************Includes***************************************/

#include "../multimod_PCA9555.h"

#include <stdint.h>
#include "../multimod_i2c.h"

/************************************Includes***************************************/

/********************************Public Functions***********************************/

uint16_t PCA9555_GetInput(uint32_t mod, uint8_t addr) {
    uint8_t data[2];
    I2C_WriteSingle(mod, addr, 0x00);
    I2C_ReadMultiple(mod, addr, data, 2);

    return (data[1] << 8 | data[0]);
}

void PCA9555_SetPinDir(uint32_t mod, uint8_t addr, uint16_t pins) {
    uint8_t data[] = {PCA9555_CONFIG_ADDR, pins & 0xFF, (pins >> 8) & 0xFF};
    I2C_WriteMultiple(mod, addr, data, 3);
}

void PCA9555_SetPinPol(uint32_t mod, uint8_t addr, uint16_t pins) {
    uint8_t data[] = {PCA9555_POLINV_ADDR, pins & 0xFF, (pins >> 8) & 0xFF};
    I2C_WriteMultiple(mod, addr, data, 3);
}
void PCA9555_SetOutput(uint32_t mod, uint8_t addr, uint16_t pins) {
    uint8_t data[] = {PCA9555_OUTPUT_ADDR, pins & 0xFF, (pins >> 8) & 0xFF};
    I2C_WriteMultiple(mod, addr, data, 3);
}



uint16_t PCA9555_ReadReg(uint32_t mod, uint8_t gpio_module_addr, uint8_t addr) {
    uint8_t data[2];
    I2C_WriteSingle(I2C_A_BASE, gpio_module_addr, addr);
    I2C_ReadMultiple(I2C_A_BASE, gpio_module_addr, data, 2);

    return (data[1] << 8 | data[0] & 0xFF);
}

/********************************Public Functions***********************************/
