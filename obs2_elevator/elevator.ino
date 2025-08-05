/*********************************************************************
 * Elevator controller — sends RUN / STOP status to pixel display
 * Code on the Arduino Uno
 *  • HIGH  on pin 12  = STOP  (pixel board draws red frame)
 *  • LOW   on pin 12  = GO    (pixel board draws green frame)
 *
 * Buttons on this board:
 *  • stopBt   (pin 7)  : emergency stop
 *  • startBt  (pin 6)  : begin automatic cycle
 *  • resetBt  (pin 5)  : zero encoder (only when stationary)
 *
 * Motion profile:
 * 1.  Move up to 5000 counts, show STOP (red)
 * 2.  Hold 5 s
 * 3.  Move down to 0, show GO (green)
 * 4.  Hold 7 s
 * 5.  Repeat until STOP pressed
 *********************************************************************/

#include <Servo.h>

// ── Hardware pins ──────────────────────────────────────────────────
const int pwmPin  = A4;        // ESC signal to Spark Mini
const int encA    = 2;         // encoder A  (INT0)
const int encB    = 3;         // encoder B  (INT1)
const int stopBt  = 7;         // buttons: INPUT_PULLUP
const int startBt = 6;
const int resetBt = 5;
const int dispPin = 12;        // link to pixel board (HIGH=STOP, LOW=GO)

// ── Globals ────────────────────────────────────────────────────────
Servo sparkMini;
const uint8_t LED_STOP = LOW;
const uint8_t LED_GO   = HIGH;
volatile long encoderCount = 0;       // updated in encoder ISRs

bool stopRequested  = false;
bool resetRequested = false;
bool sequenceStart  = false;

// ── Motion parameters ─────────────────────────────────────────────
const long  UP_POS    =  5600;  // encoder counts (before sign flip)
const long  DOWN_POS  =  -125;
const int   TOP_DELAY =  5000;  // ms
const int   BOT_DELAY = 7000;  // ms
const float Kp        =  1.0;   // proportional gain
const float Kf        =  -0.175;
const int   BAND      =  200;     // dead-band (± counts)

enum State { IDLE, MOVE_UP, TOP_WAIT, MOVE_DOWN, BOT_WAIT };
State state = IDLE;
unsigned long stateStart = 0;

// ── Helpers ───────────────────────────────────────────────────────
inline void neutral() { sparkMini.writeMicroseconds(1500); }

// Encoder direction depends on wiring; adjust if needed
void encoderA() { encoderCount += (digitalRead(encB) ? -1 :  1); }
void encoderB() { encoderCount += (digitalRead(encA) ?  1 : -1); }

/* ----- Scan manual buttons (debounced 20 ms) and service dispPin ---- */
void scanButtons()
{
  static uint32_t dStop=0, dStart=0, dReset=0;
  uint32_t now = millis();
  if (!digitalRead(stopBt))  { if (now-dStop  >20) stopRequested  = true;  dStop  = now; }
  if (!digitalRead(resetBt)) { if (now-dReset >20) resetRequested = true;  dReset = now; }
  if (!digitalRead(startBt)) { if (now-dStart >20) sequenceStart  = true;  dStart = now; }
}

/* ----- Closed-loop move with real-time button detection & debug ----- */
void moveTo(long target)
{
  target = -target;                               // gearbox inversion
  unsigned long lastPrint = 0;

  while (abs(target - encoderCount) > BAND) {
    scanButtons();
    if (stopRequested) break;

    long err  = target - encoderCount;
    int  pwm  = constrain(1500 + int(Kp * err + Kf * 1000), 1000, 2000); //constraining the pwm output to between 1000 to 2000 and also adding pidf with kf to counteract
    sparkMini.writeMicroseconds(pwm);

    if (millis() - lastPrint >= 200) {            // debug every 200 ms
      Serial.print("Target: "); Serial.print(target);
      Serial.print("  Pos: ");  Serial.print(encoderCount);
      Serial.print("  Err: ");  Serial.print(err);
      Serial.print("  PWM: ");  Serial.println(pwm);
      lastPrint = millis();
    }
    delay(5);                                     // 200 Hz loop
  }
  neutral();
}

// ── Setup ──────────────────────────────────────────────────────────
void setup()
{
  Serial.begin(9600);

  sparkMini.attach(pwmPin);
  pinMode(encA, INPUT_PULLUP);
  pinMode(encB, INPUT_PULLUP);
  pinMode(stopBt,  INPUT_PULLUP);
  pinMode(startBt, INPUT_PULLUP);
  pinMode(resetBt, INPUT_PULLUP);

  /* Pixel-display link */
  pinMode(dispPin, OUTPUT);
  digitalWrite(dispPin, LED_STOP);                  // red while climbing
  attachInterrupt(digitalPinToInterrupt(encA), encoderA, RISING);
  attachInterrupt(digitalPinToInterrupt(encB), encoderB, RISING);

  neutral();  delay(2000);            // arm ESC
  Serial.println(F("Elevator ready – press START"));
}

// ── Main loop ─────────────────────────────────────────────────────
void loop()
{
  scanButtons();                      // always sample first

  /* -- Emergency STOP ------------------------------------------------ */
  if (stopRequested) {
    neutral();
    state = IDLE;
    stopRequested = false;
    Serial.println(F("Stopped by user."));
  }

  /* -- Encoder reset -------------------------------------------------- */
  if (resetRequested)
{
  // 1. Drive to the mechanical bottom *with feedback working*
  moveTo(DOWN_POS);       
  // 2. When the motor has stopped, freeze counts and re-zero
  noInterrupts();
  encoderCount = 0;            // establish new datum
  interrupts();

  Serial.println(F("Encoder reset at bottom."));
  resetRequested = false;
}

  /* -- State machine -------------------------------------------------- */
  switch (state) {

    case IDLE:
      if (sequenceStart) { 
        sequenceStart = false; 
        state = MOVE_UP; 
        }
      break;

    case MOVE_UP:                          // step 1
      Serial.println(F("Moving up → 5000"));
      digitalWrite(dispPin, LED_STOP);                  // red while climbing
      delay(3000);
      encoderCount = 0;//reset move up to zero bc sometimes encoder codes doesnt go back down and it messes up
                       //but its a trade off bc if you press stop then start it will assume where you stopped is new 0 so dont do that
      moveTo(UP_POS);
      if (!stopRequested) { 
        stateStart = millis(); 
        state = TOP_WAIT; 
      }
      break;

    case TOP_WAIT:                         // step 2
      if (millis() - stateStart >= TOP_DELAY) state = MOVE_DOWN;
      break;

    case MOVE_DOWN:                        // step 3
      Serial.println(F("Moving down → 0"));
      moveTo(DOWN_POS);
      if (!stopRequested) {
        stateStart = millis();
        state = BOT_WAIT;                  // go straight to bottom wait
      }
      break;

    case BOT_WAIT:                         // step 4
      // Serial.println("switch led to green");
      digitalWrite(dispPin, LED_GO);
      // LED is already green; just watch the timer
      if (millis() - stateStart >= BOT_DELAY) {
        state = MOVE_UP;                   // step 5: repeat
      }
      break;
  }
}
