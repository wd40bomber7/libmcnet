#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#define __USE_BSD
#include <endian.h>

#include "../include/mcnet/packets.h"
#include "../include/mcnet/structs.h"
#include "../include/mcnet/metadata.h"
#include "../include/mcnet/parser.h"

int16_t read_int16(uint8_t* in) {
  int16_t out;

  uint8_t* t = (uint8_t*)&out;

  t[0] = in[1];
  t[1] = in[0];

  return out;
}

int32_t read_int32(uint8_t* in) {
  int32_t out;

  uint8_t* t = (uint8_t*)&out;

  t[0] = in[3];
  t[1] = in[2];
  t[2] = in[1];
  t[3] = in[0];

  return out;
}

int64_t read_int64(uint8_t* in) {
  int64_t out;

  uint8_t* t = (uint8_t*)&out;

  t[0] = in[7];
  t[1] = in[6];
  t[2] = in[5];
  t[3] = in[4];
  t[4] = in[3];
  t[5] = in[2];
  t[6] = in[1];
  t[7] = in[0];

  return out;
}

float read_float(uint8_t* in) {
  float out;

  uint8_t* t = (uint8_t*)&out;

  t[0] = in[3];
  t[1] = in[2];
  t[2] = in[1];
  t[3] = in[0];

  return out;
}

double read_double(uint8_t* in) {
  double out;

  uint8_t* t = (uint8_t*)&out;

  t[0] = in[7];
  t[1] = in[6];
  t[2] = in[5];
  t[3] = in[4];
  t[4] = in[3];
  t[5] = in[2];
  t[6] = in[1];
  t[7] = in[0];

  return out;
}

#define PACKET(id, code) size_t mcnet_parser_parse_##id(mcnet_parser_t* parser, mcnet_parser_settings_t* settings, uint8_t* data, size_t data_len) { \
  mcnet_packet_##id##_t packet; \
  size_t nparsed = 0; \
  UBYTE(pid) \
  code \
  if (settings->on_packet != NULL) { \
    settings->on_packet(parser, (mcnet_packet_t*)&packet); \
  } \
  return nparsed; \
}

#define BOOL(name)         if (data_len < nparsed + 1)      { return 0; } packet.name = *((int8_t*)(data + nparsed)) ? 1 : 0; nparsed += 1;
#define BYTE(name)         if (data_len < nparsed + 1)      { return 0; } packet.name = *((int8_t*)(data + nparsed));         nparsed += 1;
#define UBYTE(name)        if (data_len < nparsed + 1)      { return 0; } packet.name = *((uint8_t*)(data + nparsed));        nparsed += 1;
#define SHORT(name)        if (data_len < nparsed + 2)      { return 0; } packet.name = read_int16(data + nparsed);           nparsed += 2;
#define INT(name)          if (data_len < nparsed + 4)      { return 0; } packet.name = read_int32(data + nparsed);           nparsed += 4;
#define LONG(name)         if (data_len < nparsed + 8)      { return 0; } packet.name = read_int64(data + nparsed);           nparsed += 8;
#define FLOAT(name)        if (data_len < nparsed + 4)      { return 0; } packet.name = read_float(data + nparsed);           nparsed += 4;
#define DOUBLE(name)       if (data_len < nparsed + 8)      { return 0; } packet.name = read_double(data + nparsed);          nparsed += 8;
#define BLOB(name, length) if (data_len < nparsed + length) { return 0; } packet.name = data + nparsed;                       nparsed += length;
#define STRING8(name) SHORT(name##_len) BLOB(name, packet.name##_len)
#define STRING16(name) SHORT(name##_len) BLOB(name, packet.name##_len * 2)
#define METADATA(name)

PACKETS

#undef BOOL
#undef BYTE
#undef UBYTE
#undef SHORT
#undef INT
#undef LONG
#undef FLOAT
#undef DOUBLE
#undef STRING8
#undef STRING16
#undef METADATA

#undef PACKET

#define PACKET(id, code) case 0x##id: { return mcnet_parser_parse_##id(parser, settings, data, data_len); }

size_t mcnet_parser_execute(mcnet_parser_t* parser, mcnet_parser_settings_t* settings, uint8_t* data, size_t data_len) {
  if (data_len < 1) {
    return 0;
  }

  switch (data[0]) {
    PACKETS

    default: {
      printf("Unknown packet: %02x\n", data[0]);

      if (settings->on_error != NULL) {
        settings->on_error(parser, -1);
      }

      return 0;
    }
  }

  return 0;
}

#undef PACKET
