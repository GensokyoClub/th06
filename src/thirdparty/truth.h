#ifndef TRUTH_H
#define TRUTH_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int arguments;
    int intrinsics;
    int blocks;
    int diff_switches;
} DecompileOptionsC;

typedef struct {
    unsigned char* data;
    size_t len;
    size_t capacity;
} TruthBuffer;

/**
 * Get the last error message.
 * Returns a null-terminated string, or NULL if no error.
 * The string is owned by the library and should not be freed.
 */
const char* truth_get_error(void);

/**
 * Release memory owned by a TruthBuffer previously returned by this API.
 * Passing a buffer with NULL data is a no-op.
 */
void truth_buffer_free(TruthBuffer buffer);

/**
 * Compile an ANM file into an in-memory buffer.
 * @param game_str The game identifier, e.g. "th10"
 * @param input_path Path to the input ANM file
 * @param mapfile_path Path to the mapfile
 * @return A TruthBuffer containing the compiled data, or an empty buffer on error (check truth_get_error()). Free with truth_buffer_free().
 */
TruthBuffer truth_compile_anm(const char* game_str, const char* input_path, const char *mapfile_path);

/**
 * Compile an ECL script into an in-memory buffer.
 * @param game_str The game identifier, e.g. "th10"
 * @param input_path Path to the input script file
 * @param mapfile_path Path to the mapfile
 * @return A TruthBuffer containing the compiled data, or an empty buffer on error (check truth_get_error()). Free with truth_buffer_free().
 */
TruthBuffer truth_compile_ecl(const char* game_str, const char* input_path, const char *mapfile_path);

/**
 * Decompile an ECL file.
 * @param game_str The game identifier, e.g. "th10"
 * @param input_path Path to the input ECL file
 * @param output_path Path to the output script file, or NULL for stdout
 * @param mapfile_path Path to the mapfile
 * @param options Decompilation options
 * @return 0 on success, 1 on error (check truth_get_error())
 */
int truth_decompile_ecl(const char* game_str, const char* input_path, const char* output_path, const char *mapfile_path, DecompileOptionsC options);

#ifdef __cplusplus
}
#endif

#endif /* TRUTH_H */
