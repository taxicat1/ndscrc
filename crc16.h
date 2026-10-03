#ifndef CRC16_H
#define CRC16_H

#include <stddef.h>
#include <stdint.h>

uint16_t CRC16_Calc(uint8_t* data, size_t length);

#endif /* CRC16_H */
