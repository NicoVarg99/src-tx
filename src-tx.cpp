#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sndfile.h>

#define SAMPLE_RATE 44100.0
#define BUFFER_SIZE 485100 //11 seconds * sample rate, total duration
#define PI 3.141592654

#define ZERO 2000 //Hz
#define ONE 2500 //Hz
#define SYNC 1000 //Hz
#define BITLEN 30 //Milliseconds

double buffer[BUFFER_SIZE];
double cursor = 0;

int msToSamples(int ms) {
  return SAMPLE_RATE * ms / 1000;
}

void pushTone(double frequency, int duration) {
  double amplitude = 1.0;

  for (int s = cursor; s < BUFFER_SIZE && s < cursor + duration; ++s)
      buffer[s] += amplitude * sin( (2.0 * PI * frequency) * (s / SAMPLE_RATE));

  cursor += duration;
}

void pushBit(int bit) {
  double frequency = (bit ? ONE : ZERO);
  int duration = msToSamples(BITLEN);
  return pushTone(frequency, duration);
}

void pushArray(int arr[], int len) {
  for (int i = 0; i < len; i++)
    pushBit(arr[i]);
}

void addSync() {
  cursor = 4 * SAMPLE_RATE;
  pushTone(SYNC, msToSamples(100));
  cursor = 5 * SAMPLE_RATE;
  pushTone(SYNC, msToSamples(100));
  cursor = 6 * SAMPLE_RATE;
  pushTone(SYNC, msToSamples(100));
  cursor = 7 * SAMPLE_RATE;
  pushTone(SYNC, msToSamples(100));
  cursor = 8 * SAMPLE_RATE;
  pushTone(SYNC, msToSamples(100));
  cursor = 10 * SAMPLE_RATE;
  pushTone(SYNC, msToSamples(100));
}

void GenS1() {
  cursor = (SAMPLE_RATE * 2);
  int s[32];
  s[0] = 0;
  s[1] = 1;

  for (int i = 2; i < 32; i++)
    s[i] = 1;

  pushArray(s, 32);
}

void GenS2() {
  cursor = (SAMPLE_RATE * 3);
  int s[16];
  s[0] = 1;
  s[1] = 0;

  for (int i = 2; i < 16; i++)
    s[i] = 1;

  pushArray(s, 16);
}

void writeAudioFile(double *buffer) {

    SF_INFO sndInfo;

    sndInfo.samplerate = SAMPLE_RATE;
    sndInfo.channels   = 1;
    sndInfo.frames     = BUFFER_SIZE;
    sndInfo.format     = SF_FORMAT_PCM_16 | SF_FORMAT_WAV;

    SNDFILE *outFile = sf_open("src.wav", SFM_WRITE, &sndInfo);

    sf_writef_double(outFile, buffer, BUFFER_SIZE);
    sf_close(outFile);
}

int main() {
  for (int s = 0; s < BUFFER_SIZE; ++s)
      buffer[s] = 0;

  printf("SRC - Segnale Rai Codificato\n");

  GenS1();
  GenS2();
  addSync();

  double max = 1.0;
  for (int s = 0; s < BUFFER_SIZE; ++s)
      if (buffer[s] > max)
          max = buffer[s];

  //normalize
  for (int s = 0; s < BUFFER_SIZE; ++s)
      buffer[s] /= max;

  writeAudioFile(buffer);
}
