#ifndef MULTIMOD_H_
#define MULTIMOD_H_

#include "multimod_BMI160.h"
#include "multimod_i2c.h"
#include "multimod_joystick.h"
#include "multimod_OPT3001.h"
#include "multimod_PCA9956b.h"
#include "multimod_uart.h"
#include "multimod_ST7789.h"
#include "multimod_spi.h"
#include "multimod_buttons.h"
#include "multimod_mic.h"
#include "multimod_dac.h"
#include "multimod_PCA9555.h"

static void multimod_init() {
    ST7789_Init();
    PCA9956b_Init();
    UART_Init();
    MultimodMic_Init();
    MutimodDAC_Init();
    MultimodButtons_Init();
    JOYSTICK_Init();
    JOYSTICK_IntEnable();


}

#endif /* MULTIMOD_H_ */
