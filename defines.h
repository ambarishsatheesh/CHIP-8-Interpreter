#ifndef MY_DEFINES_H
#define MY_DEFINES_H

#include <chrono>

#define CLOCKCOUNT std::chrono::system_clock::now().time_since_epoch().count()


//opcode AND 111111111111 (0x0FFF) gives you the last 3 nibbles (i.e 12-bit address)
//e.g. 0x3dfe & 0x0FFF = 0xdfe
#define NNN(opcode) (opcode & 0x0FFFu)

//opcode AND 11111111 (0x00FF) gives you the last 2 nibbles (8-bit literal)
//e.g. 0x3dfe & 0x00FF = 0xfe
#define KK(opcode) (opcode & 0x00FFu)

//opcode AND 11111111 (0x0FF) gives you the first nibble followed by 00
//e.g. 0x3dfe & 0x0F00 = 0xd00
//then shifting right 8 bits gives first nibble
//e.g. 0xd00 >> 8 = 0xd
#define X(opcode) ((opcode & 0x0F00u) >> 8u)

//opcode AND 11110000 (0x00F0u) gives you the middle nibble followed by 00
//e.g. 0x3dfe & 0x0F00 = 0xf0
//then shifting right 8 bits gives middle nibble
//e.g. 0xf0 >> 4 = 0xf
#define Y(opcode) ((opcode & 0x00F0u) >> 4u)

//opcode AND 1111000000000000 (0xF000u) gives you the first nibble followed by 000
//e.g. 0x3dfe & 0xF000 = 0x3000
//then shifting right 12 bits gives first nibble
//e.g. 0x3000 >> 4 = f
#define I(opcode) ((opcode & 0xF000u) >> 12u)

#endif // MY_DEFINES_H
