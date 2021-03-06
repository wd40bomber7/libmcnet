#include <stdio.h>
#include <stddef.h>

#include <mcnet.h>

#define PACKET(id, code) case 0x##id: { \
  mcnet_packet_##id##_t* tmp = (mcnet_packet_##id##_t*)packet; \
  printf("Packet ID: 0x%02x\n", tmp->pid); \
  code \
  printf("Packet size: %llu\n", (unsigned long long int)mcnet_generator_size(parser->data, packet)); \
  break; \
};

#define ONLY_SERVER(code) if (1) { code }
#define ONLY_CLIENT(code) if (1) { code }
#define PARSER_CODE(code)
#define GENERATOR_CODE(code)
#define BOOL(name)          printf("  [bool]     %c\n",   tmp->name ? 'T' : 'F');
#define BYTE(name)          printf("  [byte]     %d\n",   tmp->name);
#define UBYTE(name)         printf("  [ubyte]    %u\n",   tmp->name);
#define SHORT(name)         printf("  [short]    %d\n",   tmp->name);
#define USHORT(name)        printf("  [ushort]   %u\n",   tmp->name);
#define INT(name)           printf("  [int]      %d\n",   tmp->name);
#define LONG(name)          printf("  [long]     %lld\n", tmp->name);
#define FLOAT(name)         printf("  [float]    %f\n",   tmp->name);
#define DOUBLE(name)        printf("  [double]   %g\n",   tmp->name);
#define STRING16(name)      printf("  [string16] %d -> ", tmp->name##_len); int name##_i; for (name##_i = 0; name##_i < tmp->name##_len; ++name##_i) { printf("%c", tmp->name[name##_i*2+1]); } printf("\n");
#define STRING8(name)       printf("  [string8]  %d -> ", tmp->name##_len); int name##_i; for (name##_i = 0; name##_i < tmp->name##_len; ++name##_i) { printf("%c", tmp->name[name##_i]); } printf("\n");
#define BLOB(name, length)  printf("  [blob]     %d -> ", tmp->length); int name##_i; for (name##_i = 0; name##_i < tmp->length; ++name##_i) { printf("%02x", tmp->name[name##_i]); if (name##_i != tmp->length) { printf(":"); } } printf("\n");
#define METADATA(name)      printf("  [metadata] %d\n",   tmp->name##_len);
#define SLOT(name)          printf("  [slot]     %d\n",   tmp->name##_len);
#define SLOTS(name, length) printf("  [slots]    %d\n",   tmp->name##_len);

void on_packet(mcnet_parser_t* parser, mcnet_packet_t* packet) {
  // This gets around the unused variable warning. Most of the time, you'd keep
  // something inside the "data" field of this object, allowing you to tie the
  // packets back to a client or stream.
  (void)parser;

  switch(packet->pid) {
    PACKETS
    default: { printf("Unknown packet type: %d\n", packet->pid); break; };
  }
}

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
#undef STRING16
#undef METADATA
#undef SLOT
#undef SLOTS

#undef PACKET

void on_error(mcnet_parser_t* parser, int err) {
  printf("[%p] error %d\n", parser->data, err);
}

int main() {
  mcnet_generator_t generator = { .type = MCNET_TYPE_SERVER };
  mcnet_parser_t parser = { .data = &generator, .type = MCNET_TYPE_SERVER };
  mcnet_parser_settings_t settings = { .on_packet = on_packet, .on_error = on_error };

  uint8_t data[] = {
    0x00,
    0x00, 0x00, 0x00, 0x01,

    0x00,
    0x00, 0x00, 0x00, 0x02,

    0x01,
    0x00, 0x00, 0x00, 0x01,
    0x00, 0x04, 0x00, 0x61, 0x00, 0x73, 0x00, 0x64, 0x00, 0x66,
    0x02,
    0x03,
    0x04,
    0x05,
    0x06,

    0x17,
    0x00, 0x00, 0x00, 0x01,
    0x02,
    0x00, 0x00, 0x00, 0x03,
    0x00, 0x00, 0x00, 0x04,
    0x00, 0x00, 0x00, 0x05,
    0x00, 0x00, 0x00, 0x06,
    0x00, 0x07,
    0x00, 0x08,
    0x00, 0x09,

    0x17,
    0x00, 0x00, 0x00, 0x01,
    0x02,
    0x00, 0x00, 0x00, 0x03,
    0x00, 0x00, 0x00, 0x04,
    0x00, 0x00, 0x00, 0x05,
    0x00, 0x00, 0x00, 0x00,

    0x00,
    0x00, 0x00, 0x00, 0x03
  };

  printf("going to try to parse %llu bytes...\n\n", (unsigned long long int)sizeof(data));

  size_t nparsed = 0, offset = 0;

  while ((nparsed = mcnet_parser_execute(&parser, &settings, data + offset, sizeof(data) - offset)) != 0) {
    if (nparsed == MCNET_EAGAIN) {
      printf("need more data\n");
      break;
    }

    if (nparsed == MCNET_EINVALID) {
      printf("invalid data encountered\n");
      break;
    }

    offset += nparsed;

    printf("parsed %llu bytes and %llu total\n", (unsigned long long int)nparsed, (unsigned long long int)offset);
  }

  printf("\nparsed %llu bytes total\n", (unsigned long long int)offset);

  return 0;
}
