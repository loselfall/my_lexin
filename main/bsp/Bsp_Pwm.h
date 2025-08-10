#ifndef __BDP_PWM_H__
#define __BDP_PWM_H__

#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (40) // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY_MAX           (8192) // (2 ** 13) = 8192
#define LEDC_FREQUENCY          (8000) // Frequency in Hertz. Set frequency at 4 kHz

void Bsp_Pwm_Init(void);
#endif /* __BDP_PWM_H__ */
