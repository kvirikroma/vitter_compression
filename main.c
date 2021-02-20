#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>

#include "include/encoder.h"
#include "include/decoder.h"

#define READ_BYTES_PER_ITERATION 204800


typedef enum
{
    MODE_ENCODING,
    MODE_DECODING
}
working_mode;


working_mode* global_mode_ptr;
void* global_coder_ptr;


void stop(int sig)
{
    switch (sig)
    {
        case SIGINT:
        case SIGTERM:
        case SIGQUIT:
        {
            if (global_coder_ptr && global_mode_ptr)
            {
                switch (*global_mode_ptr)
                {
                    case MODE_DECODING:
                    {
                        decoder_delete((decoder*)global_coder_ptr, false);
                        global_coder_ptr = NULL;
                        break;
                    }
                    case MODE_ENCODING:
                    {
                        encoder_delete((encoder*)global_coder_ptr, false);
                        global_coder_ptr = NULL;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
            FILE* terminal = fopen("/dev/tty", "w");
            fprintf(terminal, "\n");
            fclose(terminal);
            exit(0);
            break;
        }
        
        default:
        {
            break;
        }
    }
}


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
    global_mode_ptr = 0;
    global_coder_ptr = 0;
    signal(SIGTERM, stop);
    signal(SIGINT, stop);
    signal(SIGQUIT, stop);
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
    global_mode_ptr = &mode;

    if (mode == MODE_ENCODING)
    {
        encoder enc;
        global_coder_ptr = &enc;
        encoder_init(&enc, (void(*)(const uint8_t*, uint32_t, void*))write, stdout);
        bool is_eof = false;
        while(!is_eof)
        {
            is_eof = read(READ_BYTES_PER_ITERATION, &enc, mode, file);
        }
        encoder_delete(&enc, false);
        global_coder_ptr = NULL;
    }
    else
    {
        decoder dec;
        global_coder_ptr = &dec;
        decoder_init(&dec, (void(*)(const uint8_t*, uint32_t, void*))write, stdout);
        bool is_eof = false;
        while(!is_eof)
        {
            is_eof = read(READ_BYTES_PER_ITERATION, &dec, mode, file);
        }
        decoder_delete(&dec, false);
        global_coder_ptr = NULL;
    }
    fflush(stdout);

    return 0;
}
