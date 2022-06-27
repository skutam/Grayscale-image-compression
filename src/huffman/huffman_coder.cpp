/**
 * Author: Matúš Škuta (xskuta04)
 * Date created: 07.05.2021
 * Name: huffman_coder.cpp
 * Description: Contains implementations of class HuffmanCoder, that is used to encode
 * any binary data into huffman code
 * */

#include "huffman_coder.hpp"

/**
 * Constructor that will initialize values, and huffman tree
 * */
HuffmanCoder::HuffmanCoder() {
    // Initialize buffer values
    this->alloc = 0;
    this->byte_index = 0;
    this->bit_index = 0;
    this->buffer = nullptr;

    // Allocate buffer for 512 bytes
    this->ReallocateBuffer();

    // Allocate memory for 256 possible values of leaf nodes
    this->leaf_nodes = (Node **)malloc(sizeof(Node *) * N_VALUES);

    // Set each to nullptr
    for (uint16_t i = 0; i < N_VALUES; i++)
    {
        this->leaf_nodes[i] = nullptr;
    }

    // Initialize tree
    this->InitTree();
}

/**
 * Deconstructor that will free allocated values
 * */
HuffmanCoder::~HuffmanCoder() {
    // When buffer was allocated free him
    if (this->buffer) {
        free(this->buffer);
    }

    // When array of leaf pointers was allocated, free him
    if (this->leaf_nodes)
    {
        free(this->leaf_nodes);
    }

    // When tree was allocated free him
    if (this->root)
    {
        this->FreeNode(this->root);
    }
}

/**
 * When about 20 bytes are remaining of buffer, increase buffer
 * */
void HuffmanCoder::ReallocateBuffer() {
    // When about 20 bytes are left, expand buffer
    if (this->alloc <= (this->byte_index + 20)) {
        // Allocate new buffer
        uint8_t *tmp = (uint8_t *)malloc(sizeof(uint8_t) * (this->alloc + ALLOC_SIZE));

        // Clear buffer
        memset(tmp, 0, (this->alloc + ALLOC_SIZE));

        // Copy data to buffer and free buffer
        if (this->buffer != nullptr)
        {
            memcpy(tmp, this->buffer, (this->byte_index + 1));
            free(this->buffer);
        }

        // Increase allocation size
        this->alloc += ALLOC_SIZE;

        // Set buffer
        this->buffer = tmp;

        // Clear pointer to allcoated memory
        tmp = nullptr;
    }
}

/**
 * Add vector of 0 and 1 values into buffer as bits, in REVERSE order
 * @param[in] bits Vector of 0 and 1 values
 * */
void HuffmanCoder::AddBits(const std::vector<uint8_t> &bits) {
    // When vector is empty do nothing
    if (bits.size() == 0) {
        return;
    }

    // Reallocate when nessesary
    this->ReallocateBuffer();

    // Loop through bits and add them in REVERSE order
    for (int64_t i = (bits.size() - 1); i >= 0; i--)
    {
        // When value is 1, set bit
        if (bits[i] == 1)
        {
            this->buffer[this->byte_index] |= (1UL << this->bit_index);
        }

        // Increment bit index
        this->bit_index++;

        // When we see 8 bits, increase byte index
        if (this->bit_index >= BITS_IN_BYTE)
        {
            this->bit_index = 0;
            this->byte_index++;
        }
    }
}

/**
 * Convert byte value into vector of 0 and 1, and call AddBits
 * @param[in] byte Byte valule to be converted into vector and added to buffer
 * */
void HuffmanCoder::AddByte(const uint8_t &byte) {
    // Vector
    std::vector<uint8_t> bits;

    // Convert bits into vector
    for (int8_t i = 0; i < BITS_IN_BYTE; i++)
    {
        // When bit is 1, add 1 otherwise add 0
        if (byte & (1 << i)) {
        bits.push_back(1);
        } else {
        bits.push_back(0);
        }
    }

    // Call AddBits function
    this->AddBits(bits);  
}

/**
 * Initialize huffman tree, with first NYT node
 * */
void HuffmanCoder::InitTree() {
    // Create NYT node
    this->root = this->GenNode();
    this->NYT = this->root;

    // Calculate init index
    this->root->index = (N_VALUES * 2 + 1);
}

/**
 * Allocate memory for Node structure and initialize its values
 * @returns Pointer to newly created Node structure
 * */
Node* HuffmanCoder::GenNode() {
    // Allocate memory for new Node structure
    Node *node = (Node *)malloc(sizeof(Node));

    // Initialize pointers to nullptr
    node->left = nullptr;
    node->right = nullptr;
    node->parent = nullptr;

    // Initialize values to 0
    node->val = 0;
    node->weight = 0;
    node->index = 0;

    // Return pointer to allocated Node structure
    return node;
}

/**
 * Add new NYT node with value node to the tree, after current NYT node
 * @param[in] symbol Value to be added to the tree
 * @returns Return pointer to the old NYT node
 * */
Node* HuffmanCoder::AddSymbol(const uint8_t &symbol) {
    // Create new value node
    this->NYT->right = GenNode();
    this->NYT->right->val = symbol;
    this->NYT->right->index = (this->NYT->index - 1);

    // Add value to search index
    this->leaf_nodes[symbol] = this->NYT->right;

    // Create new NYT node
    this->NYT->left = GenNode();
    this->NYT->left->index = (this->NYT->index - 2);

    // Increment weights
    this->NYT->right->weight++;
    this->NYT->weight++;

    // Set parents
    this->NYT->right->parent = this->NYT;
    this->NYT->left->parent = this->NYT;
    
    // Set new NYT node
    this->NYT = this->NYT->left;

    // Return old NYT
    return this->NYT->parent;
}

/**
 * Return pointer saved in array of leaf node pointers
 * @param[in] symbol Index of required pointer
 * @returns Node pointer, when value exist, nullptr otherwise
 * */
Node* HuffmanCoder::FindSymbol(const uint8_t &symbol) {
    return this->leaf_nodes[symbol];
}

/**
 * Find path from given node to root, and save path as vector of 0's and 1's
 * @param[in] node Node from which our search begins
 * @param[out] path Vector that will contain resulting path of 0's and 1'
 * */
void HuffmanCoder::FindPathToRoot(Node *node, std::vector<uint8_t> &path) {
    // Set starting node
    Node *tmp = node;

    // Keep going up the tree until we reach root node
    while (tmp->parent != nullptr)
    {
        // When our current tmp node is right node of parent, add 1, 0 otherwise
        if (tmp->parent->right == tmp)
        {
            path.push_back(1);
        } else {
            path.push_back(0);
        }

        // Move up the tree
        tmp = tmp->parent;
    }
}

/**
 * Search tree through BFS method, that will firstly add to queue right then left node
 * @param[in] node Node weight and index to be compared against all other nodes
 * @returns First found node or given node when no node is found
 * */
Node* HuffmanCoder::FindHighestBlockNode(Node *node) {
    // Vector of Node pointers
    std::vector<Node*> queue;

    // Index for vector of nodes
    uint64_t i = 0;

    // Insert root and start searching from root
    queue.push_back(this->root);

    // Traverse tree, until we went through all the nodes
    while (i < queue.size()) {
        // Get next node in queue
        Node *tmp = queue[i];

        // Look for the same weight and index that is higher or equal of given node
        if (tmp->index >= node->index && tmp->weight == node->weight) {
            // Found value of the same block, now save when we found better
            return tmp;
        }

        // When right node exist, add it to the queue
        if (tmp->right != nullptr) {
            queue.push_back(tmp->right);
        }

        // When left node exist, add it to the queue
        if (tmp->left != nullptr) {
            queue.push_back(tmp->left);
        }
        
        // Increment queue index
        i++;
    }

    // No value found, return given node, will never happen, only as insurance
    return node;
}

/**
 * Swap position of two nodes with its children
 * @param[in] node1 Node1 that will be swapped with node2
 * @param[in] node2 Node2 that will be swapped with node1
 * */
void HuffmanCoder::SwapNodes(Node *node1, Node *node2) {
    // Save index of node1
    const int32_t tmp_index = node1->index;

    // Save parent pointers
    Node *node1_parent = node1->parent;
    Node *node2_parent = node2->parent;

    // Variables to hold on which side are node1 and node2 from position of their parents
    bool node1_side;
    bool node2_side;

    // Swap indexes
    node1->index = node2->index;
    node2->index = tmp_index;

    // Check original parent of node1, and set node2 for him
    if (node1->parent->left == node1) {
            node1_side = false;
    } else {
        node1_side = true;
    }

    // Check original parent of node2, and set node1 for him
    if (node2->parent->left == node2) {
        node2_side = false;
    } else {
        node2_side = true;
    }

    // Set right node of node1's parent to node2, otherwise set the left node
    if (node1_side) {
        node1_parent->right = node2;
    } else {
        node1_parent->left = node2;
    }

    // Set right node of node2's parent to node1, otherwise set the left node
    if (node2_side) {
        node2_parent->right = node1;
    } else {
        node2_parent->left = node1;
    }
    
    // Swap parents
    node1->parent = node2_parent;
    node2->parent = node1_parent;
}

/**
 * Free all child nodes recursively
 * @param[in] node Node to be freed
 * */
void HuffmanCoder::FreeNode(Node *node) {
    // When given node is not null
    if (node != nullptr)
    {
        // Recursively call for left child
        this->FreeNode(node->left);

        // Recursively call for right child
        this->FreeNode(node->right);

        // Free current node
        free(node);
    }
}

/**
 * Compare if compressed data are lower than RLE, when not copy RLE back to buffer, and add
 * settings that will tell us if data are huffman or RLE
 * */
void HuffmanCoder::CompareWithRLE(uint8_t * &buffer, const size_t &size, uint8_t &settings) {
    // Clear settings byte
    settings = 0;

    // When huffman increased size, copy RLE back into buffer
    if (this->GetSize() > size)
    {
        // Copy buffer back and return RLE
        memcpy(this->buffer, buffer, size);
        this->bit_index = 0;
        this->byte_index = size;
        return;
    }
    
    // Otherwise, calculate padding bits and set setting bit
    settings = ((this->bit_index == 0) ? 0 : (8 - this->bit_index));

    // Set bit
    settings |= SETTINGS_BIT_CHECK;
}

/**
 * Encode RLE data to huffman code
 * @param[in] buffer Buffer containing RLE data
 * @param[in] size Size of buffer in bytes
 * @param[out] settings Byte containing metadata
 * */
void HuffmanCoder::Encode(uint8_t * &buffer, const size_t &size, uint8_t &settings) {
    // Loop through all values of RLE
    for (uint64_t i = 0; i < size; i++) {
        // First appearance of symbol
        Node *node = this->FindSymbol(buffer[i]);
        std::vector<uint8_t> path;

        // First occurance of symbol, add symbol 
        if (node == nullptr) {
            // Add path to NYT to buffer
            this->FindPathToRoot(this->NYT, path);
            this->AddBits(path);

            // Add symbol, returned node is old NYT
            node = this->AddSymbol(buffer[i]);

            // Add symbol to buffer
            this->AddByte(buffer[i]);
        // Symbol already exist
        } else {
            // Add path to symbol to buffer
            this->FindPathToRoot(node, path);
            this->AddBits(path);
        }

        while (true) {
            // Get node of highest index with the same weight, when no is found, we will return node
            Node *highest_node = this->FindHighestBlockNode(node);

            // Swap with highest numbered block
            if (highest_node != node && highest_node != node->parent) {
                this->SwapNodes(highest_node, node);
            }
            
            // Increment weight
            node->weight++;

            // When we reached root node, stop updating tree
            if (this->root == node) {
                break;
            }

            // Move to parent
            node = node->parent;
        }
    }

    // Compare encoded data with RLE, when huffman increased size, use RLE only
    this->CompareWithRLE(buffer, size, settings);
}

/**
 * Return pointer to encoded data buffer
 * @returns Pointer to buffer
 * */
uint8_t * & HuffmanCoder::GetBuffer() {
    return this->buffer;
}

/**
 * Return size of buffer based on bit index
 * */
uint64_t HuffmanCoder::GetSize() {
    if (this->bit_index == 0)
    {
        return this->byte_index;
    }
  
    return (this->byte_index + 1);
}