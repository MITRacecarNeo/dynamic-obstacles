const int trigPinL = 2;
const int echoPinL = 3;

const int trigPinR = 4;
const int echoPinR = 5;

void setup() {
  pinMode(trigPinL, OUTPUT);
  pinMode(echoPinL, INPUT);
  pinMode(trigPinR, OUTPUT);
  pinMode(echoPinR, INPUT);
  Serial.begin(9600);
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

  Serial.print("L: ");
  Serial.print(lDist, 3);
  Serial.print(" R: ");
  Serial.println(rDist, 3);
  delay(100);
}
