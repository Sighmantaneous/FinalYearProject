#include "packet.h"
#include <string.h>

int packet_serialise(const packet_t *pkt, uint8_t *buff, size_t cap) {

  if (pkt == NULL || buff == NULL) {

    return -1;
  }

  size_t total = (size_t)PACKET_HEADER_LEN + pkt->len;

  if (total > cap) {
    return -1;
  }

  buff[0] = pkt->dest;
  buff[1] = pkt->source;
  buff[2] = (uint8_t)(pkt->msg_id & 0xFF);
  buff[3] = (uint8_t)((pkt->msg_id >> 8) & 0xFF);
  buff[4] = pkt->type;
  buff[5] = pkt->ttl;
  buff[6] = pkt->hop_start;
  buff[7] = pkt->len;

  memcpy(&buff[PACKET_HEADER_LEN], pkt->payload, pkt->len);

  return (int)total;
}

int packet_deserialise(const uint8_t *buff, size_t n, packet_t *out) {

  if (buff == NULL || out == NULL) {

    return -1;
  }

  if (n < PACKET_HEADER_LEN) {
    return -1;
  }
  uint8_t len = buff[7];

  if (n != (size_t)PACKET_HEADER_LEN + len) {
    return -1;
  }

  out->dest = buff[0];
  out->source = buff[1];
  out->msg_id = (uint16_t)(buff[2] | (buff[3] << 8)); // endian assemble
  out->type = buff[4];
  out->ttl = buff[5];
  out->hop_start = buff[6];
  out->len = len;

  memcpy(out->payload, &buff[PACKET_HEADER_LEN], len);

  return 0;
}
