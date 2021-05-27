#ifndef CODER_H
#define CODER_H

#include <stdint.h>

#include "adaptive_tree.h"


struct coder_vtbl;

typedef struct coder
{
    adaptive_tree tree;
    void(*writer)(const uint8_t* bytes, uint32_t count, void* params);
    void* writer_params;
    struct
    {
        void(*deinit)(struct coder*, bool);
        void(*write)(struct coder*, const uint8_t*, uint32_t);
        void(*flush)(struct coder*);
    } vtbl;
}
coder;

#endif
