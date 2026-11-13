#include "radio_con_test.h"
#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"
#include "hal/spi_types.h"
#include "soc/clk_tree_defs.h"
#include <stddef.h>
#include <stdint.h>

#define SCK_PIN 18   // Serial clock
#define MOSI_PIN 23  // Serial Data In
#define MISO_PIN 19  // Serial Data Out
#define NSS_PIN 5    // nSEL
#define RESET_PIN 14 // RESET
#define BUSY_PIN 26  // BUSY
#define RX_PIN 32    // RX
#define TX_PIN 33    // TX

#define TIMEOUT_BUSY_MS 100
#define SPI_CLOCK_HZ 1000000 // 1 Mhz

#define CMD_GET_STATUS 0xC0
#define CMD_READ_REG 0x1D

static spi_device_handle_t radio;

static int wait_busy_low(uint32_t timout_ms) {
  int64_t deadline = esp_timer_get_time() + (int64_t)timout_ms * 1000;
  while (gpio_get_level(BUSY_PIN)) {
    if (esp_timer_get_time() > deadline) {
      return -1;
    }
    esp_rom_delay_us(10);
  }
  return 0;
}

static int radio_trans(const uint8_t *tx, uint8_t *rx, size_t len) {
  if (wait_busy_low(TIMEOUT_BUSY_MS) != 0) {
    return -1;
  };

  spi_transaction_t t = {
      .flags = 0,
      .cmd = 0,
      .addr = 0,
      .length = len * 8,
      .rxlength = 0,
      .override_freq_hz = 0,
      .user = 0,
      .tx_buffer = tx,
      .rx_buffer = rx,
  };
  return spi_device_polling_transmit(radio, &t) == ESP_OK ? 0 : -1;
}

static void setup_gpio(void) {

  gpio_config_t out_cfg = {

      .pin_bit_mask = (1ULL << RESET_PIN) | (1ULL << RX_PIN) | (1ULL << TX_PIN),
      .mode = GPIO_MODE_OUTPUT,
      .pull_down_en = 0,
      .pull_up_en = 0,
      .intr_type = 0,
  };

  gpio_config(&out_cfg);
  gpio_set_level(RESET_PIN, 1);
  gpio_set_level(RX_PIN, 0);
  gpio_set_level(TX_PIN, 0);

  gpio_config_t in_cfg = {
      .pin_bit_mask = (1ULL << BUSY_PIN),
      .mode = GPIO_MODE_INPUT,
  };

  gpio_config(&in_cfg);
}

static void spi_setup(void) {
  spi_bus_config_t bus_cfg = {.mosi_io_num = MOSI_PIN,
                              .miso_io_num = MISO_PIN,
                              .sclk_io_num = SCK_PIN,
                              .quadwp_io_num = -1,
                              .quadhd_io_num = -1,
                              .data4_io_num = -1,
                              .data5_io_num = -1,
                              .data6_io_num = -1,
                              .data7_io_num = -1,
                              .data_io_default_level = false,
                              .max_transfer_sz = 16,
                              .flags = 0,
                              .isr_cpu_id = ESP_INTR_CPU_AFFINITY_0};
  spi_bus_initialize(SPI3_HOST, &bus_cfg, SPI_DMA_CH_AUTO);

  spi_device_interface_config_t dev_cfg = {
      .command_bits = 0,
      .address_bits = 0,
      .dummy_bits = 0,
      .mode = 0,
      .clock_source = SPI_CLK_SRC_DEFAULT,
      .duty_cycle_pos = 0,
      .cs_ena_pretrans = 0,
      .cs_ena_posttrans = 0,
      .clock_speed_hz = SPI_CLOCK_HZ,
      .input_delay_ns = 0,
      .sample_point = SPI_SAMPLING_POINT_PHASE_0,
      .spics_io_num = NSS_PIN,
      .flags = 0,
      .queue_size = 1,
      .pre_cb = NULL,
      .post_cb = NULL,
  };
  spi_bus_add_device(SPI3_HOST, &dev_cfg, &radio);
}
