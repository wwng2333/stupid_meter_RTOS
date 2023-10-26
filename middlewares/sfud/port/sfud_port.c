/*
 * This file is part of the Serial Flash Universal Driver Library.
 *
 * Copyright (c) 2016-2018, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2016-04-23
 */

#include <sfud.h>
#include <stdarg.h>
#include "at32f421_wk_config.h"

typedef struct {
    spi_type *spix;
    gpio_type *cs_gpiox;
    uint16_t cs_gpio_pin;
} spi_user_data, *spi_user_data_t;

static spi_user_data spi2 = { .spix = SPI2, .cs_gpiox = GPIOA, .cs_gpio_pin = GPIO_PINS_15 };

static char log_buf[256];

void sfud_log_debug(const char *file, const long line, const char *format, ...);

static void rcc_configuration(spi_user_data_t spi) {
    if (spi->spix == SPI2) {
			crm_periph_clock_enable(CRM_SPI2_PERIPH_CLOCK, TRUE);
    }
}

static void gpio_configuration(spi_user_data_t spi) {
    gpio_init_type gpio_init_struct;
		gpio_default_para_init(&gpio_init_struct);

    if (spi->spix == SPI2) {
			/* configure the SCK pin */
			gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
			gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
			gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
			gpio_init_struct.gpio_pins = GPIO_PINS_1;
			gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
			gpio_init(GPIOB, &gpio_init_struct);

			gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE1, GPIO_MUX_6);

			/* configure the MISO pin */
			gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
			gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
			gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
			gpio_init_struct.gpio_pins = GPIO_PINS_4;
			gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
			gpio_init(GPIOB, &gpio_init_struct);

			gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE4, GPIO_MUX_6);

			/* configure the MOSI pin */
			gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
			gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
			gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
			gpio_init_struct.gpio_pins = GPIO_PINS_5;
			gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
			gpio_init(GPIOB, &gpio_init_struct);

			gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE5, GPIO_MUX_6);
			
			/* configure the CS pin */
			gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
			gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
			gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
			gpio_init_struct.gpio_pins = GPIO_PINS_15;
			gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
			gpio_init(GPIOA, &gpio_init_struct);
			gpio_bits_write(GPIOA, GPIO_PINS_15, FALSE);
    }
}

static void spi_configuration(spi_user_data_t spi) {
  spi_init_type spi_init_struct;
  spi_default_para_init(&spi_init_struct);
	
  /* configure param */
  spi_init_struct.transmission_mode = SPI_TRANSMIT_FULL_DUPLEX;
  spi_init_struct.master_slave_mode = SPI_MODE_MASTER;
  spi_init_struct.frame_bit_num = SPI_FRAME_8BIT;
  spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
  spi_init_struct.mclk_freq_division = SPI_MCLK_DIV_4;
  spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_HIGH;
  spi_init_struct.clock_phase = SPI_CLOCK_PHASE_2EDGE;
  spi_init_struct.cs_mode_selection = SPI_CS_SOFTWARE_MODE;
  spi_init(SPI2, &spi_init_struct);

  //spi_crc_polynomial_set(SPI2, 0x7);
  //spi_crc_enable(SPI2, FALSE);

  spi_enable(SPI2, TRUE);
}

static void spi_lock(const sfud_spi *spi) {
    __disable_irq();
}

static void spi_unlock(const sfud_spi *spi) {
    __enable_irq();
}

/**
 * SPI write data then read data
 */
static sfud_err spi_write_read(const sfud_spi *spi, const uint8_t *write_buf, size_t write_size, uint8_t *read_buf,
        size_t read_size) {
    sfud_err result = SFUD_SUCCESS;
    uint8_t send_data, read_data;
    spi_user_data_t spi_dev = (spi_user_data_t) spi->user_data;

    if (write_size) {
        SFUD_ASSERT(write_buf);
    }
    if (read_size) {
        SFUD_ASSERT(read_buf);
    }

		gpio_bits_reset(spi_dev->cs_gpiox, spi_dev->cs_gpio_pin);
    for (size_t i = 0, retry_times; i < write_size + read_size; i++) {
        if (i < write_size) {
            send_data = *write_buf++;
        } else {
            send_data = SFUD_DUMMY_DATA;
        }
        /* Send */
        retry_times = 1000;
        while (spi_i2s_flag_get(spi_dev->spix, SPI_I2S_TDBE_FLAG) == RESET) {
            SFUD_RETRY_PROCESS(NULL, retry_times, result);
        }
        if (result != SFUD_SUCCESS) {
            goto exit;
        }
        spi_i2s_data_transmit(spi_dev->spix, send_data);
        /* Recv */
        retry_times = 1000;
        while (spi_i2s_flag_get(spi_dev->spix, SPI_I2S_RDBF_FLAG) == RESET) {
            SFUD_RETRY_PROCESS(NULL, retry_times, result);
        }
        if (result != SFUD_SUCCESS) {
            goto exit;
        }
        read_data = spi_i2s_data_receive(spi_dev->spix);
        /* ???????????,??? SPI ??????????? */
        if (i >= write_size) {
            *read_buf++ = read_data;
        }
    }

exit:
    gpio_bits_set(spi_dev->cs_gpiox, spi_dev->cs_gpio_pin);

    return result;
}

#ifdef SFUD_USING_QSPI
/**
 * read flash data by QSPI
 */
static sfud_err qspi_read(const struct __sfud_spi *spi, uint32_t addr, sfud_qspi_read_cmd_format *qspi_read_cmd_format,
        uint8_t *read_buf, size_t read_size) {
    sfud_err result = SFUD_SUCCESS;

    /**
     * add your qspi read flash data code
     */

    return result;
}
#endif /* SFUD_USING_QSPI */

/* about 100 microsecond delay */
static void retry_delay_100us(void) {
//    uint32_t delay = 120;
//    while(delay--);
	delay_us(100);
}

sfud_err sfud_spi_port_init(sfud_flash *flash) {
    sfud_err result = SFUD_SUCCESS;

    switch (flash->index) {
    case SFUD_W25Q32BV_DEVICE_INDEX: {
        /* RCC Init */
        rcc_configuration(&spi2);
        /* GPIO Init */
        gpio_configuration(&spi2);
        /* SPI Init */
        spi_configuration(&spi2);
        /* Flash port */
        flash->spi.wr = spi_write_read;
        flash->spi.lock = spi_lock;
        flash->spi.unlock = spi_unlock;
        flash->spi.user_data = &spi2;
        /* about 100 microsecond delay */
        flash->retry.delay = retry_delay_100us;
        /* adout 10 seconds timeout */
        flash->retry.times = 10000;

        break;
    }
    }
	
    return result;
}

/**
 * This function is print debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 */
void sfud_log_debug(const char *file, const long line, const char *format, ...) {
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    printf("[SFUD](%s:%ld) ", file, line);
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    printf("%s\n", log_buf);
    va_end(args);
}

/**
 * This function is print routine info.
 *
 * @param format output format
 * @param ... args
 */
void sfud_log_info(const char *format, ...) {
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    printf("[SFUD]");
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    printf("%s\n", log_buf);
    va_end(args);
}
