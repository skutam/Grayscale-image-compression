/**
 * Author: Matúš Škuta (xskuta04)
 * Date created: 07.05.2021
 * Name: data_worker.cpp
 * Description: Contains implementations of DataWorker class, that is used to
 * load data to buffer from files and write data from buffer to file
 * */
#include "data_worker.hpp"

/**
 * Constructor for initialization of class variables
 * */
DataWorker::DataWorker() {
  this->buff_size = 0;
  this->buffer = nullptr;
}

/**
 * Free all resources before destroying object
 * */
DataWorker::~DataWorker() {
  if (this->buffer) {
    free(this->buffer);
  }
}

/******************************************************************************
********************************PUBLIC-FUNCTIONS*******************************
******************************************************************************/

/**
 * Calculate difference of pixels than save them to class buffer
 * */
void DataWorker::Preprocess() {
  // Allocate temporaly buffer for difference of pixels
  uint8_t *diff_pixels = (uint8_t *)malloc(sizeof(uint8_t) * this->buff_size);

  // Copy first value
  diff_pixels[0] = this->buffer[0];

  for (size_t i = 1; i < this->buff_size; i++) {
    // When first value is higher than second, calculate overflow
    diff_pixels[i] = (this->buffer[i] - this->buffer[(i - 1)]);
  }

  // When buffer was allocated, copy values to buffer
  if (this->buffer) {
    memcpy(this->buffer, diff_pixels, this->buff_size);
  }

  // Free temporal buffer
  free(diff_pixels);
}

/**
 * Calculate original image back from pixels differencial
 * @param[out] buffer Containing data, from which we will calculate result and store him back here
 * @param[in] size Size of buffer
 * */
void DataWorker::Depreprocess(uint8_t * &buffer, const size_t &size) {
  // Allocate temporal buffer for original image
  uint8_t *orig_image = (uint8_t *)malloc(sizeof(uint8_t) * size);

  // Copy first value
  orig_image[0] = buffer[0];

  // Costruct image back
  for (size_t i = 1; i < size; i++) {
    orig_image[i] = orig_image[(i - 1)] + buffer[i];
  }

  // Copy original image back
  memcpy(buffer, orig_image, size);

  // Free memory
  free(orig_image);
}

/**
 * Load raw image into buffer and calculate height from width and file size
 * @param[in] filename Name of file to be loaded
 * @param[in] width Width of file
 * @param[out] height Height of file that will be calculated
 * @returns True when we succesfully loaded file into buffer, false otherwise
 * */
bool DataWorker::LoadRawImage(std::string &filename, const uint32_t &width, uint32_t &height) {
  // File pointer
  FILE *file;
  uint64_t result;

  // Open file
  file = std::fopen(filename.c_str(), "rb");

  // File is not open, exit
  if (file == nullptr) {
    std::cerr << "File could not be opened!" << std::endl;
    return false;
  }

  // Move to the end of file
  if (fseek(file, 0, SEEK_END)) {
    std::cerr << "Fseek failed to roll to end of file!" << std::endl;
    return false;
  }

  // Get file position, in bytes
  this->buff_size = ftell(file);

  // Move pointer to the start of file
  if (fseek(file, 0, SEEK_SET)) {
    std::cerr << "Fseek failed to roll to start of file!" << std::endl;
    return false;
  }

  // Calculate height
  height = this->buff_size / width;

  // Allocate memory for file
  this->buffer = (uint8_t *)malloc(sizeof(uint8_t) * this->buff_size);

  // Copy file into buffer
  result = fread(this->buffer, BYTE_SIZE, this->buff_size, file);

  // Check if given file is not smaller than id needs to be
  if (result != this->buff_size) {
    std::cerr << "Failed to read file!" << std::endl;
    return false;
  }

  // Close and return success
  std::fclose(file);
  return true;
}

/**
 * Load encoded data from given file
 * @param[in] filename Name of file to be loaded
 * @returns True when we successfully loaded file into buffer, false otherwise
 * */
bool DataWorker::LoadEncodedData(std::string &filename) {
  // Pointer to open file
  FILE *file;
  uint64_t result;

  // Open file
  file = fopen(filename.c_str(), "rb");

  // File is not open, exit
  if (file == nullptr) {
    std::cerr << "File could not be opened!" << std::endl;
    return false;
  }

  // Move to the end of file
  if (fseek(file, 0, SEEK_END)) {
    std::cerr << "Fseek failed to roll to end of file!" << std::endl;
    return false;
  }

  // Get file position, in bytes
  this->buff_size = ftell(file);

  // Move pointer to the start of file
  if (fseek(file, 0, SEEK_SET)) {
    std::cerr << "Fseek failed to roll to start of file!" << std::endl;
    return false;
  }

  // Allocate memory
  this->buffer = (uint8_t *)malloc(sizeof(uint8_t) * this->buff_size);

  // Copy file into buffer
  result = fread(this->buffer, BYTE_SIZE, this->buff_size, file);

  // Check if given file is not smaller than it needs to be
  if (result != this->buff_size) {
    std::cerr << "Failed to read file!" << std::endl;
    return false;
  }

  // Close and return success
  std::fclose(file);
  return true;
}

/**
 * Write RAW image data into specified file
 * @param[in] filename Name of file the data will be written to
 * @param[in] buffer Buffer that will be written into file
 * @param[in] size Number of bytes to be written into file
 * @param[in] decompress_model When true, call Depreprocess on buffer, before writting data to file
 * */
bool DataWorker::WriteRawImage(
  std::string &filename,
  uint8_t * &buffer,
  const size_t &size,
  const bool &decompress_model
) {
  // Calculate back original image from pixel differencial
  if (decompress_model) {
    this->Depreprocess(buffer, size);
  }

  // Open file for binary writting
  FILE *file = fopen(filename.c_str(), "wb");
  uint64_t result;

  // Failed to open file
  if (file == nullptr)
  {
    return false;
  }
  

  // Write in buffer
  result = fwrite(buffer, sizeof(uint8_t), size, file);

  // Failed to write all data to file
  if (result != size)
  {
    return false;
  }
  
  // Close file
  std::fclose(file);
  return true;
}

/**
 * Write encoded data into specified file
 * @param[in] filename Name of file the data will be written to
 * @param[in] settings Byte containing metadata to be written as first byte before buffer
 * @param[in] buffer Buffer that will be written after settings byte into file
 * @param[in] size Number of bytes to be written into file
 * @returns True when successfuly written into file
 * */
bool DataWorker::WriteEncodedData(
  std::string &filename,
  const uint8_t &settings,
  uint8_t * &buffer,
  const uint64_t &size
) {
  // Open file for binary writting
  std::FILE *file = fopen(filename.c_str(), "wb");
  uint64_t result;
  
  // File is not open
  if (file == nullptr) {
    return false;
  }

  // Write settings
  std::fwrite(&settings, sizeof(uint8_t), 1, file);

  // After settings write data
  result = std::fwrite(buffer, sizeof(uint8_t), size, file);
  
  // Failed to write all data to file
  if (result != size)
  {
    return false;
  }
  // Close file
  std::fclose(file);
  return true;
}

/**
 * Return pointer to class buffer
 * @returns Pointer to buffer
 * */
uint8_t * & DataWorker::GetBuffer() {
  return this->buffer;
}

/**
 * Return buffer size
 * @returns Size of buffer
 * */
const uint64_t & DataWorker::GetSize() {
  return this->buff_size;
}