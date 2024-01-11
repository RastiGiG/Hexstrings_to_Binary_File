/**************************************************************************
 * HEXSTRINGS TO BINARY FILE V1.0
 * ------------------------------------------------------------------------
 * Copyright (c) 2023-2024 RastiGiG <randomly.ventilates@simplelogin.co>
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would
 *    be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 ***********************************************************************/

/* Libraries
 *
 * */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/* Macros
 *
 * */
#define MAX_BUFF_SIZE                1024 * 4 
#define NUMBER_OF_CHARS_PER_DIGITS   4 
#define BINARY_WRITE                "wb"


/* Type declarations
 *
 * usize            -- Rust inspired name for size_t type
 * u8               -- Rust inspired shorthand for uint8_t
 * u16              -- Rust inspired shorthand for uint16_t
 * u32              -- Rust inspired shorthand for uint16_t
 * color_channels   -- Object to separately store rgb color channels
 * */
typedef size_t usize;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;


/* Convert hexadecimal chars into integers
 *
 * */
u8 hex_to_int(char hex_digit) {
    u8 decimal;
    if (hex_digit >= '0' && hex_digit <= '9') {
        decimal = hex_digit - '0';
    } else if (hex_digit >= 'a' && hex_digit <= 'f') {
        decimal = hex_digit - 'a' + 10;
    } else if (hex_digit >= 'A' && hex_digit <= 'F') {
        decimal = hex_digit - 'A' + 10;
    } else {
        fprintf(stderr, "[ERROR]: Invalid hexadecimal digit: %c\n", hex_digit);
        //exit(1);
        return 0;
    }

    return decimal;
}

/* Concatenate to digits 
 *
 * 0x0F 0x0F => 0xFF
 * Input file is interpreted in text format, so each digits would be stored
 * as single byte (sizeof(char)).
 *
*/
u16 concat_digits(char *hex_digits) {
    u16 output_number = 0;
    for (int i = 0; i < NUMBER_OF_CHARS_PER_DIGITS; i++) {
        if ((hex_digits[i] >= 'A' && hex_digits[i] <= 'F') 
            || (hex_digits[i] >= 'a' && hex_digits[i] <= 'f') 
            || (hex_digits[i] > '0' && hex_digits[i] <= '9')) {
            output_number |= hex_to_int(hex_digits[i]) << (12 - i*4);
        } 
    }
    return output_number;
}


/* Handle file opening and writing
 * 
 * */
usize file_open_and_write (char *filepath, void * buffer, usize size){
    // Check file exists
    if (access(filepath, F_OK) == 0){
        fprintf(stderr, "[ERROR]: file '%s' already exists! Delete/move it or choose a different filename.", filepath);
        exit(1);
    }

    // Open file and check writability, 'wb' -> 'write binary'
    FILE *file = fopen(filepath, BINARY_WRITE);
    if(file == NULL){
        fprintf(stderr, "[ERROR]: File '%s' cannot be opened! Do you have write permissions?", filepath);
        exit(1);
    }

    // cast input to 8 bit integer to process data 1 byte at a time
    u8 *data = buffer;
    usize i;

    // Write file to memory
    for (i = 0; i < size; i++) {
        fwrite(&data[i], 1, 1, file);   //sizeof(u8),
    }

    // Check size of written file
    usize file_size;
    fseek(file, 0L, SEEK_END);
    file_size = ftell(file);

    fclose(file);
    return file_size;
}

/* Handle file opening and reading
 * 
 * */
usize file_open_and_read (char *filepath, char* buffer, usize size){
    // Check file exists
    if (access(filepath, F_OK) != 0){
        fprintf(stderr, "[ERROR]: File '%s' cannot be accessed! Does it exist?", filepath);
        exit(1);
    }

    // Open file and check readability
    FILE *file = fopen(filepath, "r");
    if(file == NULL){
        fprintf(stderr, "[ERROR]: File '%s' cannot be opened! Do you have read permissions?", filepath);
        exit(1);
    }

    // Check file size
    usize file_size;
    fseek(file, 0L, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    if(file_size <= 0){
        fprintf(stderr, "[ERROR]: File has '%zu' <= 0 bytes!\n", file_size);
        exit(1);
    }
    if(file_size > size){
        fprintf(stderr, "[ERROR]: File size (%zu) is too big. Max size allowed is %zu!\n", file_size, size);
        exit(1);
    }

    // Read file to memory
    fread(buffer, sizeof(char), file_size, file);
    fclose(file);

    return file_size;
}

/* Main Function
 *
 * */
int main(int argc, char *argv[])
{
    if (argc < 3){
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        exit(0);
    }

    char *read_filepath = argv[1];
    char *write_filepath = argv[2];
    char* input_buffer = (char*) calloc(MAX_BUFF_SIZE, sizeof(char));
    u16* output_buffer = (u16*) calloc(MAX_BUFF_SIZE, sizeof(u8));

    // Read the input file
    usize file_size = file_open_and_read(read_filepath, input_buffer, MAX_BUFF_SIZE);

    char data[NUMBER_OF_CHARS_PER_DIGITS] = {0};
    /*
     * i -- counter for bytes in input file
     * j -- counter u16 numbers in output file
     * k -- counter for valid base 16 digits in input file
     * l -- counter for hex digits of current set of 4
     * */
    usize i, j, k, l;
    usize number_of_hex_prefixes = 0;
    j = 0;
    k = 0;
    l = 0;
    //convert the input buffer to binary
    for (i = 0; i < file_size; i++) {
        // Reset counter when prefix '0x' is present
        if ((input_buffer[i] == 'x' || input_buffer[i] == 'X') && input_buffer[i-1] == '0') {
            k--;
            l--;
            number_of_hex_prefixes +=1;
        } else {
            data[k%4] = input_buffer[i];
            k++;
            l++;
        }
        // Write to output buffer once 4 digits have been collected
        if (k%4 == 0 && l != 0) {
            output_buffer[j] = concat_digits(data);
            j++;
            l = 0;
        }
    }
    // get actual number bytes by removing prefixes from count
    file_size -= number_of_hex_prefixes;  

    // Write the input file to binary, half file size because of concatenation
    file_size = file_open_and_write(write_filepath, output_buffer, file_size/2);

    printf("Sucessfully wrote file '%s' of size '%zu'\n", write_filepath, file_size);
    
    free(input_buffer);
    //free(output_buffer);
    input_buffer = NULL;
    //output_buffer = NULL;
    return EXIT_SUCCESS;
}
