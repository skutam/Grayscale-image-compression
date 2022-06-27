# Grayscale image compression

Application for lossless compression of 8-bit grayscale images in raw format, using adaptive Huffman coding with Running Length Encryption.

## Build

To build the program simply type `make`, the only requiremenet is `g++`.

## Usage

To compress use

```bash
$ ./huff_codec -c -w 512 -i image.raw -o image.comp
```

to decompress use

```bash
$ ./huff_codec -d -i image.comp -o image_out.raw
```