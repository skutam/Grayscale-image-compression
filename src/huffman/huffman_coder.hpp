/**
 * Author: Matúš Škuta (xskuta04)
 * Date created: 07.05.2021
 * Name: huffman_coder.hpp
 * Description: Contains definitions of class HuffmanCoder, that is used to encode
 * any binary data into huffman code
 * */
#ifndef __HUFFMAN_CODER__
#define __HUFFMAN_CODER__

#include <iostream> // cout
#include <algorithm> // cout

#include "huffman.hpp"

/**
 * Class that will encode data to huffman code
 * */
class HuffmanCoder {
private:
  // Tree pointers
  Node *root;
  Node *NYT;
  Node **leaf_nodes;

  // Bit output
  uint8_t *buffer;
  uint64_t alloc;
  uint64_t byte_index;
  uint8_t bit_index;

  /**
   * When about 20 bytes are remaining of buffer, increase buffer
   * */
  void ReallocateBuffer();

  /**
   * Add vector of 0 and 1 values into buffer as bits, in REVERSE order
   * @param[in] bits Vector of 0 and 1 values
   * */
  void AddBits(const std::vector<uint8_t> &bits);

  /**
   * Convert byte value into vector of 0 and 1, and call AddBits
   * @param[in] byte Byte valule to be converted into vector and added to buffer
   * */
  void AddByte(const uint8_t &byte);

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
  Node* AddSymbol(const uint8_t &symbol);

  /**
   * Return pointer saved in array of leaf node pointers
   * @param[in] symbol Index of required pointer
   * @returns Node pointer, when value exist, nullptr otherwise
   * */
  Node* FindSymbol(const uint8_t &symbol);

  /**
   * Find path from given node to root, and save path as vector of 0's and 1's
   * @param[in] node Node from which our search begins
   * @param[out] path Vector that will contain resulting path of 0's and 1'
   * */
  void FindPathToRoot(Node *node, std::vector<uint8_t> &path);

  /**
   * Search tree through BFS method, that will firstly add to queue right then left node
   * @param[in] node Node weight and index to be compared against all other nodes
   * @returns First found node or given node when no node is found
   * */
  Node* FindHighestBlockNode(Node *node);

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

  /**
   * Compare if compressed data are lower than RLE, when not copy RLE back to buffer, and add
   * settings that will tell us if data are huffman or RLE
   * */
  void CompareWithRLE(uint8_t * &buffer, const size_t &size, uint8_t &settings);

public:
  /**
   * Constructor that will initialize values, and huffman tree
   * */
  HuffmanCoder ();

  /**
   * Deconstructor that will free allocated values
   * */
  ~HuffmanCoder();

  /**
   * Encode RLE data to huffman code
   * @param[in] buffer Buffer containing RLE data
   * @param[in] size Size of buffer in bytes
   * @param[out] settings Byte containing metadata
   * */
  void Encode(uint8_t * &buffer, const size_t &size, uint8_t &settings);

  /**
   * Return pointer to encoded data buffer
   * @returns Pointer to buffer
   * */
  uint8_t * & GetBuffer();

  /**
   * Return size of buffer based on bit index
   * */
  uint64_t GetSize();
};

#endif
