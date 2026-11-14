#include "radio_con_test.h"
#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "esp_err.h"
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
#define CMD_WRITE_REG 0x0D
#define CMD_SET_STANDBY 0x80
#define STDBY_RC 0x00
#define STDBY_XOSC 0x01

static spi_device_handle_t radio;

static esp_err_t wait_busy_low(uint32_t timeout_ms) {
  int64_t deadline = esp_timer_get_time() + (int64_t)timeout_ms * 1000;
  while (gpio_get_level(BUSY_PIN)) {
    if (esp_timer_get_time() > deadline) {
      return ESP_ERR_TIMEOUT;
    }
    esp_rom_delay_us(10);
  }
  return ESP_OK;
}

static esp_err_t radio_trans(const uint8_t *tx, uint8_t *rx, size_t len) {
  esp_err_t err = wait_busy_low(TIMEOUT_BUSY_MS);
  if (err != ESP_OK) {
    return err;
  }
  spi_transaction_t t = {
      .flags = 0,
      .cmd = 0,
      .addr = 0,
      .length = len * 8,
      .rxlength = 0,
      .override_freq_hz = 0,
      .user = NULL,
      .tx_buffer = tx,
      .rx_buffer = rx,
  };
  return spi_device_polling_transmit(radio, &t);
}

static esp_err_t gpio_setup(void) {

  gpio_config_t out_cfg = {

      .pin_bit_mask = (1ULL << RESET_PIN) | (1ULL << RX_PIN) | (1ULL << TX_PIN),
      .mode = GPIO_MODE_OUTPUT,
      .pull_down_en = 0,
      .pull_up_en = 0,
      .intr_type = 0,
  };
  esp_err_t err = gpio_config(&out_cfg);
  if (err != ESP_OK) {
    return err;
  }

  err = gpio_set_level(RESET_PIN, 1);
  if (err != ESP_OK) {
    return err;
  }
  err = gpio_set_level(RX_PIN, 0);
  if (err != ESP_OK) {
    return err;
  }
  err = gpio_set_level(TX_PIN, 0);
  if (err != ESP_OK) {
    return err;
  }

  gpio_config_t in_cfg = {
      .pin_bit_mask = (1ULL << BUSY_PIN),
      .mode = GPIO_MODE_INPUT,
  };

  return gpio_config(&in_cfg);
}

static esp_err_t spi_setup(void) {
  spi_bus_config_t bus_cfg = {
      .mosi_io_num = MOSI_PIN,
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
      .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
  };
  esp_err_t err = spi_bus_initialize(SPI3_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
  if (err != ESP_OK) {
    return err;
  }

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
  return spi_bus_add_device(SPI3_HOST, &dev_cfg, &radio);
}

static esp_err_t radio_reset(void) {
  gpio_set_level(RESET_PIN, 0);
  esp_rom_delay_us(200);
  gpio_set_level(RESET_PIN, 1);
  return wait_busy_low(TIMEOUT_BUSY_MS);
}

static esp_err_t get_status(uint8_t *status) {
  uint8_t tx[2] = {CMD_GET_STATUS, 0x00};
  uint8_t rx[2] = {0};

  esp_err_t err = radio_trans(tx, rx, 2);
  if (err != ESP_OK) {
    return err;
  }
  *status = rx[1];
  return ESP_OK;
}

static esp_err_t read_reg(uint16_t addr, uint8_t *val) {
  uint8_t tx[5] = {CMD_READ_REG, addr >> 8, addr & 0xFF, 0x00, 0x00};
  uint8_t rx[5] = {0};
  esp_err_t err = radio_trans(tx, rx, 5);
  if (err != ESP_OK) {
    return err;
  }
  *val = rx[4];
  return ESP_OK;
}

static esp_err_t write_reg(uint16_t addr, const uint8_t *data, size_t n) {
  if (n > 13) {
    return ESP_ERR_INVALID_SIZE;
  }

  uint8_t tx[16] = {CMD_WRITE_REG, addr >> 8, addr & 0xFF};
  uint8_t rx[16] = {0};
  for (size_t i = 0; i < n; i++) {
    tx[3 + i] = data[i];
  }
  return radio_trans(tx, rx, 3 + n);
}

static esp_err_t set_standby(uint8_t cfg) {
  uint8_t tx[2] = {CMD_SET_STANDBY, cfg};
  uint8_t rx[2] = {0};
  return radio_trans(tx, rx, 2);
}

static void test_sync_write(void) {
  const uint8_t pub[2] = {0x34, 0x44};
  const uint8_t priv[2] = {0x14, 0x24};
  uint8_t msb, lsb;
  esp_err_t err;

  err = write_reg(0x0740, pub, 2);
  if (err != ESP_OK) {
    ESP_LOGE("radio", "Write pub %s", esp_err_to_name(err));
    return;
  }

  err = read_reg(0x740, &msb);
  if (err == ESP_OK) {
    err = read_reg(0x0741, &lsb);
  }
  if (err != ESP_OK) {
    ESP_LOGE("radio", "readback: %s", esp_err_to_name(err));
    return;
  }
  ESP_LOGI("radio", "wrote 0x3444, read 0x%02X%02X (%s)", msb, lsb,
           (msb == 0x34 && lsb == 0x44) ? "PASS" : "FAIL");

  err = write_reg(0x0740, priv, 2); // restore default
  if (err != ESP_OK) {
    ESP_LOGE("radio", "write priv: %s", esp_err_to_name(err));
    return;
  }
}

static void test_standby(void) {
  uint8_t st;
  esp_err_t err = set_standby(STDBY_XOSC);
  if (err == ESP_OK) {
    err = get_status(&st);
  }
  if (err != ESP_OK) {
    ESP_LOGE("radio", "xosc: %s", esp_err_to_name(err));
    return;
  }
  ESP_LOGI("radio", "after STDBY_XOSC mode=%d (expect 3)", (st >> 4) & 7);

  err = set_standby(STDBY_RC);
  if (err == ESP_OK) {
    err = get_status(&st);
  }
  if (err != ESP_OK) {
    ESP_LOGE("radio", "rc: %s", esp_err_to_name(err));
    return;
  }
  ESP_LOGI("radio", "after STDBY_RC mode=%d (expect 2)", (st >> 4) & 7);
}
void radio_con_test(void) {

  gpio_setup();
  esp_err_t err = spi_setup();
  if (err != ESP_OK) {
    ESP_LOGE("radio", "spi_setup: %s", esp_err_to_name(err));
    return;
  }
  err = radio_reset();
  if (err != ESP_OK) {
    ESP_LOGE("radio", "reset: %s", esp_err_to_name(err));
    return;
  }
  uint8_t st;
  err = get_status(&st);
  if (err == ESP_OK) {
    ESP_LOGI("radio", "status=0x%02X mode=%d (expect 2)", st, (st >> 4) & 7);
  } else {
    ESP_LOGE("radio", "get_status: %s", esp_err_to_name(err));
  }

  uint8_t msb, lsb;
  err = read_reg(0x0740, &msb);
  if (err != ESP_OK) {
    ESP_LOGE("radio", "read 0x0740: %s", esp_err_to_name(err));
    return;
  }
  err = read_reg(0x0741, &lsb);
  if (err != ESP_OK) {
    ESP_LOGE("radio", "read 0x0741: %s", esp_err_to_name(err));
    return;
  }

  ESP_LOGI("radio", "sync_word=0x%02X%02X (expect 0x1424)", msb, lsb);
  test_sync_write();
  test_standby();
}
