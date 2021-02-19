#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "include/encoder.h"
#include "include/decoder.h"

#define READ_BYTES_PER_ITERATION 20480


typedef enum
{
    MODE_ENCODING,
    MODE_DECODING
}
working_mode;


void write(const uint8_t* bytes, uint32_t count, FILE* output_file)
{
    fwrite(bytes, sizeof(uint8_t), count, output_file);
}

// Returns is_eof
bool read(uint32_t bytes_count, void* coder, working_mode mode, FILE* file)
{
    uint8_t bytes[bytes_count];
    uint32_t read_bytes = fread(bytes, sizeof(uint8_t), bytes_count, file);
    if (mode == MODE_DECODING)
    {
        decoder_write((decoder*)coder, bytes, read_bytes);
        if (read_bytes != bytes_count)
        {
            decoder_flush((decoder*)coder);
        }
    }
    else
    {
        encoder_write_bytes((encoder*)coder, bytes, read_bytes);
        if (read_bytes != bytes_count)
        {
            encoder_flush((encoder*)coder);
        }
    }
    return read_bytes != bytes_count;
    
}


int main(int argc, char** argv)
{
    FILE* file = stdin;
    bool compression_found = false;
    bool decompression_found = false;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--compress") == 0)
        {
            compression_found = true;
            continue;
        }
        else if (strcmp(argv[i], "--decompress") == 0)
        {
            decompression_found = true;
            continue;
        }
        else
        {
            fprintf(stderr, "Unknown flag \"%s\"\n", argv[i]);
            exit(1);
        }
    }
    if (compression_found && decompression_found)
    {
        fprintf(stderr, "Contradictory flags found. Exiting\n");
        exit(1);
    }
    working_mode mode = MODE_ENCODING;
    if (decompression_found)
    {
        mode = MODE_DECODING;
    }

    if (mode == MODE_ENCODING)
    {
        encoder enc;
        encoder_init(&enc, (void(*)(const uint8_t*, uint32_t, void*))write, stdout);
        bool is_eof = false;
        while(!is_eof)
        {
            is_eof = read(READ_BYTES_PER_ITERATION, &enc, mode, file);
        }
        encoder_delete(&enc, false);
    }
    else
    {
        decoder dec;
        decoder_init(&dec, (void(*)(const uint8_t*, uint32_t, void*))write, stdout);
        bool is_eof = false;
        while(!is_eof)
        {
            is_eof = read(READ_BYTES_PER_ITERATION, &dec, mode, file);
            fflush(stdout);
        }
        decoder_delete(&dec, false);
    }
    fflush(stdout);

    return 0;
}
