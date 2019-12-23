#include "io.h"
#include "wave.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc != 5) {
    fatal_error("Incorrect number of arguments");
  }
  int16_t int16Max = 32767;
  int16_t int16Min = -32768;
  char * wavfilein = argv[1];
  FILE * input = fopen(wavfilein, "rb");
  FILE * output = fopen(argv[2], "wb");
  if (input == NULL) {
    fatal_error("Failed to open input file");
  }
  int delay = (int) atoi(argv[3]); //Delay in # of samples
  float amplitude = (float) atof(argv[4]);
  unsigned numSamples; //Total samples
  read_wave_header(input, &numSamples);
  if (delay > (int) numSamples) {
    fatal_error("Delay longer than entire clip");
  }
  unsigned numChannels = numSamples * 2; //TOTAL number of channel data
  int16_t * buf = calloc(numChannels, sizeof(int16_t)); //Buffer initialized to 0
  read_s16_buf(input, buf, numChannels); //Reads input sample data into buffer
  int32_t sampleHolder; //Necessary to check for clipping
  //Adds sample * amplitude to later in buffer according to delay value
  for(int i = 0; i < (int) numChannels - (delay * 2); i++) {
    sampleHolder = buf[i + (delay * 2)] + (amplitude * buf[i]);
    //Check for clipping
    if (sampleHolder > int16Max) {
      sampleHolder = int16Max;
    }
    if (sampleHolder < int16Min) {
      sampleHolder = int16Min;
    }
    buf[i + (delay * 2)] = sampleHolder;
  }
  write_wave_header(output, numSamples);
  write_s16_buf(output, buf, numChannels);

  //Close stuff
  free(buf);
  fclose(output);
  fclose(input);
}
