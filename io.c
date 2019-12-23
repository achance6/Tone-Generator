#include <stdlib.h>
#include "io.h"
#include <stdio.h>
#include <stdint.h>

//Exits program with error message
void fatal_error(const char *message) {
  fprintf(stderr, "Error: %s\n", message);
  exit(0);
}

//Writes a single byte (char) to output file
void write_byte(FILE *out, char val) {
  fputc(val, out);
  if (ferror(out) != 0) {
    fatal_error("Error writing byte\n");
  }
}

//Writes multiples bytes(chars from an array of length n) to an output file.
void write_bytes(FILE *out, const char data[], unsigned n) {
  for(int i = 0; i < (int) n; i++) {
    write_byte(out, data[i]);
  }
}

//Writes a value of type uint16_t in binary format to a file.
void write_u16(FILE *out, uint16_t value) {
  fputc(value & 0xFF, out);
  fputc((value >> 8) & 0xFF, out);
  if (ferror(out) != 0) {
    fatal_error("Error writing byte\n");
  }
}

//Writes a value of type uint32_t in binary format to a file.
void write_u32(FILE *out, uint32_t value) {
  fputc(value & 0xFF, out);
  fputc((value >> 8) & 0xFF, out);
  fputc((value >> 16) & 0xFF, out);
  fputc((value >> 24) & 0xFF, out);
  if (ferror(out) != 0) {
    fatal_error("Error writing byte\n");
  }
}

//Writes a value of type int16_t in binary format to a file.
void write_s16(FILE *out, int16_t value) {
  fputc(value & 0xFF, out);
  fputc((value >> 8) & 0xFF, out);
  if (ferror(out) != 0) {
    fatal_error("Error writing byte\n");
  }
}

//Writes multiple values of type int16_t (from an array of length n) to a file.
void write_s16_buf(FILE *out, const int16_t buf[], unsigned n) {
  for(int i = 0; i < (int) n; i++) {
    write_s16(out, buf[i]);
  }
}

//Read a single byte (as a character) from a file to an address.
void read_byte(FILE *in, char *val) {
  fscanf(in, "%c", val);
  if (ferror(in) != 0) {
    fatal_error("Error reading byte\n");
  }
}

//Reads multiple bytes (as characters) from a file into an array of length n
void read_bytes(FILE *in, char data[], unsigned n) {
  for(int i = 0; i < (int) n; i++) {
    read_byte(in, data + i);
  }
}

//reads a byte from a file into a uint16_t address.
void read_u16(FILE *in, uint16_t *val) {
  uint16_t v1 = (uint16_t) fgetc(in);
  uint16_t v2 = (uint16_t) fgetc(in);
  if (ferror(in) != 0) {
    fatal_error("Error reading byte\n");
  }
  *val = v1 | (v2 << 8);
}

//Read a byte from a file into a uint32_t address.
void read_u32(FILE *in, uint32_t *val) {
  uint32_t v1 = (uint32_t) fgetc(in);
  uint32_t v2 = (uint32_t) fgetc(in);
  uint32_t v3 = (uint32_t) fgetc(in);
  uint32_t v4 = (uint32_t) fgetc(in);
  if (ferror(in) != 0) {
    fatal_error("Error reading byte\n");
  }
  *val = v1 | (v2 << 8) | (v3 << 16) | (v4 << 24);
}

//Reads a byte from a file into a int16_t address.
void read_s16(FILE *in, int16_t *val) {
  int16_t v1 = (int16_t) fgetc(in);
  int16_t v2 = (int16_t) fgetc(in);
  if (ferror(in) != 0) {
    fatal_error("Error reading byte\n");
  }
  *val = v1 | (v2 << 8);
}

//Reads bytes from a file into a int16_t array of length n.
void read_s16_buf(FILE *in, int16_t buf[], unsigned n) {
  for (int i = 0; i < (int) n; i++) {
    read_s16(in, buf + i);
  }
}
