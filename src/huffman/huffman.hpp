/**
 * Author: Matúš Škuta (xskuta04)
 * Date created: 07.05.2021
 * Name: huffman.hpp
 * Description: Contains definitions for both huffman coder and decoder structures and constant data
 * */
#ifndef __HUFFMAN__
#define __HUFFMAN__

#include <cstdint>    // uint8_t, uint16_t, uint64_t
#include <cstring>    // memcpy
#include <stdlib.h>   // malloc
#include <vector>     // vector
#include <iostream>   // cerr

// Maximum number of values
constexpr uint16_t N_VALUES = 256;
// Default allocation size
constexpr uint16_t ALLOC_SIZE = 512;
// Number of bits in byte
constexpr uint8_t BITS_IN_BYTE = 8;
// Bits that represent number of padding bits in data
constexpr uint8_t PADDING_BITS_MASK = 0x07;
// When given bit is set, do huffman decoding, otherwise only copy data to output buffer
constexpr uint8_t SETTINGS_BIT_CHECK = 0x08;

/**
 * Represent Node structure for holding huffman data
 * @param left Pointer to left node
 * @param right Pointer to right node
 * @param parent Pointer to parent node
 * @param index Index of node
 * @param weight Counter for how many times val exist
 * @param val Value
 * */
typedef struct Node {
  Node *left;
  Node *right;
  Node *parent;
  int32_t index;
  uint64_t weight;
  uint8_t val;
} Node;


#endif