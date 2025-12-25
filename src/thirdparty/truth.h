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

/**
 * Get the last error message.
 * Returns a null-terminated string, or NULL if no error.
 * The string is owned by the library and should not be freed.
 */
const char* truth_get_error(void);

/**
 * Compile an ECL script.
 * @param game_str The game identifier, e.g. "th10"
 * @param input_path Path to the input script file
 * @param output_path Path to the output ECL file
 * @param mapfile_path Path to the mapfile
 * @return 0 on success, 1 on error (check truth_get_error())
 */
int truth_compile_ecl(const char* game_str, const char* input_path, const char* output_path, const char *mapfile_path);

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
