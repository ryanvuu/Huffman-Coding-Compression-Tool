# Huffman-Coding-Compression-Tool
# About

This project contains programs used for compressing and decompressing text files utilizing Huffman Trees.

## hencode
This program uses the Huffman coding algorithm to compress a text file. Text files are compressed by building a Huffman tree based on frequencies of characters and extracting the 
new bit codes into the compressed file.
### Usage
    hencode infile [ outfile ]
  If outfile is not specified, output will go to standard output.

## hdecode
This program reverses the compression of a file that was compressed using Huffman encoding. Reversal is done by regenerating the original Huffman tree. Simultaneous traversal of the tree and writing of the original characters occurs.
### Usage
    hdecode infile [ outfile ]
  If outfile is not specified, output will go to standard output.
