/**
 * Usage: abcout <file> [no. cells printed] [no. cells printed in debug]
 */

#define __STDC_WANT_LIB_EXT1__ 1

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t const HALT_CONDITION[] = {0x00, 0x00, 0x00, 0x00, 0x7F, 0xFF};

int run(FILE *file, uint16_t result_limit, uint16_t debug_limit) {
  uint8_t cells[UINT16_MAX] = {0, 1};
  uint8_t instr[6];

  while (true) {
    size_t const read_bytes = fread(instr, sizeof(*instr), 6, file);
    if (read_bytes == 0) {
      break;
    }

    if (read_bytes != 6) {
      fprintf(stderr, "Misaligned ROM\n");
      return 1;
    }

    if (memcmp(HALT_CONDITION, instr, 6) == 0) {
      if (debug_limit) {
        printf("\n\tHALT\n");
      }
      break;
    }

    uint16_t const a = instr[0] * 256 + instr[1];
    uint16_t const b = instr[2] * 256 + instr[3];
    uint16_t const c = instr[4] * 256 + instr[5];

    cells[a] += cells[b];

    if (cells[a] < cells[b])
      fseek(file, c, SEEK_SET);

    if (debug_limit) {
      printf("\n\t%d %d %d\n", a, b, c);
      for (long i = 0; i != debug_limit; ++i)
        printf("%.2x ", cells[i]);
      putchar('\n');
    }
  }

  for (long i = 0; i != result_limit; ++i)
    printf("%.2x ", cells[i]);
  putchar('\n');

  return 0;
}

int main(int argc, char const **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: abcout <file>");
    return 1;
  }

  char const *file_name = argv[1];
  uint16_t result_limit = 100;
  uint16_t debug_limit = 0;

  if (argc > 2)
    result_limit = atoi(argv[2]);

  if (argc > 3)
    debug_limit = atoi(argv[3]);

  FILE *file;
  if (fopen_s(&file, file_name, "rb")) {
    printf("Couldn't open %s\n", file_name);
    return 1;
  }
  printf("Running %s (showing first %d cells)\n", file_name, result_limit);
  run(file, result_limit, debug_limit);
  fclose(file);
}
