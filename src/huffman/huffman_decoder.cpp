/**
 * Author: Matúš Škuta (xskuta04)
 * Date created: 07.05.2021
 * Name: huffman_decoder.cpp
 * Description: Contains implementations of class HuffmanDecoder, that is used to decode
 * hufman code into binary data 
 * */
#include "huffman_decoder.hpp"

/**
 * Constructor that will initialize values, and huffman tree
 * */
HuffmanDecoder::HuffmanDecoder() {
    // Initialize read indexes
    this->read_byte_index = 0;
    this->read_bit_index = 0;

    // Initialize write index
    this->write_byte_index = 0;
    this->alloc = 0;
    this->buffer = nullptr;

    // Initialize starting node of the tree
    this->InitTree();
}

/**
 * Deconstructor that will free allocated values
 * */
HuffmanDecoder::~HuffmanDecoder() {
    // When buffer was allocated, free him
    if (this->buffer)
    {
        free(this->buffer);
    }

    // When tree was allocated, free tree recursively
    if (this->root)
    {
        this->FreeNode(this->root);
    }
}

/**
 * When about 20 bytes are remaining of buffer, increase buffer
 * */
void HuffmanDecoder::ReallocateBuffer() {
    // When about 20 bytes are left, expand buffer
    if (this->alloc <= (this->write_byte_index + 20)) {
        // Allocate new buffer
        uint8_t *tmp = (uint8_t *)malloc(sizeof(uint8_t) * (this->alloc + ALLOC_SIZE));

        // Clear buffer
        memset(tmp, 0, (this->alloc + ALLOC_SIZE));

        // When buffer was allocated, copy data to tmp buffer and free buffer
        if (this->buffer != nullptr)
        {
            // Copy data to buffer
            memcpy(tmp, this->buffer, this->write_byte_index);
            free(this->buffer);
        }

        // Increase allocation size
        this->alloc += ALLOC_SIZE;

        // Set buffer
        this->buffer = tmp;

        // Clear pointer to allocated memory
        tmp = nullptr;
    }
}

/**
 * Add symbol to buffer
 * @param[in] symbol Symbol to be added to buffer
 * */
void HuffmanDecoder::AddSymbolToBuffer(const uint8_t &symbol) {
    // Reallocate if nessesary
    this->ReallocateBuffer();
    // Add symbol to buffer and increment index
    this->buffer[this->write_byte_index] = symbol;
    this->write_byte_index++;
}

/**
 * Read 8bits from buffer and convert them to byte
 * @param[in] buffer Buffer containing encoded data
 * @param[in] size Size of buffer in bytes
 * @param[out] symbol Here we will set 8bits that we will read
 * @returns True when there are still data, false when we reached end of buffer
 * */
bool HuffmanDecoder::ReadSymbol(uint8_t * &buffer, const uint64_t &size, uint8_t &symbol) {
    // End of buffer bool
    bool end_of_buffer = false;

    // Clear symbol
    symbol = 0;

    // Read 8 bits
    for (size_t i = 0; i < BITS_IN_BYTE; i++)
    {
        // When bit is 1, set bit on given index in symbol
        if (this->NextBit(buffer, size, end_of_buffer))
        {
            symbol |= (1UL << (7 - i));
        }

        // Did we reach end of buffer ?, End
        if (end_of_buffer)
        {
            return false;
        }
    }

    // Successfully read 8bits into symbol, return true
    return true;
}

/**
 * Get next bit in buffer as boolean value and return it
 * @param[in] buffer Buffer containing encoded data
 * @param[in] size Size of buffer in bytes
 * @param[out] end_of_buffer When we reach end of buffer, set to true
 * @returns Next bit in buffer as boolean value
 * */
bool HuffmanDecoder::NextBit(uint8_t * &buffer, const uint64_t &size, bool &end_of_buffer) {
    // Bit as boolean value
    bool res;

    // Check if we reached end of buffer
    if (this->read_byte_index == size)
    {
        // Set bool end_of_buffer to true, adn return anything
        end_of_buffer = true;
        return false;    
    }

    // When next value is 1, set to true, otherwise set to false
    if (buffer[this->read_byte_index] & (1 << (this->read_bit_index))) {
        res = true;
    } else {
        res = false;
    }

    // Increment bit index
    this->read_bit_index++;

    // When we see 8 bits, increase byte index
    if (this->read_bit_index >= BITS_IN_BYTE) {
        this->read_bit_index = 0;
        this->read_byte_index++;
    }

    // Return result
    return res;
}

/**
 * Check if we reached padding bits
 * @returns True when we reached padding bits, false otherwise
 * */
bool HuffmanDecoder::IsEnd(const uint64_t &size, const uint8_t &padding_bits) {
    return ((this->read_bit_index + padding_bits) == 9 && (size == (this->read_byte_index + 1)));
}

/**
 * Initialize huffman tree, with first NYT node
 * */
void HuffmanDecoder::InitTree() {
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
Node* HuffmanDecoder::GenNode() {
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
Node* HuffmanDecoder::AddSymbol(const uint8_t & symbol) {
    // Add symbol to buffer
    this->AddSymbolToBuffer(symbol);

    // Create new value node
    this->NYT->right = GenNode();
    this->NYT->right->val = symbol;
    this->NYT->right->index = (this->NYT->index - 1);

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
 * Search tree through BFS method, that will firstly add to queue right then left node
 * @param[in] node Node weight and index to be compared against all other nodes
 * @returns First found node or given node when no node is found
 * */
Node* HuffmanDecoder::FindHighestBlockNode(Node *node) {
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

        // When left node value exist, add it to the queue
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
 * Check whetever given node is external or note
 * @param[in] node Node that may be external
 * @returns True when node is external, false otherwise
 * */
bool HuffmanDecoder::IsExternalNode(Node *node) {
    return node != nullptr && node->left == nullptr && node->right == nullptr;
}

/**
 * Swap position of two nodes with its children
 * @param[in] node1 Node1 that will be swapped with node2
 * @param[in] node2 Node2 that will be swapped with node1
 * */
void HuffmanDecoder::SwapNodes(Node *node1, Node *node2) {
    // Save index of node1
    const uint16_t tmp_index = node1->index;

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
void HuffmanDecoder::FreeNode(Node *node) {
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
 * Decode huffman encoded data
 * @param[in] buffer Buffer containing huffman encoded values
 * @param[in] size Size of data in buffer
 * @returns True when decode was successfull, false otherwise
 * */
bool HuffmanDecoder::Decode(uint8_t * & buffer, const uint64_t &size) {
    // Start node pointer at root
    Node *node = this->root;
    
    // Variable that will hold symbol
    uint8_t symbol;

    // Ending bool
    bool end_of_buffer = false;

    // Number of padding bits in data
    uint8_t padding_bits;

    // First value of buffer are settings
    // first 3 bits represent number of padding bits
    // When 4th bit is set, we are using huffman, otherwise we are copying buffer to output
    if (size > 1 && (buffer[0] & SETTINGS_BIT_CHECK)) {
        padding_bits = (buffer[0] & PADDING_BITS_MASK);
        this->read_byte_index++;
    // Data are encoded using only RLE, copy buffer
    } else {
        // Allocate buffer
        this->buffer = (uint8_t *)malloc(sizeof(uint8_t) * (size - 1));

        // Copy data to buffer
        memcpy(this->buffer, &buffer[1], (size - 1));

        // Set resulting size
        this->write_byte_index = (size - 1);
        return true;
    }
    
    // Loop until the end of given buffer
    while (!end_of_buffer)
    {
        // If we reached padding bits, quit
        if (this->IsEnd(size, padding_bits))
        {
            return true;
        }

        // Node is not external, read another bit and move down the tree
        if (!this->IsExternalNode(node))
        {
            // Get next bit as bool value
            bool move_right = this->NextBit(buffer, size, end_of_buffer);

            // Reached end of buffer, ending
            if (end_of_buffer)
            {
                return true;
            }

            // Move right
            if (move_right) {
                // Ending invalid data on input
                if (node->right == nullptr) {
                    std::cerr << "INVALID DATA ON INPUT" << std::endl;
                    return false;
                }

                // Move to the right node
                node = node->right;
            // Move left
            } else {
                // Ending invalid data on input
                if (node->left == nullptr) {
                    std::cerr << "INVALID DATA ON INPUT" << std::endl;
                    return false;
                }

                // Move to the left node
                node = node->left;
            }
            
            continue;
        }

        // We reached external node and it is NYT
        if (node == this->NYT) {
            // Try to read 8bits, from data
            if (!this->ReadSymbol(buffer, size, symbol))
            {
                std::cerr << "There needs to be 8 bit value after NYT node" << std::endl;
                return false;
            }

            // Add symbol to buffer and to tree
            node = this->AddSymbol(symbol);
        // Not NYT, Add value from node to buffer
        } else {
            this->AddSymbolToBuffer(node->val);
        }

        // Update tree
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

    return true;
}

/**
 * Return pointer to compressed data buffer
 * @returns Pointer to buffer
 * */
uint8_t * & HuffmanDecoder::GetBuffer() {
    return this->buffer;
}

/**
 * Return compressed data buffer size
 * @returns Size of buffer
 * */
uint64_t HuffmanDecoder::GetSize() {
    return this->write_byte_index;
}