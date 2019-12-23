#include "io.h"
#include "wave.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc != 6) {
    fatal_error("Usage: voice(0, 1, 2), frequency (hz), amplitude, # samples, wavfile");
  }
  unsigned voice = (unsigned) atoi(argv[1]);
  float freq_hz = (float) atof(argv[2]);
  float amplitude = (float) atof(argv[3]);
  //Each sample has 2 channels of data
  unsigned numSamples = (unsigned) atoi(argv[4]);
  char* wavfileout = argv[5];
  unsigned numChannelData = numSamples * 2;
  //End input collection
  
  int16_t * buf = calloc(numChannelData, sizeof(int16_t));
  render_voice_stereo(buf, numSamples, freq_hz, amplitude, voice);
  FILE* output = fopen(wavfileout, "wb");
  write_wave_header(output, numSamples);
  write_s16_buf(output, buf, numChannelData);
  //End file writing
  
  free(buf);
  fclose(output);

}
