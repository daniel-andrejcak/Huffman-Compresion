# Huffman-Compresion

Program to compress and decompress files using optimal Huffman compresion

## Supported funcitons as CLI arguments
`--print` - Prints huffman codes for all characters (all unique characters) to stdout

`--compress` - Compresses the file. The format will be:
```
<file> = <header><body>

<header> = <u64:size of body in bits><codes>

<codes> = <code>[256] -- all extended ASCII characters

<code> = <u8:size in bits N><u8:actual code>[] -- code can span multiple bytes

<body> = <code of first byte><code of second byte>...<code of last byte>
```
`--decompress` - Decompresses the file

*For example:*

```Bash
./huffman --print input_file.txt
./huffman --compress input_file.pdf encoded_file 
./huffman --decompress encoded_file output_file.pdf
```
