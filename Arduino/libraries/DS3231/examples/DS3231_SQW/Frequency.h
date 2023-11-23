unsigned long prevMicros;
volatile long Frequency = 0;
const long OneSecond = 1000000;
const byte interruptPin = 2;

void Measure() {
  Frequency++;
}
