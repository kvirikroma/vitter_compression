#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "include/encoder.h"
#include "include/decoder.h"

#define READ_BYTES_PER_ITERATION 1024


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
bool read(uint32_t bytes_count, void* coder, working_mode mode)
{
    uint8_t bytes[bytes_count];
    uint32_t read_bytes = fread(bytes, sizeof(uint8_t), bytes_count, stdin);
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
    bool compression_found = false;
    bool decompression_found = false;
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "--compress") == 0)
        {
            compression_found = true;
            continue;
        }
        if (strcmp(argv[i], "--decompress") == 0)
        {
            decompression_found = true;
            continue;
        }
    }
    if (compression_found && decompression_found)
    {
        fprintf(stderr, "%s\n", "Contradictory flags found. Exiting");
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
            is_eof = read(READ_BYTES_PER_ITERATION, &enc, mode);
        }
        // encoder_flush(&enc);
        // encoder_delete(&enc, false);
    }
    else
    {
        decoder dec;
        decoder_init(&dec, (void(*)(const uint8_t*, uint32_t, void*))write, stdout);
        bool is_eof = false;
        while(!is_eof)
        {
            is_eof = read(READ_BYTES_PER_ITERATION, &dec, mode);
        }
        // decoder_flush(&dec);
        // decoder_delete(&dec, false);
    }
    //fflush(stdout);

    return 0;
}
