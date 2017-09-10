/*
Read data being sent from another ATMega328 to the Arduino using the photodiode,
and print out the data to the console. This can be used with remoteprog.c
on a second ARMega328 to remotely read data on button presses. 
*/
int threshold;
void setup() {
  // Photodiode pin
  pinMode(A0, INPUT);
  Serial.begin(9600);
  // get threshold value for photodiode when IR LED is off
  int min_val=10000, max_val=0, x;
  digitalWrite(3,HIGH);
  for (int i=0; i<32; i++) {
    x = analogRead(A0);
    delay(1);
    if (x < min_val)
      min_val = x;
    if (x > max_val)
      max_val = x;
  }
  threshold = max_val + (max_val-min_val)/3;
  Serial.print("Threshold = ");
  Serial.println(threshold);
}

long sps = 18950;
int bitLen = sps/100, halfBitLen = sps/200;

byte getByte() {
  long k;
  int a;
  int pos = 0;
  byte ans = 0;
  long readings[8];
  int p = 0;
  // get lengths of on and off sequences and store
  long stopNum = 9*bitLen;
  countCurrent();
  while (stopNum > 0) {
    k = ctCurStop(stopNum);
    readings[p++] = k;
    stopNum -= k;
  }  
  // split values of counts into individual bits
  for (int i=0; i<p; i++) {
    a = (readings[i]+halfBitLen)/bitLen;
    if (i == 0) a--;
    while (a > 0) {
      if (i%2 == 0)
        ans |= (1<<pos);
      pos++;
      a--;
    }
  }
  return ans;
}

long countCurrent() {
  byte val = analogRead(A0) > threshold;
  long k = 0;
  while ((analogRead(A0) > threshold) == val)
    k++;
  return k;
}

// returns number of consecutive readings with same value, up to a given cap
long ctCurStop(long stopNum) {
  byte val = analogRead(A0) > threshold;
  long k = 0;
  while ((analogRead(A0) > threshold) == val && k < stopNum)
    k++;
  return k;
}

// continually read and print bytes from the communications link
void loop() {
  Serial.println(getByte());
}
