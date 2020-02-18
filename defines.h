#ifndef MY_DEFINES_H
#define MY_DEFINES_H


//opcode bitwise AND 111111111111 (0x0FFF) gives you the last 3 nibbles (i.e 12-bit address)
//e.g. 0x3dfe & 0x0FFF = dfe
#define NNN(opcode) (opcode & 0x0FFFu)

//opcode bitwise AND 11111111 (0x00FF) gives you the last 2 nibbles (8-bit literal)
//e.g. 0x3dfe & 0x00FF = fe
#define KK(opcode) (opcode & 0x00FFu)

//opcode bitwise AND 11111111 (0x0FF) gives you the first nibble followed by 00
//e.g. 0x3dfe & 0x0F00 = d00
//then shifting right 8 bits gives first nibble
//e.g. d00 >> 8 = d
#define X(opcode) ((opcode & 0x0F00u) >> 8u)

//opcode bitwise AND 11110000 (0x00F0u) gives you the middle nibble followed by 00
//e.g. 0x3dfe & 0x0F00 = f0
//then shifting right 8 bits gives middle nibble
//e.g. f0 >> 4 = f
#define Y(opcode) ((opcode & 0x00F0u) >> 4u)

#endif // MY_DEFINES_H