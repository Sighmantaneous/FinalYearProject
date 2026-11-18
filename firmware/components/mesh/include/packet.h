#ifndef PACKET_H
#define PACKET_H

#include "phy.h"
#include <stddef.h>
#include <stdint.h>

#define PACKET_HEADER_LEN 8
#define PACKET_MAX_PAYLOAD (PHY_MAX_FRAME - PACKET_HEADER_LEN)
#define PACKET_MAX PHY_MAX_FRAME

typedef struct {
  uint8_t dest;
  uint8_t source;
  uint16_t msg_id;
  uint8_t type;
  uint8_t ttl;
  uint8_t hop_start;
  uint8_t len;
  uint8_t payload[PACKET_MAX_PAYLOAD];
} packet_t;

int packet_serialise(const packet_t *pkt, uint8_t *buff, size_t cap);
int packet_deserialise(const uint8_t *buff, size_t n, packet_t *out);

#endif
