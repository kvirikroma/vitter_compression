#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


void* check_pointer_after_malloc(void* ptr)
{
    if (!ptr)
    {
        fprintf(stderr, "%s", "An error occurred while allocating memory! Exiting\n");
        exit(1);
    }
    return ptr;
}

int64_t max(uint8_t items_count, ...)
{
    va_list args;
    va_start(args, items_count);
    int64_t result = INT64_MIN;
    for (uint8_t item = 0; item < items_count; item++)
    {
        int64_t current_arg = va_arg(args, int64_t);
        if (result < current_arg)
        {
            result = current_arg;
        }
    }
    va_end(args);
    return result;
}
