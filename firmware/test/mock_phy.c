#include "mock_phy.h"
#include "packet.h"
#include <stddef.h>
#include <string.h>

#define MOCK_QUEUE_LEN 16

typedef struct {
  uint8_t bytes[PACKET_MAX];
  size_t len;
} mock_frame_t;

static mock_frame_t queue[MOCK_QUEUE_LEN];
static size_t head = 0;
static size_t tail = 0;
static size_t count = 0;

void mock_phy_resetq(void) { head = tail = count = 0; }

int phy_send(const uint8_t *buff, size_t len) {

  if (buff == NULL || len > PACKET_MAX) {
    return -1;
  }

  if (count == MOCK_QUEUE_LEN) {
    return -1;
  }

  memcpy(queue[tail].bytes, buff, len);
  queue[tail].len = len;
  tail = (tail + 1) % MOCK_QUEUE_LEN;
  count++;
  return 0;
}

int phy_rec(uint8_t *buff, size_t cap) {

  if (buff == NULL) {
    return -1;
  }

  if (count == 0) {
    return 0;
  }

  size_t len = queue[head].len;
  if (len > cap) {
    return -1;
  }

  memcpy(buff, queue[head].bytes, len);
  head = (head + 1) % MOCK_QUEUE_LEN;
  count--;
  return (int)len;
}
