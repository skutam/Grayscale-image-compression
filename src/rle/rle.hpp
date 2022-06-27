/**
 * Author: Matúš Škuta (xskuta04)
 * Date created: 07.05.2021
 * Name: rle.hpp
 * Description: Contains definitions of constant data for both RLE compressor and decompressor
 * */
#ifndef __RLE__
#define __RLE__

// Constants used both in Rle_Compressor and Rle_Decompressor

// Bit of scanning type VERTICAL/HORIZONTAL
constexpr uint8_t SCANNING_MASK = 0b10000000;

// Bit representing if -m was used
constexpr uint8_t MODEL_MASK = 0b01000000;

// 3 bits representing number of bytes following representing height
constexpr uint8_t HEIGHT_COUNT_MASK = 0b00000111;

// 3 bits representing number of bytes following representing width
constexpr uint8_t WIDTH_COUNT_MASK = 0b00111000;

// Mask to check first bit
constexpr uint8_t FIRST_BIT_MASK = 0x01;

// Size of Uint8_t in bits
constexpr size_t UINT8_T_SIZE = 8;

// Padding bits
constexpr uint8_t UINT8_T_PADDING = 0;

// Maximum value for 8 bits
constexpr uint8_t MAX_COUNTER_VAL = 0xFF;  // 255

#endif
