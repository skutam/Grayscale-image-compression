/**
 * Author: Matúš Škuta (xskuta04)
 * Date created: 07.05.2021
 * Name: rle_compressor.hpp
 * Description: Contains definitions of RLE compressor class that is used to compress
 * raw grayscale 8bit images into RLE encoded data
 * */
#ifndef __RLE_COMPRESSOR__
#define __RLE_COMPRESSOR__

#include <cstdint>  // uint8_t
#include <cstring>  // memcpy
#include <string>
#include <iostream>
#include <vector>   // vector
#include <cassert>   // assert

#include "rle.hpp"

// Default data when no settings are pressent
constexpr uint8_t * NO_SETTINGS = nullptr;

/**
 * Class that will compress image data into RLE compressed data
 * */
class RleCompressor {
private:
  const uint8_t *buffer;
  uint8_t *encoded_buff;
  size_t encoded_index;
  size_t encoded_alloc;
  size_t alloc_size;

  /**
   * Create new buffer when there is none or reallocate existing buffer
   * */
  void ReallocateBuffer();

  /**
   * Append settings byte with width and height of image to buffer
   * @param[in] settings Settings byte to be added to buffer
   * @param[in] width Width of image to be added to buffer
   * @param[in] height Height of image to be added to buffer
   * */
  void appendSettingsToBuff(
    uint8_t &settings,
    uint32_t width,
    uint32_t height
  );

  /**
   * Horrizontally scan image data and convert them into RLE encrypted data
   * @param[in] width Width of image
   * @param[in] height Height of image
   * */
  void HorizontalScanning(
    const size_t &width,
    const size_t &height
  );

  /**
   * Vertically scan image data and convert them into RLE encrypted data
   * @param[in] width Width of image
   * @param[in] height Height of image
   * */
  void VerticalScanning(
    const size_t &width,
    const size_t &height
  );

  /**
   * Append data to group vector, and when we got 8 values in group vector push them into 
   * buffer with group byte
   * @param[out] group_vec Vector of byte values to be added to buffer
   * @param[out] group Group byte representing values and counters saved in group vector
   * @param[in] counter Adding counter value 
   * @param[in] end_push When true push all remaining values in buffer
   * @param[in] settings Settings byte that will be added to buffer
   * */
  void appendToBuff(
    std::vector<uint8_t> &group_vec,
    uint8_t &group,
    const uint8_t &val,
    bool counter,
    bool end_push,
    const uint8_t *settings
  );
  /**
   * Add counter value with value that 
   * COUNTER DOES NOT USE VALUES 0 AND 1, so we will use these values, so we can save into uint8_t values from 2 to 257, because 0 => 2, 1 => 3 ...255 => 257
   * @param[out] group_vec Vector of 8 values that will be appended to buffer after group byte
   * @param[out] group Group byte, 1 represents counter, 0 represents value
   * @param[in] val Value to be added to buffer
   * @param[in] counter Counter value that will be added before value
   * */
  void appendCounterValue(
    std::vector<uint8_t> &group_vec,
    uint8_t &group,
    const uint8_t &val,
    size_t &counter
  );

public:
  /**
   * Constructor that will initialize values
   * @param[in] buffer Buffer representing image data
   * @param[in] width Width of image in buffer
   * @param[in] height Height of image in buffer
   * */
  RleCompressor(const uint8_t *buffer, const uint32_t &width, const uint32_t &height);

  /**
   * Deconstructor that will free allocated data
   * */
  ~RleCompressor();

  /**
   * Start sequence scanning of image and convert it into RLE encoded data
   * @param[in] width Width of image
   * @param[in] height Height of image
   * @param[in] input_preprocessing True when image was preprocessed, false otherwise
   * */
  void SequenceScanning(
    const size_t &width,
    const size_t &height,
    const bool &input_preprocessing
  );

  /**
   * Start adaptive scanning where we choose the best scanning type that reduces the data the most
   * @param[in] width Width of image
   * @param[in] height Height of image
   * @param[in] input_preprocessing True when image was preprocessed, false otherwise
   * */
  void AdaptiveScanning(
    const size_t &width,
    const size_t &height,
    const bool &input_preprocessing
  );

  /**
   * Return pointer to decompressed data buffer
   * @returns Pointer to buffer
   * */
  uint8_t * & GetBuffer();

  /**
   * Return decompressed data buffer size
   * @returns Size of buffer
   * */
  size_t & GetSize();
};

#endif
