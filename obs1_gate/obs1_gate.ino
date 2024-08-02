// const int trigPin = 9;
// const int echoPin = 10;
const int stepPin = 10;
const int dirPin = 16;

const int pulsesPerRev = 400;
const float revsPerSec = 1.0;
const int pulseWidthMicros = 20;

const float pulsesPerSec = pulsesPerRev * revsPerSec;
const int pulseDelayMicros = (int) (1000000 / pulsesPerSec) - pulseWidthMicros;

bool stopSignVisible = true;

void setup() {
  // pinMode(trigPin, OUTPUT);
  // pinMode(echoPin, INPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);

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
  // float dist = getDistance(trigPin, echoPin);

  if (stopSignVisible) {
    Serial.println("Hiding stop sign and opening gate...");
    rotateStepper(-0.25, stepPin, dirPin);
  } else {
    Serial.println("Showing stop sign and closing gate...");
    rotateStepper(0.25, stepPin, dirPin);
  }

  stopSignVisible = !stopSignVisible;
  delay(10000);
}
