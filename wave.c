#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "io.h"
#include "wave.h"

/*
 * Write a WAVE file header to given output stream.
 * Format is hard-coded as 44.1 KHz sample rate, 16 bit
 * signed samples, two channels.
 *
 * Parameters:
 *   out - the output stream
 *   num_samples - the number of (stereo) samples that will follow
 */
void write_wave_header(FILE *out, unsigned num_samples) {
  /*
   * See: http://soundfile.sapp.org/doc/WaveFormat/
   */

  uint32_t ChunkSize, Subchunk1Size, Subchunk2Size;
  uint16_t NumChannels = NUM_CHANNELS;
  uint32_t ByteRate = SAMPLES_PER_SECOND * NumChannels * (BITS_PER_SAMPLE/8u);
  uint16_t BlockAlign = NumChannels * (BITS_PER_SAMPLE/8u);

  /* Subchunk2Size is the total amount of sample data */
  Subchunk2Size = num_samples * NumChannels * (BITS_PER_SAMPLE/8u);
  Subchunk1Size = 16u;
  ChunkSize = 4u + (8u + Subchunk1Size) + (8u + Subchunk2Size);

  /* Write the RIFF chunk descriptor */
  write_bytes(out, "RIFF", 4u);
  write_u32(out, ChunkSize);
  write_bytes(out, "WAVE", 4u);

  /* Write the "fmt " sub-chunk */
  write_bytes(out, "fmt ", 4u);       /* Subchunk1ID */
  write_u32(out, Subchunk1Size);
  write_u16(out, 1u);                 /* PCM format */
  write_u16(out, NumChannels);
  write_u32(out, SAMPLES_PER_SECOND); /* SampleRate */
  write_u32(out, ByteRate);
  write_u16(out, BlockAlign);
  write_u16(out, BITS_PER_SAMPLE);

  /* Write the beginning of the "data" sub-chunk, but not the actual data */
  write_bytes(out, "data", 4);        /* Subchunk2ID */
  write_u32(out, Subchunk2Size);
}

/*
 * Read a WAVE header from given input stream.
 * Calls fatal_error if data can't be read, if the data
 * doesn't follow the WAVE format, or if the audio
 * parameters of the input WAVE aren't 44.1 KHz, 16 bit
 * signed samples, and two channels.
 *
 * Parameters:
 *   in - the input stream
 *   num_samples - pointer to an unsigned variable where the
 *      number of (stereo) samples following the header
 *      should be stored
 */
void read_wave_header(FILE *in, unsigned *num_samples) {
  char label_buf[4];
  uint32_t ChunkSize, Subchunk1Size, SampleRate, ByteRate, Subchunk2Size;
  uint16_t AudioFormat, NumChannels, BlockAlign, BitsPerSample;

  read_bytes(in, label_buf, 4u);
  if (memcmp(label_buf, "RIFF", 4u) != 0) {
    fatal_error("Bad wave header (no RIFF label)");
  }

  read_u32(in, &ChunkSize); /* ignore */

  read_bytes(in, label_buf, 4u);
  if (memcmp(label_buf, "WAVE", 4u) != 0) {
    fatal_error("Bad wave header (no WAVE label)");
  }

  read_bytes(in, label_buf, 4u);
  if (memcmp(label_buf, "fmt ", 4u) != 0) {
    fatal_error("Bad wave header (no 'fmt ' subchunk ID)");
  }

  read_u32(in, &Subchunk1Size);
  if (Subchunk1Size != 16u) {
    fatal_error("Bad wave header (Subchunk1Size was not 16)");
  }

  read_u16(in, &AudioFormat);
  if (AudioFormat != 1u) {
    fatal_error("Bad wave header (AudioFormat is not PCM)");
  }

  read_u16(in, &NumChannels);
  if (NumChannels != NUM_CHANNELS) {
    fatal_error("Bad wave header (NumChannels is not 2)");
  }

  read_u32(in, &SampleRate);
  if (SampleRate != SAMPLES_PER_SECOND) {
    fatal_error("Bad wave header (Unexpected sample rate)");
  }

  read_u32(in, &ByteRate); /* ignore */

  read_u16(in, &BlockAlign); /* ignore */

  read_u16(in, &BitsPerSample);
  if (BitsPerSample != BITS_PER_SAMPLE) {
    fatal_error("Bad wave header (Unexpected bits per sample)");
  }

  read_bytes(in, label_buf, 4u);
  if (memcmp(label_buf, "data", 4u) != 0) {
    fatal_error("Bad wave header (no 'data' subchunk ID)");
  }

  /* finally we're at the Subchunk2Size field, from which we can
   * determine the number of samples */
  read_u32(in, &Subchunk2Size);
  *num_samples = Subchunk2Size / NUM_CHANNELS / (BITS_PER_SAMPLE/8u);
}
/*
 * Renders half a stereo digital representation of a sine wave given the frequency
 * and amplitude into one channel. Each "sample" represents a positive or negative pressure
 * at a point in time. Sample value ranges from -32768 to 32767 oscillating
 * according to formula to calculate sin value at a given time
 */
void render_sine_wave(int16_t buf[], unsigned num_samples, unsigned channel,  float freq_hz, float amplitude) {
  double currTime, sample;
  int int16Max = 32767;
  int int16Min = -32768;
  int32_t sampleInt; //Necessary to hold sample + buf[i] incase of overflow
  //fills out 2N or 2N + 1 elements depending on channel
  for (int i = channel; i < (int) num_samples * 2; i += 2) {
    currTime = ((double) ((((i - channel)) / 2) + 1)) / SAMPLES_PER_SECOND;
    /*
      Formula for pressure ("sample") of a sound wave.
      Can be positive or negative value ranging from
      -1.0 to 1.0.
    */
    sample = amplitude * sin(currTime * freq_hz * 2 * PI); 
    /*
      Converts pressure value to fillout int16_t range (-32768 to 32767)
      Also adds to current sample value and caps if above/below int16 range
    */
    if (sample > 0) {
      sampleInt = (int32_t) ((sample * int16Max * amplitude) + buf[i]);
    }
    else {
      sampleInt = (int32_t) ((sample * (-1 * int16Min) * amplitude) + buf[i]);
    }
  
    if (sampleInt > int16Max) {
      sampleInt = int16Max;
    }
    if (sampleInt < int16Min) {
      sampleInt = int16Min;
    }
    buf[i] = sampleInt;
  }
}
//Simply calls render_sine_wave once for each channel with given num_samples, frequency, and amplitude.
void render_sine_wave_stereo(int16_t buf[], unsigned num_samples, float freq_hz, float amplitude) {
  unsigned channel1 = 0;
  unsigned channel2 = 1;
  render_sine_wave(buf, num_samples, channel1, freq_hz, amplitude);
  render_sine_wave(buf, num_samples, channel2, freq_hz, amplitude);
}
/* Same idea of render_sine_wave but anytime pressure would be positive it is
 * brought to max value (with respect to amplitude and vice versa for negative
 * values.
 */
void render_square_wave(int16_t buf[], unsigned num_samples, unsigned channel,  float freq_hz, float amplitude) {
  double currTime, sample;
  int int16Max = 32767;
  int int16Min = -32768;
  int32_t sampleInt;
  //fills out 2N or 2N + 1 elements depending on channel
  for (int i = channel; i < (int) num_samples * 2; i += 2) {
   currTime = ((double) ((((i - channel)) / 2) + 1)) / SAMPLES_PER_SECOND;
    /*
      Formula for pressure ("sample") of a sound wave.
      Can be positive or negative value ranging from
      -1.0 to 1.0.
    */
    sample = amplitude * sin(currTime * freq_hz * 2 * PI); 
    //Converts pressure value to fillout int16_t range (-32768 to 32767)
    //Also adds to current sample value and caps if above/below int16 range
    if (sample > -0.001 || sample < 0.001) { //necessary to check if 0
      sampleInt = 0;
    }
    if (sample > 0) {
      sampleInt = (int32_t) ((int16Max * amplitude) + buf[i]);
    }
    if (sample < 0) {
      sampleInt = (int32_t) ((int16Min * amplitude) + buf[i]);
    }
  
    if (sampleInt > 0) {
      sampleInt = int16Max * amplitude;
    }
    if (sampleInt < 0) {
      sampleInt = int16Min * amplitude;
    }
    buf[i] = sampleInt;
  }
}
//Calls render_square_wave twice to fill each channel given num_samples, frequency, and amplitude
void render_square_wave_stereo(int16_t buf[], unsigned num_samples,  float freq_hz, float amplitude) {
  unsigned channel1 = 0;
  unsigned channel2 = 1;
  render_square_wave(buf, num_samples, channel1, freq_hz, amplitude);
  render_square_wave(buf, num_samples, channel2, freq_hz, amplitude);
}

/* Over the duration of each cycle, linearly increases in pressure starting
 * from the minimum value and ending (at the end of one cycle) at the maximum
 * value. At the beginning of the next cycle it begins at the minimum again.
 * Requires the int16_t buffer to fill, the number of samples, the channel to fill,
 * the frequency and the amplitude.
 */
void render_sawtooth_wave(int16_t buf[], unsigned num_samples, unsigned channel,  float freq_hz, float amplitude) {
  double currTime, sample, cycleTime;
  int int16Max = 32767;
  int int16Min = -32768;
  int absint16Max = 65535;
  int32_t sampleInt;
  double dump;
  cycleTime = (1 / (double) freq_hz);
  //fills out 2N or 2N + 1 elements depending on channel
  for (int i = channel; i < (int) num_samples * 2; i += 2) {
    currTime = ((double) ((((i - channel)) / 2) + 1)) / SAMPLES_PER_SECOND;
    //Sample increases linearly over length of cycle.
    sample = amplitude * modf((currTime / cycleTime), &dump);
    //Converts pressure value to fillout int16_t range (-32768 to 32767)
    //Also adds to current sample value and caps if above/below int16 range
    sampleInt = (int32_t) (((sample * absint16Max) + int16Min) + buf[i]);
    //Clipping
    if (sampleInt > int16Max) {
      sampleInt = int16Max;
    }
    if (sampleInt < int16Min) {
      sampleInt = int16Min;
    }
    buf[i] = sampleInt;
  }
}

/* Calls render_sawtooth_wave twice to fill out each channel given
 * buffer to fill, number of samples, frequency, amplitude.
 */
void render_sawtooth_wave_stereo(int16_t buf[], unsigned num_samples,  float freq_hz, float amplitude) {
  unsigned channel1 = 0;
  unsigned channel2 = 1;
  render_sawtooth_wave(buf, num_samples, channel1, freq_hz, amplitude);
  render_sawtooth_wave(buf, num_samples, channel2, freq_hz, amplitude);
}

/* switch to determine which type of wave to render
 * Requires buffer to fill, number of samples, channel to fill, frequency, amplitude, type of wave.
 */
void render_voice(int16_t buf[], unsigned num_samples, unsigned channel, float freq_hz, float amplitude, unsigned voice) {
  switch(voice) {
  case 0:
    render_sine_wave(buf, num_samples, channel, freq_hz, amplitude);
    break;
  case 1:
    render_square_wave(buf, num_samples, channel, freq_hz, amplitude);
    break;
  case 2:
    render_sawtooth_wave(buf, num_samples, channel, freq_hz, amplitude);
    break;
  default:
    fatal_error("Invalid wave: 0 (sine), 1 (square), 2 (sawtooth)");
    break;
  }
}

/* Calls render_voice twice for each channel
 * Requires buffer to fill, number of samples, frequency, amplitude, type of wave.
 */
void render_voice_stereo(int16_t buf[], unsigned num_samples, float freq_hz, float amplitude, unsigned voice) {
  unsigned channel1 = 0;
  unsigned channel2 = 1;
  render_voice(buf, num_samples, channel1, freq_hz, amplitude, voice);
  render_voice(buf, num_samples, channel2, freq_hz, amplitude, voice);
}

//Returns frequency of MIDI note supplied
float getFreq(int note){
  float freq;
  float power = (float) (note-69) / (float) 12;
  freq = 440 * pow(2, power);
  return freq;

}
