**Title:** Lempel-Ziv Compression (UCSC CSE13S Assignment by Professor Dunne)

**Description:** This program is to simulate the Lempel-Ziv lossless compression and decompression of data for any file type. 
The program utilizes a Trie ADT for the compression of data by associating a code to a symbol and inserting this pair into the 
Trie as the logic is being some for the reading a buffering of the pair. While the program loops to get every symbol in the 
uncompressed file each code and symbol pair are broken down into bits and inserted into a buffer to hold the bit values of the 
pair. For efficiency of space, we utilize variable bit length for the code when inserting the code bit value into the buffer 
that holds all the bit values as well as starting with the Least Significant Bit (LSB) for each code and symbol. Once a sufficient 
block of bits is inserted into the buffer it is then emptied out into the out-file (compressed file). This process is done 
repeatedly until all the symbols and codes are processed from the in-file (uncompressed file) and converted into bits and outputted 
into the out-file (compressed file). We also use a function to flush the bits from the buffer in the case where the number of bits 
left over doesn’t meet the requirement to empty the buffer in the loop, in this case we use the block size of 4096 bytes. To 
terminate this compression, we use the macro STOP_CODE to signify to the program that this is the end of the compressed data. Prior 
to the bulk of the data going into the compressed file we insert a magic number into the file in order to ensure that the decompression 
algorithm that is used is the same as the compression that was used for it. Prior to all the bulk of the data we also statically 
change the protection/permissions of the compressed file to that of the original file. The decompression program is simply the 
inverse of the compression, where the magic number is verified and the protection/permissions bits are used for the decompressed 
file to be generated. Then the bits are read in starting from the LSB for the code value then the symbols value. From there those 
symbols are placed into a Word ADT that holds an array of word pointer that holds individual words and appended words if there were 
repetitions. As the symbols and codes are being added to the Word ADT there is function inserting these symbols and appended symbols 
into a buffer where the buffer is emptied out once full. There is also a flush function to empty this buffer in the case for remaining 
symbols left in the buffer. This program has the command line options flags -i that sets the in file to be read, -o that sets the out 
file to be written to, and -v to print out the statistics of the compression and decompression of the out file. By default, the in and 
out file will be STDIN and STDOUT respectively in the case one or both of these options aren’t supplied. In the case STDIN and STDOUT 
are the in and out files, io re-direction can be used to echo the in file for STDIN and direction the STDOUT to a specific file.

**Functions:**

trie.c

	TrieNode *trie_node_create(uint16_t index)
		This function allocates memory for a Trie node and sets the code value for it and returns the pointer for the node

	void trie_node_delete(TrieNode *n)
		This function deallocates memory for the single node being passed

	void trie_reset(TrieNode *root)
		This function calls trie_delete for all the children of the root and does not deallocate memory for the root node

	void trie_delete(TrieNode *n)	
		This function uses recursion to deallocate memory for the node being passed as well as all its childrens nodes
		from 0 to 256

	TrieNode *trie_step(TrieNode *n, uint8_t sym)
		This function returns the address for the child of the node being passed at the index sym

word.c

	Word *word_create(uint8_t *syms, uint32_t len)
		This function allocates memory for a word member and the symbol within the word and set the length of the word
		to the length being passed, then returns the address for the word

	Word *word_append_sym(Word *w, uint8_t sym)
		This function allocates memory for a word member and the symbol within the word for length +1 elements and
		sets the length of the new word to the length being passed +1. It then copies the symbol in the word being 
		passed and then appends the new symbol to the end of the new symbol, then sets the new len to the length
		if the old word +1. Returns the new word once done.

	void word_delete(Word *w)
		This funcion deallocated memory for a single word and the symbol within

	WordTable *wt_create(void)
		This function allocated memory for a word table with MAX_CODE elements to hold and array or Word pointers
		then returns the address of the table

	void wt_reset(WordTable *wt)
		This function deletes all the words within the table except for the index 1 within the table by using a loop
		from 2 to MAX_CODE by calling word_delete.

	void wt_delete(WordTable *wt)
		This function loops from 1 to MAX_CODE to delete all the memory allocated in the word table, once done the function
		deallocated memory for the entire wordtable itself. The deallocation for words is dont in word_delete().

bv.c

	BitVector *bv_create(uint32_t bit_len)
		Function creates a new Bit Vector member by allocating memory on the heap for the structure it then creates another 
		space on the heap for its vector array and uses the bit_len integer to identify how many indexes is needed in order 
		to have enough bits for every integer up until bit_len. Once all the member created and the variables and arrays are 
		initialized the function returns the address of the member.

	void bv_delete(BitVector *v)
		This function simply takes the Bit Vector pointer and frees the allocated memory for the vector array then frees the 
		memory for the entire Bit Vector member.

	uint32_t bv_get_len(BitVector *v)
		Function returns the value of the length variable in the Bit Vector member being passed.

	void bv_set_bit(BitVector *v, uint32_t i)
		In order to turn bits on or off function life bv_set_bit need to be implemented. This function takes the Bit Vector 
		and integer value as an argument to find the bit associated with the integer being passed and its bit value to 1. This 
		function also have a condition to check if the bit associated with the integer is already set on in order to prevent 
		the bit from being set off since this function on flips the current bit value.

	void bv_clr_bit(BitVector *v, uint32_t i)
		This functions does something similar to the bv_set_bit() function but instead of turning the bit on it turns the bit off. 
		A condition is always implemented to check if the bit associated with the integer value being passed is already set to off 
		in order to prevent the bit from being flipped multiple times.

	uint8_t bv_get_bit(BitVector *v, uint32_t i)
		This function is used to check whether a number being check if prime or composite. This function returns a 0 or 1 by checking 
		whether the bit associated to the integer i is on or off.

	void bv_set_all_bits(BitVector *v)
		This function turns all the bits inside the Bit Vector on by going through every index allocated and setting each index equal 
		to 0xFF which is equivalent to 255 or 1111 1111 in an 8 bit integer.
	
io.c

	int read_bytes(int infile, uint8_t *buf, int to_read)	
		This function is a wrapper for the read system call and loops calling the read() until the amount specified in to_read is met
		or until there is nothing else to read.

	int write_bytes(int outfile, uint8_t *buf, int to_write)
		This function is a wrapper for the write system call and loops calling the write() until the amount specified in the to_write
		is met or until there is nothing else to write.

	void read_header(int infile, FileHeader *header)
		Calls the read_byte() passing in the header pointer and casting it to a uint8_t and the size being the sizeof(FileHeader)

	void write_header(int outfile, FileHeader *header)
		Calls the write_byte() passing in the header pointer and casting it to a uint8_t and the size being the sizeof(FileHeader)

	bool read_sym(int infile, uint8_t *byte)
		This function goes byte by byte within the global static byte buffer and assigns it to the byte variable passed, once all the
		bytes within the buffer are read, it then read another block by calling read_bytes().
	
	void buffer_pair(int outfile, uint16_t code, uint8_t sym, uint8_t bit_len)
		This function turns the code and symbol into its bit value starting from the LSB and uses a variable bit length for the code
		for optimization of storage. Once the buffer that holds the bits hits the Block size, the buffer is written out and over written
		until all the codes and symbols are translated.
	
	void flush_pairs(int outfile
		This function writes out any remainder bits that may be left over in the buffer and writes it out to the outfile.
	
	bool read_pair(int infile, uint16_t *code, uint8_t *sym, uint8_t bit_len)
		This function reads in the code and assign it to the code variable passed and does the same for the symbols within the bit buffer
		Once all the bits within the bit buffer are read a new block of bits are read in by the read_bytes() until all the bits are read from
		the infile.
	
	void buffer_word(int outfile, Word *w)
		This function takes the symbols within the words symbols array and writes it out into the byte buffer, once a block is written into
		the buffer the buffer is emptied out to the outfile and overwrites the old data until all the symbols are processed.

	void flush_words(int outfile)
		This function flushes out any remainder bytes left over in the byte buffer to the outfile.	

encode.c

	uint8_t bit_length(uint16_t next_code)
		This function calculates the minimum number of bits needed fort the code value being passed.	

	void get_options(int argc, char **argv, char **read_file, char **write_file)
		This function parses through the command line arguments and sets flags for the program based on what the user enters.

	int main(int argc, char **argv)	
		The main program controls all function calls and opens and closes files. This function gets set the magic number for a header file and the 
		permission bits and copies those over to the outfile. The main function is what implements the Trie ADT and the Lempzel compression algorithm 
		for the compressed outfile

decode.c

	uint8_t bit_length(uint16_t next_code)
                This function calculates the minimum number of bits needed fort the code value being passed.

	void get_options(int argc, char **argv, char **read_file, char **write_file)
                This function parses through the command line arguments and sets flags for the program based on what the user enters.

	int main(int argc, char **argv)
		The main program controls all function calls and opens and closes files. This function gets set the magic number from the infile by read_header and 
		sets the permission bits and copies those over to the outfile. The main function is what implements the Word ADT and the Lempzel de-compression algorithm
                for the outfile.

**Makefile:**


	Commands:

	Make or Make all
		In order to compile this program the user must call make or make all with the Makefile within the same directory as the rest if the program. 
		The make/make all command will compile and generate object files trie.o, word.o, bv.o, io.o, encode.o, and decode.o from their
		associated C files. The make command will then link all the object files and genrate an executable file called encode and anothe called decode.

	Make encode
		In order to compile this program to encode a file the user must call make encode with the Makefile within the same directory as the rest of the program.
		The make encode command will compile and generate object files trie.o, bv.o, io.o, and encode.o from their associated C files. The make command will then
		link all the object files and generate an executable file called encode.

	Make decode
		In order to compile this program to decode a file the user must call make decode with the Makefile within the same directory as the rest of the program.
                The make decode command will compile and generate object files word.o, bv.o, io.o, and decode.o from their associated C files. The make command will then
                link all the object files and generate an executable file called decode.

	Make clean
		To quickly remove the object files by make the user can enter 'make clean' to remove all object files executables that were previously made.

	Make infer
		This command analyzes the code in the program for potential logical errors

**Example:i**

		-bash-4.2$ ./encode -i fox.decode -o fox2.encode
		
		-bash-4.2$ ./decode -i fox.encode -o fox2.decode
		
		-bash-4.2$ ./encode -v -i binary.decode -o binary2.encode
		Compressed file size: 24870 bytes
		Uncompressed file size: 20000 bytes
		Compressed ratio: -24.35%
		
		-bash-4.2$ ./decode -v -i binary.encode -o binary2.decode
		Compressed file size: 24870 bytes
		Uncompressed file size: 20000 bytes
		Compressed ratio: -24.35%
