#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>

#include "include/encoder.h"
#include "include/decoder.h"
#include "include/utils.h"

#define READ_BYTES_PER_ITERATION 204800


typedef enum
{
    MODE_ENCODING,
    MODE_DECODING
}
working_mode;


coder* coder_in_use;


void stop(int sig)
{
    switch (sig)
    {
        case SIGINT:
        case SIGTERM:
        case SIGQUIT:
        {
            if (coder_in_use)
            {
                coder_in_use->vtbl.deinit(coder_in_use, false);
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
bool read(uint32_t bytes_count, coder* coder, working_mode mode, FILE* file)
{
    uint8_t bytes[bytes_count];
    uint32_t read_bytes = fread(bytes, sizeof(uint8_t), bytes_count, file);
    coder->vtbl.write(coder, bytes, read_bytes);
    if (read_bytes != bytes_count)
    {
        coder->vtbl.flush(coder);
    }
    return read_bytes != bytes_count;
}


int main(int argc, char** argv)
{
    coder_in_use = NULL;
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

    coder_in_use = malloc(max(2, sizeof(encoder), sizeof(decoder)));
    if (mode == MODE_ENCODING)
    {
        encoder_init((encoder*)coder_in_use, (void(*)(const uint8_t*, uint32_t, void*))write, stdout);
    }
    else
    {
        decoder_init((decoder*)coder_in_use, (void(*)(const uint8_t*, uint32_t, void*))write, stdout);
    }

    bool is_eof = false;
    while(!is_eof)
    {
        is_eof = read(READ_BYTES_PER_ITERATION, coder_in_use, mode, file);
    }
    coder_in_use->vtbl.flush(coder_in_use);
    if (mode == MODE_DECODING)
    {
        ((decoder*)coder_in_use)->vtbl.flush_final((decoder*)coder_in_use);
    }
    coder_in_use->vtbl.deinit(coder_in_use, false);
    free(coder_in_use);
    coder_in_use = NULL;
    fflush(stdout);

    return 0;
}
