#ifndef _DIO_H_
#define _DIO_H_


#include <stdint.h>
#include <sys/_intsup.h>

#include "stm32f4xx_ll_gpio.h"

#define DIO_PORT_A  (GPIOA)
#define DIO_PORT_B  (GPIOB)
#define DIO_PORT_C  (GPIOC)
#define DIO_PORT_D  (GPIOD)
#define DIO_PORT_E  (GPIOE)
#define DIO_PORT_F  (GPIOF)
#define DIO_PORT_G  (GPIOG)
#define DIO_PORT_H  (GPIOH)

#define DIO_PIN_0   (LL_GPIO_PIN_0)
#define DIO_PIN_1   (LL_GPIO_PIN_1)
#define DIO_PIN_2   (LL_GPIO_PIN_2)
#define DIO_PIN_3   (LL_GPIO_PIN_3)
#define DIO_PIN_4   (LL_GPIO_PIN_4)
#define DIO_PIN_5   (LL_GPIO_PIN_5)
#define DIO_PIN_6   (LL_GPIO_PIN_6)
#define DIO_PIN_7   (LL_GPIO_PIN_7)
#define DIO_PIN_8   (LL_GPIO_PIN_8)
#define DIO_PIN_9   (LL_GPIO_PIN_9)
#define DIO_PIN_10  (LL_GPIO_PIN_10)
#define DIO_PIN_11  (LL_GPIO_PIN_11)
#define DIO_PIN_12  (LL_GPIO_PIN_12)
#define DIO_PIN_13  (LL_GPIO_PIN_13)
#define DIO_PIN_14  (LL_GPIO_PIN_14)
#define DIO_PIN_15  (LL_GPIO_PIN_15)


#define DIO_PULL_NO     (LL_GPIO_PULL_NO)
#define DIO_PULL_UP     (LL_GPIO_PULL_UP)
#define DIO_PULL_DOWN   (LL_GPIO_PULL_DOWN)

#define DIO_SPEED_FREQ_LOW          (LL_GPIO_SPEED_FREQ_LOW)
#define DIO_SPEED_FREQ_MEDIUM       (LL_GPIO_SPEED_FREQ_MEDIUM)
#define DIO_SPEED_FREQ_HIGH         (LL_GPIO_SPEED_FREQ_HIGH)
#define DIO_SPEED_FREQ_VERY_HIGH    (LL_GPIO_SPEED_FREQ_VERY_HIGH)

#define DIO_OUTPUT_PUSHPULL     (LL_GPIO_OUTPUT_PUSHPULL)
#define DIO_OUTPUT_OPENDRAIN    (LL_GPIO_OUTPUT_OPENDRAIN)

typedef GPIO_TypeDef dio_port;

struct dio_in_info {
    const char* const name;
    dio_port* const port;
    const uint32_t pin;
    const uint32_t pull;
    const uint8_t invert;
};


struct dio_out_info {
    const char* const name;
    dio_port* const port;
    const int32_t pin;
    const int32_t pull;
    const int8_t invert;
    const int8_t init_value;
    const int32_t speed;
    const uint32_t output_type;
};

struct dio_cfg {
    const uint32_t num_inputs;
    const struct dio_in_info* const inputs;
    const uint32_t num_outputs*;
    const struct dio_out_info* const outputs;
};

int32_t dio_init(struct dio_cfg* cfg);
int32_t dio_start(void);

int32_t dio_get(uint32_t din_idx);
int32_t dio_get_out(uint32_t dout_idx);
int32_t dio_set(uint32_t dout_idx, uint32_t value);
int32_t dio_get_num_in(void);
int32_t dio_get_num_out(void);

#endif