/**
 * Author: Matúš Škuta (xskuta04)
 * Date created: 07.05.2021
 * Name: huffman_decoder.hpp
 * Description: Contains definitions of class HuffmanDecoder, that is used to decode
 * hufman code into binary data 
 * */
#ifndef __HUFFMAN__DECODER__
#define __HUFFMAN__DECODER__

#include "huffman.hpp"

/**
 * Class that will decode huffman encoded data
 * */
class HuffmanDecoder {
private:
  // Buffer that will hold decoded data
  uint8_t *buffer;
  uint64_t alloc;
  uint64_t write_byte_index;

  // Indexes of buffer from which we are reading
  uint64_t read_byte_index;
  uint8_t read_bit_index;

  // Tree pointers
  Node *root;
  Node *NYT;

  /**
   * When about 20 bytes are remaining of buffer, increase buffer
   * */
  void ReallocateBuffer();

  /**
   * Add symbol to buffer
   * @param[in] symbol Symbol to be added to buffer
   * */
  void AddSymbolToBuffer(const uint8_t &symbol);
  
  
  /**
   * Read 8bits from buffer and convert them to byte
   * @param[in] buffer Buffer containing encoded data
   * @param[in] size Size of buffer in bytes
   * @param[out] symbol Here we will set 8bits that we will read
   * @returns True when there are still data, false when we reached end of buffer
   * */
  bool ReadSymbol(uint8_t * &buffer, const uint64_t &size, uint8_t &symbol);
    
  /**
   * Get next bit in buffer as boolean value and return it
   * @param[in] buffer Buffer containing encoded data
   * @param[in] size Size of buffer in bytes
   * @param[out] end_of_buffer When we reach end of buffer, set to true
   * @returns Next bit in buffer as boolean value
   * */
  bool NextBit(uint8_t * &buffer, const uint64_t &size, bool &end_of_buffer);
    
  /**
   * Check if we reached padding bits
   * @returns True when we reached padding bits, false otherwise
   * */
  bool IsEnd(const uint64_t &size, const uint8_t &padding_bits);

  /**
   * Initialize huffman tree, with first NYT node
   * */
  void InitTree();

  /**
   * Allocate memory for Node structure and initialize its values
   * @returns Pointer to newly created Node structure
   * */
  Node* GenNode();

  /**
   * Add new NYT node with value node to the tree, after current NYT node
   * @param[in] symbol Value to be added to the tree
   * @returns Return pointer to the old NYT node
   * */
  Node* AddSymbol(const uint8_t & symbol);
  
  /**
   * Search tree through BFS method, that will firstly add to queue right then left node
   * @param[in] node Node weight and index to be compared against all other nodes
   * @returns First found node or given node when no node is found
   * */
  Node* FindHighestBlockNode(Node *node);

  /**
   * Check whetever given node is external or note
   * @param[in] node Node that may be external
   * @returns True when node is external, false otherwise
   * */
  bool IsExternalNode(Node *node);

  /**
   * Swap position of two nodes with its children
   * @param[in] node1 Node1 that will be swapped with node2
   * @param[in] node2 Node2 that will be swapped with node1
   * */
  void SwapNodes(Node *node1, Node *node2);

  /**
   * Free all child nodes recursively
   * @param[in] node Node to be freed
   * */
  void FreeNode(Node *node);

public:
  /**
   * Constructor that will initialize values, and huffman tree
   * */
  HuffmanDecoder();

  /**
   * Deconstructor that will free allocated values
   * */
  ~HuffmanDecoder();

  /**
   * Decode huffman encoded data
   * @param[in] buffer Buffer containing huffman encoded values
   * @param[in] size Size of data in buffer
   * @returns True when decode was successfull, false otherwise
   * */
  bool Decode(uint8_t * & buffer, const uint64_t &size);

  /**
   * Return pointer to compressed data buffer
   * @returns Pointer to buffer
   * */
  uint8_t * & GetBuffer();

  /**
   * Return compressed data buffer size
   * @returns Size of buffer
   * */
  uint64_t GetSize();
};

#endif
