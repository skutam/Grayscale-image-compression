/**
 * Author: Matúš Škuta (xskuta04)
 * Date created: 07.05.2021
 * Name: rle_decompressor.hpp
 * Description: Contains definitions of RLE decompressor class that is used to decompress
 * RLE encoded data into raw grayscale 8bit images
 * */
#ifndef __RLE_DECOMPRESSOR__
#define __RLE_DECOMPRESSOR__

#include <iostream> // cout, size_t
#include <cstdint>  // uint8_t
#include <cassert>  // assert
#include <cassert>  // assert
#include <cstring>  // memset

#include "rle.hpp"

/**
 * Class used for decommpressing RLE data compressed by class RleCompressor
 * */
class RleDecompressor {
private:
  // Buffer that holds loaded data
  const uint8_t *buffer;
  // Size of loaded data buffer
  size_t size;
  // Current index in loaded data buffer
  size_t index;

  // Buffer for holding decompressed data
  uint8_t *dec_buffer;
  // Allocation size of decompressed data buffer
  size_t dec_buffer_alloc;
  // Current index in decompressed data buffer
  size_t dec_buffer_index;

  /**
   * Decompress image horizontally
   * @returns True when image has been horrizontally decompressed, false otherwise
   * */
  bool DecompressHorizontally();
  
  /**
   * Decompress image vertically
   * @returns True when image has been vertically decompressed, false otherwise
   * */
  bool DecompressVertically(const size_t &width, const size_t &height);

  /**
   * Convert RLE compressed data into count and val
   * @param[out] count Number of times to replicate val value
   * @param[out] val Value to be replicated
   * @param[out] bit_index Represent actual index of bit while reading bit by bit
   * @param[out] byte Represent actual group byte, representing which bits are count bits and which are value bits
   * @returns True while there are still values, false otherwise
   * */
  bool GetValCount(
    size_t &count,
    uint8_t &val,
    uint8_t &bit_index,
    uint8_t &byte
  );

  /**
   * Read width and height from metadata
   * @param[out] width Width of image got from metadata
   * @param[out] height Height of image got from metadata
   * @returns True when we successfully got size from metadata, false otherwise
   * */
  bool GetSize(uint32_t &width, uint32_t &height);

public:
  /**
   * Constructor for RLE_Decompressor that will initialize values
   * @param[in] buffer Data buffer holding compressed RLE data
   * @param[in] size Size of data buffer
   * */
  RleDecompressor(uint8_t * &buffer, const size_t &size);

  /**
   * Deconstructor for RLE_Decompressor that will free allocated data
   * */
  ~RleDecompressor();

  /**
   * Decompress RLE data
   * @param[out] convert_from_model Set to true when first settings byte has -m bit set
   * @returns True when decompression was successfull, false otherwise
   * */
  bool Decompress(bool &convert_from_model);

  /**
   * Return pointer to decompressed data buffer
   * @returns Pointer to buffer
   * */
  uint8_t * & GetBuffer();

  /**
   * Return decompressed data buffer size
   * @returns Size of buffer
   * */
  size_t GetSize();
};

#endif
