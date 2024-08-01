const int trigPinL = 2;
const int echoPinL = 3;
const int trigPinR = 4;
const int echoPinR = 5;

const int stepPinL = 8;
const int dirPinL = 9;
const int stepPinM = 10;
const int dirPinM = 16;
const int stepPinR = 6;
const int dirPinR = 7;

const int pulsesPerRev = 400;
const float revsPerSec = 1.0;
const int pulseWidthMicros = 20;

const float pulsesPerSec = pulsesPerRev * revsPerSec;
const int pulseDelayMicros = (int) (1000000 / pulsesPerSec) - pulseWidthMicros;

bool leftOpen = false;

void setup() {
  pinMode(trigPinL, OUTPUT);
  pinMode(echoPinL, INPUT);
  pinMode(trigPinL, OUTPUT);
  pinMode(echoPinL, INPUT);

  pinMode(stepPinL, OUTPUT);
  pinMode(dirPinL, OUTPUT);
  pinMode(stepPinM, OUTPUT);
  pinMode(dirPinM, OUTPUT);
  pinMode(stepPinR, OUTPUT);
  pinMode(dirPinR, OUTPUT);

  Serial.begin(9600);
}

void rotateStepper(float revs, int stepPin, int dirPin) {
  digitalWrite(dirPin, revs < 0 ? LOW : HIGH);

  float absRevs = abs(revs);

  for (int x = 0; x < (int) (absRevs * pulsesPerRev); x++) {
    digitalWrite(stepPin, HIGH); 
    delayMicroseconds(pulseWidthMicros);
    digitalWrite(stepPin, LOW); 
    delayMicroseconds(pulseDelayMicros); 
  }
}

float getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  float duration = pulseIn(echoPin, HIGH);
  return (duration * 0.0343) / 2;
}

void loop() {
  float lDist = getDistance(trigPinL, echoPinL);
  float rDist = getDistance(trigPinR, echoPinR);

  if (leftOpen) {
    rotateStepper(0.5, stepPinM, dirPinM);
    rotateStepper(-0.25, stepPinR, dirPinR);
    rotateStepper(-0.25, stepPinL, dirPinL);

    leftOpen = false;
  } else {
    rotateStepper(-0.5, stepPinM, dirPinM);
    rotateStepper(0.25, stepPinR, dirPinR);
    rotateStepper(0.25, stepPinL, dirPinL);

    leftOpen = true;
  }
  delay(10000); //change every 5-10 seconds. 10 for now
}
