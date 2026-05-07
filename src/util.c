#include "util.h"

String read_entire_file(const char *filepath)
{
    FILE *f = fopen(filepath, "rb");
    if (!f)
        return ERROR_STRING;

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return ERROR_STRING;
    }

    long size = ftell(f);
    if (size < 0) {
        fclose(f);
        return ERROR_STRING;
    }

    rewind(f);

    char *buffer = malloc((size_t)size + 1);
    if (!buffer) {
        fclose(f);
        return ERROR_STRING;
    }

    size_t read_size = fread(buffer, 1, (size_t)size, f);
    fclose(f);

    if (read_size != (size_t)size) {
        free(buffer);
        return ERROR_STRING;
    }

    buffer[size] = '\0';
    return STRING(buffer, size);
}
