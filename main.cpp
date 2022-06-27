/**
 * Author: Matúš Škuta (xskuta04)
 * Date created: 07.05.2021
 * Name: main.cpp
 * Description: Main file used for parsing arguments, and based on given arguments,
 * start compressing or decompressing data
 * */
#include <string>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <cstdint>  // uint32_t


#include "src/data_worker.hpp"
#include "src/huffman/huffman_coder.hpp"
#include "src/huffman/huffman_decoder.hpp"
#include "src/rle/rle_compressor.hpp"
#include "src/rle/rle_decompressor.hpp"

/**
 * Function will parse arguments and assign their values to given variables
 * @param[in] argc Number of arguments
 * @param[in] argv Array of arguments
 * @param[out] compress_decompress Set to true when param -c is present or false when -d is present,
 * @param[out] input_preprocessing Set to true when param -m is present, false otherwise
 * @param[out] adaptive_sequence_scanning Set to true when param -a is present, false otherwise
 * @param[out] input_file Set to name of file specified in -i param
 * @param[out] output_file Set to name of file specified in -o param
 * @param[out] input_width Set to number specified in -w param
 * @param[out] help Set to true when -h argument is present
 * @return True when all arguments were rightly formatted, false otherwise
 **/
bool parse_arguments(
  const int &argc,
  char* argv[],
  bool &compress_decompress,
  bool &input_preprocessing,
  bool &adaptive_sequence_scanning,
  std::string &input_file,
  std::string &output_file,
  uint32_t &input_width,
  bool &help
) {
  // -c, -d are mandatory, used for checking if one of them was set
  bool compress_decompress_set = false;

  // Set default values before looping through arguments
  input_preprocessing = false;
  adaptive_sequence_scanning = false;
  input_file = "";
  output_file = "";
  input_width = 0;

  int opt;

  // Loop through all arguments
  while ((opt = getopt(argc, argv, ":cdmaw:i:o:h")) != -1) {
    switch (opt) {
      // Compress argument
      case 'c':
        compress_decompress = true;
        compress_decompress_set = true;
        break;
      // Decompress argument
      case 'd':
        compress_decompress = false;
        compress_decompress_set = true;
        break;
      // Model preprocessing of image argument
      case 'm':
        input_preprocessing = true;
        break;
      // Adaptive scanning in RLE argument
      case 'a':
        adaptive_sequence_scanning = true;
        break;
      // Input image argument
      case 'i':
        input_file = optarg;
        break;
      // Output image argument
      case 'o':
        output_file = optarg;
        break;
      // Width of image argument
      case 'w':
        {
          std::stringstream sstream(optarg);
          sstream >> input_width;
          if (input_width < 1) {
            std::cerr << "Input width, needs to be >= 1!" << std::endl;
            return false;
          }
        }
        break;
      // Helo argument
      case 'h':
        // Print out help and exit
        help = true;
        return true;
        break;

      case ':':
        std::cout << "Option needs a value" << std::endl;
        break;

      case '?':
        std::cout << "Unknown param" << std::endl;
        break;
    }
  }

  // Check we -c or -d were set
  if (!compress_decompress_set) {
    std::cerr << "Param -c or -d are mandatory!" << std::endl;
    return false;
  }

  // Check if we were given input file
  if (input_file == "") {
    std::cerr << "Input file is mandatory!" << std::endl;
    return false;
  }

  // Check if we were given output file
  if (output_file == "") {
    std::cerr << "Output file is mandatory!" << std::endl;
    return false;
  }

  // When compressing, we require width
  if (compress_decompress && input_width == 0) {
    std::cerr << "Width of input is mandatory with param -c!" << std::endl;
    return false;
  }

  // Extra arguments given
  if (optind < argc) {
    std::cerr << "Extra arguments given, remove these arguments and try again, for help type -h!" << std::endl;
    return false;
  }

  return true;
}

/**
 * Print help text
 * */
void print_help() {
  std::cout <<
  "Program to compress and decompress RAW 8 bit grayscale images\n"
  "Usage:\n"
    "./huff_codec -c -i image.raw -o compressed_image -w 512\n"
    "./huff_codec -c -i image.raw -o compressed_image -w 512 -a\n"
    "./huff_codec -c -i image.raw -o compressed_image -w 512 -m\n"
    "./huff_codec -c -i image.raw -o compressed_image -w 512 -a -m\n"
    "./huff_codec -d -i compressed_image -o image.raw\n"
    "./huff_codec -h\n\n"
  "Options:\n"
    "-h\t\tShow this screen.\n"
    "-c\t\tCompress input image.\n"
    "-d\t\tDecompress input data.\n"
    "-i=<filename>\tSpecify input file that is either RAW image when -c is pressent or compressed data when -d is present.\n"
    "-o=<filename>\tSpecify output file name that will be either RAW image when -d is pressent or compressed data when -c is present.\n"
    "-w=<width>\tSpecify width of image, value needs to be higher than 0.\n"
    "-m\t\tSpecify to use preprocessing of image, that will calculate difference of pixels.\n"
    "-a\t\tSpecify to use adaptive scanning for RLE algorithm, that will choose option that reduces image the most.\n";
}

/**
 * Starting point of program
 * */
int main(int argc, char *argv[]) {
  // Variables for configuring program
  bool compress_decompress;
  bool input_preprocessing;
  bool adaptive_sequence_scanning;
  std::string input_file;
  std::string output_file;
  uint32_t width;
  uint32_t height;
  bool help = false;

  // Parse agruments
  if (!parse_arguments(argc, argv, compress_decompress, input_preprocessing, adaptive_sequence_scanning, input_file, output_file, width, help)) {
    return -1;
  }

  // Exit program after printing help menu
  if (help) {
    print_help();
    return 0;
  }

  // Initialize data worker
  DataWorker data_worker;

  if (compress_decompress) {
    /**********************************COMPRESSING*************************************/

    // Load raw image, with its height
    if (!data_worker.LoadRawImage(input_file, width, height)) {
      return -1;
    }

    // Preprocess data when compressing and argument -m was set
    if (input_preprocessing) {
      data_worker.Preprocess();
    }

    // Initialize RLE compressor
    RleCompressor rle_compressor(data_worker.GetBuffer(), width, height);

    // When given argument -a, do adaptive scanning
    if (adaptive_sequence_scanning) {
      rle_compressor.AdaptiveScanning(width, height, input_preprocessing);
    // Otherwise do normal horizontal scanning
    } else {
      rle_compressor.SequenceScanning(width, height, input_preprocessing);
    }

    // Initialize huffman coder
    HuffmanCoder huffman_coder;
    uint8_t settings = 0;

    // Do huffman encoding
    // Huffman encoding, will compare RLE data length with huffman result length
    // And when huffman is lower will return him
    // Otherwise will return RLE and not use huffman
    // Which will be saved in first byte, that will also contain number of padding bits
    huffman_coder.Encode(rle_compressor.GetBuffer(), rle_compressor.GetSize(), settings);

    // Write setting byte and data to file
    if (!data_worker.WriteEncodedData(output_file, settings, huffman_coder.GetBuffer(), huffman_coder.GetSize()))
    {
      std::cerr << "Failed to write encoded data to given file." << std::endl;
      return -1;
    }

    return 0;
  }

  /**********************************DECOMPRESSING*************************************/

  // When reading from file failed, return error
  if (!data_worker.LoadEncodedData(input_file))
  {
    std::cerr << "Failed to read from given file" << std::endl;
    return -1;
  }

  // Initialize huffman decoder
  HuffmanDecoder huffman_decoder;

  // Do huffman decoding
  // Check first byte, and when 4th bit is set, do huffman decoding and when not
  // Just copy data to output buffer because we are only using RLE
  if (!huffman_decoder.Decode(data_worker.GetBuffer(), data_worker.GetSize())) {
    return -1;
  }

  // Initialize RLE decompressor
  RleDecompressor rle_decompressor(huffman_decoder.GetBuffer(), huffman_decoder.GetSize());
  bool convert_from_model = false;

  // Decompress data
  if (!rle_decompressor.Decompress(convert_from_model))
  {
    std::cerr << "Failed to decompress given data, invalid data" << std::endl;
    return -1;
  }

  // Write image to file and when `convert_from_model` is true, preprocess data before saving to file
  if (!data_worker.WriteRawImage(output_file, rle_decompressor.GetBuffer(), rle_decompressor.GetSize(), convert_from_model))
  {
    std::cerr << "Failed to write RAW image data into given file." << std::endl;
    return -1;
  }
  return 0;
}
