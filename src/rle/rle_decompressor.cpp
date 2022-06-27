/**
 * Author: Matúš Škuta (xskuta04)
 * Date created: 07.05.2021
 * Name: rle_decompressor.cpp
 * Description: Contains implementations of RLE decompressor class that is used to decompress
 * RLE encoded data into raw grayscale 8bit images
 * */
#include "rle_decompressor.hpp"

/**
 * Constructor for RLE_Decompressor that will initialize values
 * @param[in] buffer Data buffer holding compressed RLE data
 * @param[in] size Size of data buffer
 * */
RleDecompressor::RleDecompressor(uint8_t * &buffer, const size_t &size) {
  // Receive buffer
  this->buffer = buffer;
  this->size = size;
  this->index = 0;

  // Initialize decompressed data buffer
  this->dec_buffer = nullptr;
  this->dec_buffer_alloc = 0;
  this->dec_buffer_index = 0;
}

/**
 * Deconstructor for RLE_Decompressor that will free allocated data
 * */
RleDecompressor::~RleDecompressor() {
  // Free allocated decompressed data buffer, when one was allocated
  if (this->dec_buffer != nullptr) {
    free(this->dec_buffer);
  }

  // Destroy pointer to outside buffer
  this->buffer = nullptr;
}

/**
 * Decompress image horizontally
 * @returns True when image has been horrizontally decompressed, false otherwise
 * */
bool RleDecompressor::DecompressHorizontally() {
  // Number of times to copy value val
  size_t count = 0;
  uint8_t val = 0;

  uint8_t bit_index = 0;
  uint8_t byte = 0;

  // While getting data from function, keep setting them into buffer
  while (this->GetValCount(count, val, bit_index, byte)) {
    // Stop with decompression
    if ((this->dec_buffer_index + count) > this->dec_buffer_alloc)
    {
      size_t tmp = (this->dec_buffer_alloc - this->dec_buffer_index);

      if (tmp > 0)
      {
        // Set remaining data to be written and end
        memset(((this->dec_buffer) + this->dec_buffer_index), val, tmp);
      }

      // Set index as alloc, and exit loop
      this->dec_buffer_index = this->dec_buffer_alloc;
      break;
    }

    // Set 'count' numbers of value
    memset(((this->dec_buffer) + this->dec_buffer_index), val, count);
    
    // Increment buffer index
    this->dec_buffer_index += count;
  }

  // Check if all data were decompressed
  return this->dec_buffer_index == this->dec_buffer_alloc;
}

/**
 * Decompress image vertically
 * @returns True when image has been vertically decompressed, false otherwise
 * */
bool RleDecompressor::DecompressVertically(
  const size_t &width,
  const size_t &height
) {
  // Count represent number of times val needs to be replicated
  size_t count = 0;
  uint8_t val = 0;

  uint8_t bit_index = 0;
  uint8_t byte = 0;

  // X and Y represent positions on image
  size_t x = 0;
  size_t y = 0;

  // Calculate image size
  size_t image_size = width * height;

  // Keep setting data until you keep getting data
  while (this->GetValCount(count, val, bit_index, byte)) {
    // Set given val, count times
    for (size_t i = 0; i < count; i++) {
      // Calculate index
      this->dec_buffer_index = (y * width + x);

      // When index is equal or higher than image size, end, invalid data
      if (this->dec_buffer_index >= image_size)
      {
        return false;
      }

      // On given index add value
      this->dec_buffer[this->dec_buffer_index] = val;
      y++;

      // When reached bottom, move to the right
      if (y == height) {
        y = 0;
        x++;
      }
    }
  }

  // Check if we succesfully decompressed image, we need to end on index (height * width - 1)
  if (this->dec_buffer_index != (image_size - 1))
  {
    return false;
  }

  // Set index to image size, because we will be writting it into file
  this->dec_buffer_index = image_size;
  return true;
}

/**
 * Convert RLE compressed data into count and val
 * @param[out] count Number of times to replicate val value
 * @param[out] val Value to be replicated
 * @param[out] bit_index Represent actual index of bit while reading bit by bit
 * @param[out] byte Represent actual group byte, representing which bits are count bits and which are value bits
 * @returns True while there are still values, false otherwise
 * */
bool RleDecompressor::GetValCount(
  size_t &count,
  uint8_t &val,
  uint8_t &bit_index,
  uint8_t &byte
) {
  // Will represent if we encountered count value
  bool count_bit = false;

  // Reset values
  count = 0;
  val = 0;

  // Loop until we reach end of buffer
  while (this->index < this->size) {
    // Load group byte, representing count and values
    if (bit_index == 0) {
      byte = this->buffer[this->index++];
    }

    // Keep reading values from byte
    while (bit_index < UINT8_T_SIZE) {
      // When first bit is 1, value is counter, convert to number
      if (byte & (FIRST_BIT_MASK << (bit_index++))) {
        count_bit = true;
        count |= this->buffer[this->index++];
        count = (count << UINT8_T_SIZE);
        continue;
      }

      // When count bit is set, convert count to number
      if (count_bit) {
        count = (count >> UINT8_T_SIZE);
        count += 2;
      // Only value was, no counters were given
      } else {
        count = 1;
      }

      // Set value
      val = this->buffer[this->index++];
      return true;
    }

    // Reset bit index
    bit_index = 0;
  }

  // No more values, return false
  return false;
}

/**
 * Read width and height from metadata
 * @param[out] width Width of image got from metadata
 * @param[out] height Height of image got from metadata
 * @returns True when we successfully got size from metadata, false otherwise
 * */
bool RleDecompressor::GetSize(uint32_t &width, uint32_t &height) {
  // Number of width bytes following setting byte
  const uint8_t count_w = ((this->buffer[0] & WIDTH_COUNT_MASK) >> 3) + 1;
  // Number of bytes following after last width byte
  const uint8_t count_h = ((this->buffer[0] & HEIGHT_COUNT_MASK)) + 1;
  // Size of width and height bytes
  const uint8_t count = count_w + count_h;
  
  // Index of byte
  uint16_t i = 0;

  // Check if required bytes are in buffer
  if (this->size < (count_h + count_w)) {
    return false;
  }

  // Convert width data from bytes into one big value
  for (; i < count_w; i++) {
    width |= this->buffer[i + 1];
    if ((i + 1) < count_w) {
      width = (width << 8);
    }
  }

  // Convert height data from bytes into one big value
  for (; i < count; i++) {
    height |= this->buffer[i + 1];
    if ((i + 1) < count) {
      height = (height << 8);
    }
  }

  // Move index, where the data are, settings byte + width bytes + height bytes
  this->index = (count_w + count_h + 1);
  return true;
}


/**
 * Decompress RLE data
 * @param[out] convert_from_model Set to true when first settings byte has -m bit set
 * @returns True when decompression was successfull, false otherwise
 * */
bool RleDecompressor::Decompress(bool &convert_from_model) {
  // When given size is 0, no buffer was given
  if (this->size == 0) {
    std::cerr << "No buffer given" << std::endl;
    return false;
  }

  // Variables into which we will load width and height, from metadata
  uint32_t width = 0;
  uint32_t height = 0;

  // Check what type of decompression we are going to do from settings byte
  bool horizontal_decompress = (this->buffer[0] & SCANNING_MASK);

  // Set to true when bit representing -m is true
  convert_from_model = (this->buffer[0] & MODEL_MASK);

  // Load size of image from metadata
  if (!this->GetSize(width, height)) {
    std::cerr << "Buffer does not contain size!" << std::endl;
    return false;
  }

  // Allocate memory for image
  this->dec_buffer_alloc = (width * height);
  this->dec_buffer = (uint8_t *)malloc(this->dec_buffer_alloc * sizeof(uint8_t));

  // Invalid allocation
  assert(this->dec_buffer != nullptr);

  // Decompress image horrizontally
  if (horizontal_decompress) {
    return this->DecompressHorizontally();
  }

  // Decompress iamge vertically
  return this->DecompressVertically(width, height);
}

/**
 * Return pointer to decompressed data buffer
 * @returns Pointer to buffer
 * */
uint8_t * & RleDecompressor::GetBuffer() {
  return this->dec_buffer;
}

/**
 * Return decompressed data buffer size
 * @returns Size of buffer
 * */
size_t RleDecompressor::GetSize() {
  return this->dec_buffer_index;
}
