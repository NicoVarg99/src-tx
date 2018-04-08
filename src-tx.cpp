#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sndfile.h>
#include <iostream>
#include <ctime>
#include <vector>

#define SAMPLE_RATE 44100.0
#define BUFFER_SIZE 485100 //11 seconds * sample rate, total duration
#define PI 3.141592654

#define ZERO 2000 //Hz
#define ONE 2500 //Hz
#define SYNC 1000 //Hz
#define BITLEN 30 //Milliseconds

double buffer[BUFFER_SIZE];
double cursor = 0;
int s1[32];
int s1cursor = 0;
int s2[16];
int s2cursor = 0;

tm *ltmtx;

int getDigit(int number, int digit) {
  return number / ((int) pow(10, digit)) % 10;
}

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

void pushBit(int bit, int n) {
  for (int i = 0; i < n; i++)
    pushBit(bit);
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

std::vector<int> push1bcd(int num) {
  std::vector< int > arr;

  switch(num) {
    case 0 : arr.push_back(0); break;
    case 1 : arr.push_back(1); break;
  }

  return arr;
}

std::vector<int> push2bcd(int num) {
  std::vector< int > arr;

  switch(num) {
    case 0 : arr.push_back(0); arr.push_back(0); break;
    case 1 : arr.push_back(0); arr.push_back(1); break;
    case 2 : arr.push_back(1); arr.push_back(0); break;
    case 3 : arr.push_back(1); arr.push_back(1); break;
  }

  return arr;
}

std::vector<int> push3bcd(int num) {
  std::vector< int > arr;

  switch(num) {
    case 0 : arr.push_back(0); arr.push_back(0); arr.push_back(0); break;
    case 1 : arr.push_back(0); arr.push_back(0); arr.push_back(1); break;
    case 2 : arr.push_back(0); arr.push_back(1); arr.push_back(0); break;
    case 3 : arr.push_back(0); arr.push_back(1); arr.push_back(1); break;
    case 4 : arr.push_back(1); arr.push_back(0); arr.push_back(0); break;
    case 5 : arr.push_back(1); arr.push_back(0); arr.push_back(1); break;
    case 6 : arr.push_back(1); arr.push_back(1); arr.push_back(0); break;
    case 7 : arr.push_back(1); arr.push_back(1);  arr.push_back(1);break;
  }

  return arr;
}

std::vector<int> push4bcd(int num) {
  std::vector< int > arr;

  switch(num) {
    case 0 : arr.push_back(0); arr.push_back(0); arr.push_back(0); arr.push_back(0); break;
    case 1 : arr.push_back(0); arr.push_back(0); arr.push_back(0); arr.push_back(1); break;
    case 2 : arr.push_back(0); arr.push_back(0); arr.push_back(1); arr.push_back(0); break;
    case 3 : arr.push_back(0); arr.push_back(0); arr.push_back(1); arr.push_back(1); break;
    case 4 : arr.push_back(0); arr.push_back(1); arr.push_back(0); arr.push_back(0); break;
    case 5 : arr.push_back(0); arr.push_back(1); arr.push_back(0); arr.push_back(1); break;
    case 6 : arr.push_back(0); arr.push_back(1); arr.push_back(1); arr.push_back(0); break;
    case 7 : arr.push_back(0); arr.push_back(1); arr.push_back(1); arr.push_back(1); break;
    case 8 : arr.push_back(1); arr.push_back(0); arr.push_back(0); arr.push_back(0); break;
    case 9 : arr.push_back(1); arr.push_back(0); arr.push_back(0); arr.push_back(1); break;
    case 10 : arr.push_back(1); arr.push_back(0); arr.push_back(1); arr.push_back(0); break;
    case 11 : arr.push_back(1); arr.push_back(0); arr.push_back(1); arr.push_back(1); break;
    case 12 : arr.push_back(1); arr.push_back(1); arr.push_back(0); arr.push_back(0); break;
    case 13 : arr.push_back(1); arr.push_back(1); arr.push_back(0); arr.push_back(1); break;
    case 14 : arr.push_back(1); arr.push_back(1); arr.push_back(1); arr.push_back(0); break;
    case 15 : arr.push_back(1); arr.push_back(1); arr.push_back(1); arr.push_back(1); break;
  }

  return arr;
}

int parity(int *arr, int a, int b) {
  int sum = 0;
  // std::cout << a << "a" << std::endl;
  // std::cout << b << "b" << std::endl;

  for (int i = a; i < b; i++)
    sum += arr[i];

  if (sum % 2 == 0)
    return 0;
  return 1;
}


void GenS1() {
  cursor = (SAMPLE_RATE * 2);
  std::vector<int> v;

  // ID = identificatore del segmento di codice - 2
  s1[s1cursor++] = 0;
  s1[s1cursor++] = 1;

  // OR = ore (decine ed unita') - 2+4 = 6
  v = push2bcd(getDigit(ltmtx->tm_hour, 1));
  for (auto i = v.begin(); i != v.end(); ++i) //Copy v to s1
    s1[s1cursor++] = *i;

  v = push4bcd(getDigit(ltmtx->tm_hour, 0));
  for (auto i = v.begin(); i != v.end(); ++i)
    s1[s1cursor++] = *i;

  // MI = minuti (decine ed unita') - 3+4 = 7
  v = push3bcd(getDigit(ltmtx->tm_min, 1));
  for (auto i = v.begin(); i != v.end(); ++i)
    s1[s1cursor++] = *i;

  v = push4bcd(getDigit(ltmtx->tm_min, 0));
  for (auto i = v.begin(); i != v.end(); ++i)
    s1[s1cursor++] = *i;

  // std::cout << "min decine " << getDigit(ltmtx->tm_min, 1) << std::endl;
  // std::cout << "min unita " << getDigit(ltmtx->tm_min, 0) << std::endl;

  // OE = ora estiva (1), ora solare (0) - 1
  s1[s1cursor++] = ltmtx->tm_isdst;

  // P1 = parita' del primo gruppo di informazioni - 1
  s1[s1cursor++] = parity(s1, 2, s1cursor - 1);

  // ME = mese (decine ed unita') 1 + 4 = 5
  v = push1bcd(getDigit(ltmtx->tm_mon, 1));
  for (auto i = v.begin(); i != v.end(); ++i)
    s1[s1cursor++] = *i;

  v = push4bcd(getDigit(ltmtx->tm_mon, 0));
  for (auto i = v.begin(); i != v.end(); ++i)
    s1[s1cursor++] = *i;

  // GM = giorno del mese (decine ed unita') 2+4 =  6
  v = push2bcd(getDigit(ltmtx->tm_mday, 1));
  for (auto i = v.begin(); i != v.end(); ++i)
    s1[s1cursor++] = *i;

  v = push4bcd(getDigit(ltmtx->tm_mday, 0));
  for (auto i = v.begin(); i != v.end(); ++i)
    s1[s1cursor++] = *i;

  // GS = giorno settimana (1 = lunedi') 3
  v = push3bcd(ltmtx->tm_wday);
  for (auto i = v.begin(); i != v.end(); ++i)
    s1[s1cursor++] = *i;

  // P2 = parita' del secondo gruppo di informazioni - 1
  s1[s1cursor++] = parity(s1, 17, s1cursor - 1);

  for (int i = 0; i < 32; i++) //Print first signal
    std::cout << s1[i];
  std::cout << std::endl;

  pushArray(s1, 32);
}

void GenS2() {
  cursor = (SAMPLE_RATE * 3);
  std::vector<int> v;

  // ID = identificatore del segmento di codice
  s2[s1cursor++] = 1;
  s2[s1cursor++] = 0;

  // AN = anno (decine ed unita')
  v = push2bcd(getDigit(ltmtx->tm_year - 100, 1));
  for (auto i = v.begin(); i != v.end(); ++i)
    s2[s2cursor++] = *i;

  v = push4bcd(getDigit(ltmtx->tm_year - 100, 0));
  for (auto i = v.begin(); i != v.end(); ++i)
    s2[s2cursor++] = *i;

  // SE = preavviso cambio ora solare/estiva
  // 111 : nessun cambio nei prossimi 7 giorni
  // 110 : previsto un cambio entro 6 giorni
  // ...
  // 001 : previsto un cambio entro 1 giorno
  // 000 : alle ore 02:00 si passa all'ora estiva oppure alle 03:00 si passa all'ora solare.

  v = push3bcd(7);
  for (auto i = v.begin(); i != v.end(); ++i)
    s2[s2cursor++] = *i;


  // SI = preavviso secondo intercalare
  // 00 : nessun secondo intercalare entro il mese
  // 01 : ritardo di 1 secondo a fine mese
  // 11 : anticipo di 1 secondo a fine mese.
  v = push2bcd(0);
  for (auto i = v.begin(); i != v.end(); ++i)
    s2[s2cursor++] = *i;

  // PA = parita'
  s2[s2cursor++] = parity(s2, 2, s2cursor - 1);

  for (int i = 0; i < 16; i++) //Print second signal
    std::cout << s2[i];
  std::cout << std::endl;

  pushArray(s2, 16);
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

  std::time_t currTime = time(0); // current date/time based on current system
  std::time_t timeToTX = time(0); // current date/time based on current system
  tm *ltm = localtime(&timeToTX);

  if (ltm->tm_sec > 45) {
    timeToTX = time(0) + 120;
  } else {
    timeToTX = time(0) + 60;
  }

  ltmtx = localtime(&timeToTX);
  ltmtx->tm_sec = 0;
  std::cout << "Date to transmit: " << ltmtx->tm_mday << "/" << ltmtx->tm_mon << "/" << (ltmtx->tm_year + 1900) << std::endl;
  std::cout << "Time to transmit: " << ltmtx->tm_hour << ":" << ltmtx->tm_min << ":" << ltmtx->tm_sec << std::endl;
  std::cout << "DST: " << (ltmtx->tm_isdst ? "yes" : "no")  << std::endl;

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
