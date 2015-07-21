#ifndef COSTSNB_CLISRV_STRUCTS_H
#define COSTSNB_CLISRV_STRUCTS_H


// This file was autogenerated
#include <stdint.h>
#include <cstring>

#pragma pack(push,1)
struct App_message {
  uint16_t opcode;
};

struct Handshake_srv : App_message{
  uint16_t vsn;
};

struct Handshake_cli_ack : App_message{
};

struct Str_data : App_message{
  uint32_t seq_num;
  uint32_t data_size;
  uint8_t last;
  uint8_t data[1024];
};

#pragma pack(pop)
#endif

