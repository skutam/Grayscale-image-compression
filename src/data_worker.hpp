/**
 * Author: Matúš Škuta (xskuta04)
 * Date created: 07.05.2021
 * Name: data_worker.hpp
 * Description: Contains definitions of DataWorker class, that is used to
 * load data to buffer from files and write data from buffer to file
 * */
#ifndef __DATA_WORKER__
#define __DATA_WORKER__

#include <string>
#include <string.h>
#include <iostream>
#include <cstdint>

constexpr int BYTE_SIZE = 1;

/**
 * Class will load data from file or write data to file
 * */
class DataWorker {
private:
  // Buffer that will hold file data
  uint8_t *buffer;
  // Size of buffer, that is also allocated size of buffer
  uint64_t buff_size;

public:
  /**
   * Constructor
   * */
  DataWorker();

  /**
   * Deconstructor
   * */
  virtual ~DataWorker ();

  /**
   * Calculate difference of pixels than save them to class buffer
   * */
  void Preprocess();

  /**
   * Calculate original image back from pixels differencial
   * @param[out] buffer Containing data, from which we will calculate result and store him back here
   * @param[in] size Size of buffer
   * */
  void Depreprocess(uint8_t * &buffer, const size_t &size);

  /**
   * Load raw image into buffer and calculate height from width and file size
   * @param[in] filename Name of file to be loaded
   * @param[in] width Width of file
   * @param[out] height Height of file that will be calculated
   * @returns True when we succesfully loaded file into buffer, false otherwise
   * */
  bool LoadRawImage(std::string &filename, const uint32_t &width, uint32_t &height);
  
  /**
   * Load encoded data from given file
   * @param[in] filename Name of file to be loaded
   * @returns True when we successfully loaded file into buffer, false otherwise
   * */
  bool LoadEncodedData(std::string &filename);

  /**
   * Write RAW image data into specified file
   * @param[in] filename Name of file the data will be written to
   * @param[in] buffer Buffer that will be written into file
   * @param[in] size Number of bytes to be written into file
   * @param[in] decompress_model When true, call Depreprocess on buffer, before writting data to file
   * */
  bool WriteRawImage(
    std::string &filename,
    uint8_t * &buffer,
    const size_t &size,
    const bool &decompress_model
  );

  /**
   * Write encoded data into specified file
   * @param[in] filename Name of file the data will be written to
   * @param[in] settings Byte containing metadata to be written as first byte before buffer
   * @param[in] buffer Buffer that will be written after settings byte into file
   * @param[in] size Number of bytes to be written into file
   * @returns True when successfuly written into file
   * */
  bool WriteEncodedData(
    std::string &filename,
    const uint8_t &settings,
    uint8_t * &buffer,
    const uint64_t &size
  );

  /**
   * Return pointer to class buffer
   * @returns Pointer to buffer
   * */
  uint8_t * & GetBuffer();

  /**
   * Return buffer size
   * @returns Size of buffer
   * */
  const uint64_t & GetSize();
};

#endif
