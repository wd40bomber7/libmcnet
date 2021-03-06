#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "../include/mcnet/error.h"
#include "../include/mcnet/packets.h"
#include "../include/mcnet/structs.h"
#include "../include/mcnet/metadata.h"
#include "../include/mcnet/slot.h"
#include "../include/mcnet/read.h"
#include "../include/mcnet/parser.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PARSER_CODE(code) code
#define GENERATOR_CODE(code)
#define BOOL(name)         if (data_len < nparsed + 1)             { return MCNET_EAGAIN; } packet.name = mcnet_read_bool(data + nparsed);   nparsed += 1;
#define BYTE(name)         if (data_len < nparsed + 1)             { return MCNET_EAGAIN; } packet.name = mcnet_read_int8(data + nparsed);   nparsed += 1;
#define UBYTE(name)        if (data_len < nparsed + 1)             { return MCNET_EAGAIN; } packet.name = mcnet_read_uint8(data + nparsed);  nparsed += 1;
#define SHORT(name)        if (data_len < nparsed + 2)             { return MCNET_EAGAIN; } packet.name = mcnet_read_int16(data + nparsed);  nparsed += 2;
#define USHORT(name)       if (data_len < nparsed + 2)             { return MCNET_EAGAIN; } packet.name = mcnet_read_uint16(data + nparsed); nparsed += 2;
#define INT(name)          if (data_len < nparsed + 4)             { return MCNET_EAGAIN; } packet.name = mcnet_read_int32(data + nparsed);  nparsed += 4;
#define LONG(name)         if (data_len < nparsed + 8)             { return MCNET_EAGAIN; } packet.name = mcnet_read_int64(data + nparsed);  nparsed += 8;
#define FLOAT(name)        if (data_len < nparsed + 4)             { return MCNET_EAGAIN; } packet.name = mcnet_read_float(data + nparsed);  nparsed += 4;
#define DOUBLE(name)       if (data_len < nparsed + 8)             { return MCNET_EAGAIN; } packet.name = mcnet_read_double(data + nparsed); nparsed += 8;
#define BLOB(name, length) if (data_len < nparsed + packet.length) { return MCNET_EAGAIN; } packet.name = data + nparsed;                    nparsed += packet.length;
#define STRING8(name) SHORT(name##_len) BLOB(name, name##_len)
#define STRING16(name) SHORT(name##_len) BLOB(name, name##_len * 2)
#define METADATA(name) \
  size_t name = mcnet_metadata_parser_parse(NULL, data + nparsed, data_len - nparsed); \
  if ((name == MCNET_EAGAIN) || (name == MCNET_EINVALID)) { return name; } \
  packet.name##_len = name; \
  packet.name = data + nparsed; \
  nparsed += name;
#define SLOT(name) \
  size_t name = mcnet_slot_parser_parse(NULL, data + nparsed, data_len - nparsed); \
  if ((name == MCNET_EAGAIN) || (name == MCNET_EINVALID)) { return name; } \
  packet.name##_len = name; \
  packet.name = data + nparsed; \
  nparsed += name;
#define SLOTS(name, len) \
  size_t name = 0; \
  int i = 0; \
  for (i = 0; i < packet.len; ++i) { \
    size_t tmp = mcnet_slot_parser_parse(NULL, data + nparsed + name, data_len - nparsed - name); \
    if ((tmp == MCNET_EAGAIN) || (tmp == MCNET_EINVALID)) { return tmp; } \
    name += tmp; \
  } \
  packet.name##_len = name; \
  packet.name = data + nparsed; \
  nparsed += name;

#define ONLY_SERVER(code) if ((parser != NULL) && (parser->type == MCNET_TYPE_SERVER)) { code }
#define ONLY_CLIENT(code) if ((parser != NULL) && (parser->type == MCNET_TYPE_CLIENT)) { code }

#define PACKET(id, code) case 0x##id: { \
  mcnet_packet_##id##_t packet; \
  memset(&packet, 0, sizeof(mcnet_packet_##id##_t)); \
  size_t nparsed = 0; \
  UBYTE(pid) \
  code \
  if (settings->on_packet != NULL) { \
    settings->on_packet(parser, (mcnet_packet_t*)&packet); \
  } \
  return nparsed; \
}

size_t mcnet_parser_execute(mcnet_parser_t* parser, mcnet_parser_settings_t* settings, uint8_t* data, size_t data_len) {
  if (data_len < 1) {
    if (settings->on_error != NULL) {
      settings->on_error(parser, MCNET_EAGAIN);
    }

    return MCNET_EAGAIN;
  }

  switch (data[0]) {
    PACKETS

    default: {
      if (settings->on_error != NULL) {
        settings->on_error(parser, MCNET_EINVALID);
      }

      return MCNET_EINVALID;
    }
  }

  return MCNET_EAGAIN;
}

#undef PACKET

#undef ONLY_SERVER
#undef ONLY_CLIENT

#undef PARSER_CODE
#undef GENERATOR_CODE
#undef BOOL
#undef BYTE
#undef UBYTE
#undef SHORT
#undef USHORT
#undef INT
#undef LONG
#undef FLOAT
#undef DOUBLE
#undef STRING8
#undef STRING16
#undef METADATA
#undef SLOT
#undef SLOTS

#ifdef __cplusplus
}
#endif
