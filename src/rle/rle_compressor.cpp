/**
 * Author: Matúš Škuta (xskuta04)
 * Date created: 07.05.2021
 * Name: rle_compressor.cpp
 * Description: Contains implementations of RLE compressor class that is used to compress
 * image data into RLE encoded data
 * */
#include "rle_compressor.hpp"

/**
 * Constructor that will initialize values
 * @param[in] buffer Buffer representing image data
 * @param[in] width Width of image in buffer
 * @param[in] height Height of image in buffer
 * */
RleCompressor::RleCompressor(
  const uint8_t *buffer,
  const uint32_t &width,
  const uint32_t &height
) {
  // Set buffer which we will be converting to RLE
  this->buffer = buffer;

  // We will be using third method of RLE, which in worst case increase output by 12.5%
  this->alloc_size = static_cast<size_t>((width * height) + (width * height) / 8 + 1);
  // Set encoded buffer data
  this->encoded_buff = nullptr;
  this->encoded_alloc = 0;
  this->encoded_index = 0;
}

/**
 * Deconstructor that will free allocated data
 * */
RleCompressor::~RleCompressor() {
  // When buffer was allocated, free him
  if (this->encoded_buff) {
    free(this->encoded_buff);
  }

  // Remove pointer pointing to outside buffer
  this->buffer = nullptr;
}

/**
 * @param[out] byte Byte with set bit on given index
 * @param[in] index Index of bit to be set
 * */
void set_bit(uint8_t &byte, const size_t &index) {
  byte |= (1UL << index);
}

/**
 * Create new buffer when there is none or reallocate existing buffer
 * */
void RleCompressor::ReallocateBuffer() {
  // Increase buffer by 1/4 of initial buffer length
  this->encoded_alloc = (this->encoded_alloc == 0) ? this->alloc_size : this->encoded_alloc + (static_cast<size_t>(this->alloc_size * 0.25 + 1));
  
  // Allocate new buffer
  void * tmp = (uint8_t *)malloc(sizeof(uint8_t) * this->encoded_alloc);

  // Invalid pointer
  assert(tmp != nullptr);

  // When buffer exist, copy data and free buffer
  if (this->encoded_buff != nullptr) {
    memcpy(tmp, this->encoded_buff, sizeof(uint8_t) * this->encoded_index);
    free(this->encoded_buff);
  }

  // Set new buffer
  this->encoded_buff = (uint8_t *)tmp;
  
  // Null pointer to newly created buffer
  tmp = nullptr;
}

/**
 * Append settings byte with width and height of image to buffer
 * @param[in] settings Settings byte to be added to buffer
 * @param[in] width Width of image to be added to buffer
 * @param[in] height Height of image to be added to buffer
 * */
void RleCompressor::appendSettingsToBuff(
  uint8_t &settings,
  uint32_t width,
  uint32_t height
) {
  // Counter for height and width bytes
  uint8_t count_h = 0;
  uint8_t count_w = 0;

  // Value of 8 bits into which we will copy first 8 bits of width and settings 
  uint8_t val = 0;

  // Vector of width and height values
  std::vector<uint8_t> vec_w;
  std::vector<uint8_t> vec_h;

  // Convert width into 8bit values
  while (width > MAX_COUNTER_VAL) {
    // Copy first 8 bits of width
    val = (width & MAX_COUNTER_VAL);
    // Add value to vector
    vec_w.push_back(val);
    // Shift width to the right by 8 bits
    width = (width >> 8);
    // Increment counter
    count_w++;
  }

  // Add last 8 bits to the vector
  vec_w.push_back((width & MAX_COUNTER_VAL));

  // Convert height into 8bit values
  while (height > MAX_COUNTER_VAL) {
    // Copy first 8 bits of height
    val = (height & MAX_COUNTER_VAL);
    // Add value to the vector
    vec_h.push_back(val);
    // Shift height to the right by 8 bits
    height = (height >> 8);
    // Increment counter
    count_h++;
  }

  // Add last 8 bits to the vector
  vec_h.push_back((height & MAX_COUNTER_VAL));

  // Set width and height
  // FIRST BIT  => HORIZONTAL | VERTICAL
  // SECOND BIT => MODEL      | NO MODEL
  // 3 - 5      => COUNT OF WIDTH BYTES
  // 6 - 7      => COUNT OF HEIGHT BYTES
  settings |= (count_w << 3);
  settings |= (count_h);

  // Increase buffer, when we need more value for our metadata
  if (this->encoded_alloc < ((size_t)(1 + (count_w + 1 + count_h + 1) + 1))) {
    this->ReallocateBuffer();
  }

  // Push settings first
  this->encoded_buff[this->encoded_index++] = settings;

  // When vec is empty, return error
  if (vec_w.size() == 0) {
    std::cerr << "WIDTH VECTOR IS EMPTY" << std::endl;
    return;
  }

  // Push from back to front
  for (int8_t i = (vec_w.size() - 1); i >= 0; i--) {
    this->encoded_buff[this->encoded_index++] = vec_w[i];
  }

    // When vec is empty, return error
  if (vec_h.size() == 0) {
    std::cerr << "HEIGHT VECTOR IS EMPTY" << std::endl;
    return;
  }

  // Push from back to front
  for (int8_t i = (vec_h.size() - 1); i >= 0; i--) {
    this->encoded_buff[this->encoded_index++] = vec_h[i];
  }  
}

/**
 * Append data to group vector, and when we got 8 values in group vector push them into 
 * buffer with group byte
 * @param[out] group_vec Vector of byte values to be added to buffer
 * @param[out] group Group byte representing values and counters saved in group vector
 * @param[in] counter Adding counter value 
 * @param[in] end_push When true push all remaining values in buffer
 * @param[in] settings Settings byte that will be added to buffer
 * */
void RleCompressor::appendToBuff(
  std::vector<uint8_t> &group_vec,
  uint8_t &group,
  const uint8_t &val,
  bool counter,
  bool end_push,
  const uint8_t *settings
) {
  // When we do not have enough space for GROUP + 8 values, increase buff
  if (this->encoded_alloc <= (this->encoded_index + (UINT8_T_SIZE + 1))) {
    this->ReallocateBuffer();
  }

  // When adding settings, add only them
  if (settings != nullptr) {
    this->encoded_buff[this->encoded_index++] = (*settings);
    return;
  }

  // Set 1 when adding counter value
  if (counter) {
    set_bit(group, group_vec.size());
  }

  // Add value, when we are not pushing all values at last
  if (!end_push) {
    group_vec.push_back(val);
  }

  // When group vector has 8 values, add them after group value
  if (group_vec.size() == UINT8_T_SIZE || end_push) {
    // Add group value
    this->encoded_buff[this->encoded_index] = group;
    this->encoded_index++;

    // Clear group
    group = 0;

    // Add values of vector
    for (size_t i = 0; i < group_vec.size(); i++) {
      this->encoded_buff[this->encoded_index] = group_vec[i];
      this->encoded_index++;
    }

    // Clear vector
    group_vec.clear();
  }
}

/**
 * Add counter value with value that 
 * COUNTER DOES NOT USE VALUES 0 AND 1, so we will use these values, so we can save into uint8_t values from 2 to 257, because 0 => 2, 1 => 3 ...255 => 257
 * @param[out] group_vec Vector of 8 values that will be appended to buffer after group byte
 * @param[out] group Group byte, 1 represents counter, 0 represents value
 * @param[in] val Value to be added to buffer
 * @param[in] counter Counter value that will be added before value
 * */
void RleCompressor::appendCounterValue(
  std::vector<uint8_t> &group_vec,
  uint8_t &group,
  const uint8_t &val,
  size_t &counter
) {
  // When given counter, is bigger than 1, start adding counter split into 8bit values
  if (counter > 1) {
    // Vector to hold values
    std::vector<uint8_t> counter_values;

    // When we got value 2, save it separately
    if (counter == 2) {
      counter_values.push_back(0);
    }

    // values 0 and 1 are unused
    counter -= 2;

    // Start converting counter into 8bit values
    while(counter > 0) {
      counter_values.push_back((counter & MAX_COUNTER_VAL));
      counter = (counter >> 8);
    }

    // Append values into buffer
    for (int8_t i = (counter_values.size() - 1); i >= 0; i--) {
      this->appendToBuff(group_vec, group, counter_values[i], true, false, NO_SETTINGS);
    }

    // Reset counter to 1
    counter = 1;
  }

  // Append value
  this->appendToBuff(group_vec, group, val, false, false, NO_SETTINGS);
}

/**
 * Horrizontally scan image data and convert them into RLE encrypted data
 * @param[in] width Width of image
 * @param[in] height Height of image
 * */
void RleCompressor::HorizontalScanning(
  const size_t &width,
  const size_t &height
) {
  // Set counter to 1
  size_t counter = 1;
  // Calculate image size
  size_t size = width * height;
  // Copy first pixel
  uint8_t pixel = buffer[0];

  // Variables that will be used for converting into 1 GROUP BYTE and 8 DATA BYTES
  uint8_t group = 0;
  std::vector<uint8_t> group_vec;

  // Start looping through all values byte by byte
  for (size_t i = 1; i < size; i++) {
    // Pixel is the same increment counter and move to another value
    if (this->buffer[i] == pixel) {
      counter++;
      continue;
    }

    // Append Counter with its value to buffer
    this->appendCounterValue(group_vec, group, pixel, counter);

    // Set new pixel to be compared to
    pixel = this->buffer[i];
  } 

  // Add last value
  this->appendCounterValue(group_vec, group, pixel, counter);

  // Push all values, that were not pushed yet, as padding
  if (group_vec.size() > 0) {
    this->appendToBuff(group_vec, group, UINT8_T_PADDING, false, true, NO_SETTINGS);
  }
}

/**
 * Vertically scan image data and convert them into RLE encrypted data
 * @param[in] width Width of image
 * @param[in] height Height of image
 * */
void RleCompressor::VerticalScanning(
  const size_t &width,
  const size_t &height
) {
  // Set counter to 1
  size_t counter = 1;
  uint8_t pixel;

  // Variables that will be used for converting into 1 GROUP BYTE and 8 DATA BYTES
  uint8_t group = 0;
  std::vector<uint8_t> group_vec;

  // Start going through image vertically
  for (size_t x = 0; x < width; x++) {
    for (size_t y = 0; y < height; y++) {
      // Save first value to pixel
      if (x == 0 && y == 0) {
        pixel = this->buffer[0];
        continue;
      }

      // Pixel the same, increment counter and move to another
      if (this->buffer[y * width + x] == pixel) {
        counter++;
        continue;
      }

      // Append value to buffer
      this->appendCounterValue(group_vec, group, pixel, counter);

      // Set new pixel
      pixel = this->buffer[y * width + x];
    }
  }

  // Add last value
  this->appendCounterValue(group_vec, group, pixel, counter);

  // Push all values, that were not pushed yet, as padding 
  if (group_vec.size() > 0) {
    this->appendToBuff(group_vec, group, UINT8_T_PADDING, false, true, NO_SETTINGS);
  }
}

/**
 * Start sequence scanning of image and convert it into RLE encoded data
 * @param[in] width Width of image
 * @param[in] height Height of image
 * @param[in] input_preprocessing True when image was preprocessed, false otherwise
 * */
void RleCompressor::SequenceScanning(
  const size_t &width,
  const size_t &height,
  const bool &input_preprocessing
) {
  // Set first and second bits when input preprocessing is true, otherwise only first bit indicating horizontal scanning
  uint8_t settings = (input_preprocessing) ? (SCANNING_MASK | MODEL_MASK) : (SCANNING_MASK);
  
  // Append settings byte to buffer with image width and height
  this->appendSettingsToBuff(settings, width, height);

  // Do horrizontal scanning
  this->HorizontalScanning(width, height);
}

/**
 * Start adaptive scanning where we choose the best scanning type that reduces the data the most
 * @param[in] width Width of image
 * @param[in] height Height of image
 * @param[in] input_preprocessing True when image was preprocessed, false otherwise
 * */
void RleCompressor::AdaptiveScanning(
  const size_t &width,
  const size_t &height,
  const bool &input_preprocessing
) {
  // Set scanning bit and model bit to true when input preprocessing is true, or only scanning bit otherwise
  uint8_t horizontal_settings = (input_preprocessing) ? (SCANNING_MASK | MODEL_MASK) : (SCANNING_MASK);
  
  // Set model bit to true when input preprocessing is true, or nothing otherwise
  uint8_t vertical_settings = (input_preprocessing) ? (MODEL_MASK) : 0;

  // Append settings to buffer with image width and height
  this->appendSettingsToBuff(horizontal_settings, width, height);

  // Do horizontal scanning
  this->HorizontalScanning(width, height);

  // Save buffer data of horrizontal scanning into temporally variables
  uint8_t *tmp_buff = this->encoded_buff;
  const size_t tmp_buff_alloc = this->encoded_alloc;
  const size_t tmp_buff_index = this->encoded_index;

  // Clear buffer
  this->encoded_buff = nullptr;
  this->encoded_alloc = 0;
  this->encoded_index = 0;

  // Append settings to buffer with image width and height
  this->appendSettingsToBuff(vertical_settings, width, height);

  // Do verticall scanning
  this->VerticalScanning(width, height);

  // Vertical scanning has better compression ratio, free horizontal buffer
  if (this->encoded_index <= tmp_buff_index) {
    free(tmp_buff);
    return;
  }

  // Horizontal scanning has better compression ratio, free verticall buffer
  free(this->encoded_buff);

  // Set back horrizontal buffer, saved in temporally variables
  this->encoded_buff = tmp_buff;
  this->encoded_alloc = tmp_buff_alloc;
  this->encoded_index = tmp_buff_index;
}

/**
 * Return pointer to decompressed data buffer
 * @returns Pointer to buffer
 * */
uint8_t * & RleCompressor::GetBuffer() {
  return this->encoded_buff;
}

/**
 * Return decompressed data buffer size
 * @returns Size of buffer
 * */
size_t & RleCompressor::GetSize() {
  return this->encoded_index;
}
