//render_song.c

#include "io.h"
#include "wave.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
  if(argc != 3) {
    fatal_error("Invalid # of args");
  }
  FILE * songinput = fopen(argv[1], "r");
  char* wavfileout = argv[2];
  //end input collection

  FILE * output = fopen(wavfileout, "wb");
  if (songinput == NULL) {
    fatal_error("Failed to open file");
  }
  unsigned numSamples; //number of total samples
  unsigned numSPB; //number of samples per beat
  if (fscanf(songinput, "%u %u", &numSamples, &numSPB) != 2) {
    fatal_error("Malformed Input");
  }
  char directive;
  float b;
  unsigned n;
  float amplitude  = 0.1; // default amplitude is 0.1
  unsigned voice = 0; //default voice is sine wave
  float freq_hz;
  unsigned samples; //Used to know how many samples to write for each directive
  write_wave_header(output, numSamples);
  int16_t * buf = calloc(numSamples * 2, sizeof(int16_t));
  int16_t * bufIndex = buf; //Used to track current place in buffer.
  //Execute according to directives
  while(fscanf(songinput, " %c", &directive) == 1) {
    //single note for # of beats
    if (directive == 'N'){
      fscanf(songinput, " %f %u", &b, &n);
      freq_hz = getFreq(n);
      samples = b * (float)numSPB;
      render_voice_stereo(bufIndex, samples, freq_hz, amplitude, voice);
      write_s16_buf(output, bufIndex, samples * 2);
      bufIndex += samples * 2;
    }
    //Chord for # of beats
    else if (directive == 'C') {
      fscanf(songinput, " %f", &b);
      samples = b * (float)numSPB;
      fscanf(songinput, " %u", &n);
      //Keep loading buffer with specified notes (by adding note samples together) until n = 999(stop).
      while (n != 999) {	
	freq_hz = getFreq(n);
	render_voice_stereo(bufIndex, samples, freq_hz, amplitude, voice);
	fscanf(songinput, " %u", &n);
      }
      write_s16_buf(output, bufIndex, samples * 2);
      bufIndex += samples * 2;
    }
    //Pause for # of beats
    else if (directive == 'P') {
      fscanf(songinput, " %f", &b);
      samples = b * (float)numSPB;
      render_voice_stereo(bufIndex, samples, 0, 0, voice); //Fills buffer with filler
      write_s16_buf(output, bufIndex, samples * 2);
      bufIndex += samples * 2;
    }
    //Alters voice
    else if (directive == 'V') {
      fscanf(songinput, " %u", &voice);
    }
    //Alters amplitude
    else if (directive == 'A') {
      fscanf(songinput, " %f", &amplitude);
    }
    else {
      fatal_error("Malformed Input");
    }
  }
  //closing stuff
  free(buf);
  fclose(songinput);
  fclose(output);
}
