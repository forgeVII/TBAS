#include <Arduino.h>
#include <Servo.h>
#include <IRremote.hpp>
#include <math.h>

// ================== PIN DEFINITIONS ==================
#define IR_RECV     4
#define IR_LED      5
#define MIC1        2
#define MIC2        3
#define BTN_LEFT    8
#define BTN_RIGHT   9
#define SERVO_PIN   12
#define RGB_R       A3
#define RGB_G       A4
#define RGB_B       A5
#define TRIG        6
#define ECHO        7

// ================== CONSTANTS ==================
#define SERVO_MIN      25
#define SERVO_MAX      155
#define IFF_KEY        7
#define IFF_TIMEOUT    150
#define RESULT_HOLD    3000
#define LOCKOUT_MS     5000
#define DOUBLE_CLK_MS  500
#define MIC_SILENCE_US 500000UL

// ================== GLOBALS ==================
Servo myServo;

int angle = 90;
int iffCounter = 0;

enum Mode { MANUAL, SURVEILLANCE, SERVO_TEST, ACOUSTIC };
enum TXMode { FRIEND, ENEMY };

Mode currentMode = MANUAL;
TXMode txMode = FRIEND;

enum ManualState { ACO_WAITING, BTN_CONTROL };
ManualState manualState = ACO_WAITING;

volatile unsigned long mic1Time = 0;
volatile unsigned long mic2Time = 0;
volatile bool mic1Hit = false;
volatile bool mic2Hit = false;

const char* modeName(Mode mode) {
  switch (mode) {
    case MANUAL: return "MANUAL";
    case SURVEILLANCE: return "SURVEILLANCE";
    case SERVO_TEST: return "SERVO_TEST";
    case ACOUSTIC: return "ACOUSTIC";
  }
  return "UNKNOWN";
}

const char* txName(TXMode mode) {
  return (mode == FRIEND) ? "FRIEND" : "ENEMY";
}

void sendStatus() {
  Serial.print("STATUS: MODE=");
  Serial.print(modeName(currentMode));
  Serial.print(" TX=");
  Serial.print(txName(txMode));
  Serial.print(" ANGLE=");
  Serial.print(angle);
  Serial.print(" IFF=");
  Serial.println(iffCounter);
}

// ================== ISR ==================
void isr1() {
  if (micros() - mic1Time > MIC_SILENCE_US) {
    mic1Time = micros();
    mic1Hit = true;
  }
}

void isr2() {
  if (micros() - mic2Time > MIC_SILENCE_US) {
    mic2Time = micros();
    mic2Hit = true;
  }
}

// ================== RGB ==================
void setRGB(bool r, bool g, bool b) {
  digitalWrite(RGB_R, r);
  digitalWrite(RGB_G, g);
  digitalWrite(RGB_B, b);
}

void blinkRGB(bool r, bool g, bool b, int times) {
  for (int i = 0; i < times; i++) {
    setRGB(r, g, b);
    delay(200);
    setRGB(0, 0, 0);
    delay(200);
  }
}

void skyBlueBlink() {
  static unsigned long lastBlink = 0;
  static bool state = false;
  if (millis() - lastBlink > 500) {
    state = !state;
    setRGB(0, state, state);
    lastBlink = millis();
  }
}

void setModeColor() {
  if (currentMode == MANUAL) {
    setRGB(1, 0, 1);
  } else if (currentMode == SURVEILLANCE) {
    setRGB(0, 0, 1);
  } else if (manualState == BTN_CONTROL) {
    setRGB(1, 0, 1);
  } else {
    setRGB(0, 0, 0);
  }
}

void enterMode(Mode mode) {
  currentMode = mode;
  manualState = ACO_WAITING;
  setModeColor();
  sendStatus();
}

// ================== ULTRASONIC ==================
int getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 30000);
  int dist = duration * 0.034 / 2;

  return (dist == 0 || dist > 400) ? -1 : dist;
}

// ================== TDOA ==================
int tdoaToAngle(long tdoa_us) {
  float dt = tdoa_us / 1000000.0;
  float sinVal = 343.0 * dt / 0.28;
  if (sinVal > 1.0) sinVal = 1.0;
  if (sinVal < -1.0) sinVal = -1.0;
  float a = asin(sinVal) * 180.0 / 3.14159265 + 90.0;
  if (a < SERVO_MIN) a = SERVO_MIN;
  if (a > SERVO_MAX) a = SERVO_MAX;
  return (int)a;
}

// ================== IFF ==================
void runIFF_blocking() {
  myServo.detach();
  setRGB(0, 0, 1);

  int result = 2;
  unsigned long globalStart = millis();
  
  while (result == 2 && millis() - globalStart < 5000) {
    uint8_t expected = (IFF_KEY * iffCounter + 5) % 256;
    uint8_t sendCode = (txMode == FRIEND) ? expected : (expected + 1) % 256;

    IrSender.sendNEC(0x00, sendCode, 0);
    delay(50);

    unsigned long start = millis();
    while (millis() - start < IFF_TIMEOUT) {
      if (IrReceiver.decode()) {
        int received = IrReceiver.decodedIRData.command;
        IrReceiver.resume();

        if (received == 0) continue;

        if (received == (int)expected) {
          result = 0;
          iffCounter++;
        } else {
          result = 1;
        }
        break;
      }
    }
  }

  int d = getDistance();
  if (result == 0) {
    setRGB(0, 1, 0);
    Serial.print("IFF: FRIEND");
  } else if (result == 1) {
    setRGB(1, 0, 0);
    Serial.print("IFF: ENEMY");
  } else {
    setRGB(0, 0, 1);
    Serial.print("IFF: NO RESPONSE");
  }

  if (d > 0) {
    Serial.print(" DIST:");
    Serial.print(d);
    Serial.println("cm");
  } else {
    Serial.println();
  }

  delay(RESULT_HOLD);

  myServo.attach(SERVO_PIN);
  myServo.write(angle);
  delay(100);
}

void runIFF() {
  myServo.detach();

  uint8_t expected = (IFF_KEY * iffCounter + 5) % 256;
  uint8_t sendCode = (txMode == FRIEND) ? expected : (expected + 1) % 256;

  IrSender.sendNEC(0x00, sendCode, 0);
  delay(50);

  int result = 2;
  unsigned long start = millis();
  while (millis() - start < IFF_TIMEOUT) {
    if (digitalRead(BTN_LEFT) == LOW || digitalRead(BTN_RIGHT) == LOW) {
      myServo.attach(SERVO_PIN);
      myServo.write(angle);
      return;
    }

    if (IrReceiver.decode()) {
      int received = IrReceiver.decodedIRData.command;
      IrReceiver.resume();

      if (received == 0) continue;

      if (received == (int)expected) {
        result = 0;
        iffCounter++;
      } else {
        result = 1;
      }
      break;
    }
  }

  if (result == 0) {
    int d = getDistance();
    setRGB(0, 1, 0);
    Serial.print("IFF: FRIEND");
    if (d > 0) {
      Serial.print(" DIST:");
      Serial.print(d);
      Serial.println("cm");
    } else {
      Serial.println();
    }
    delay(RESULT_HOLD);
  } else if (result == 1) {
    int d = getDistance();
    setRGB(1, 0, 0);
    Serial.print("IFF: ENEMY");
    if (d > 0) {
      Serial.print(" DIST:");
      Serial.print(d);
      Serial.println("cm");
    } else {
      Serial.println();
    }
    delay(RESULT_HOLD);
  }

  myServo.attach(SERVO_PIN);
  myServo.write(angle);
  delay(100);
}

// ================== LOCKOUT ==================
void doLockout() {
  Serial.println("LOCKOUT");
  mic1Hit = false;
  mic2Hit = false;

  unsigned long start = millis();
  while (millis() - start < LOCKOUT_MS) {
    skyBlueBlink();
    delay(10);
  }

  mic1Hit = false;
  mic2Hit = false;
  manualState = ACO_WAITING;
  Serial.println("ACO READY");
}

// ================== ACOUSTIC CHECK ==================
bool checkAcoustic() {
  if (mic1Hit && !mic2Hit) {
    if (micros() - mic1Time > 20000UL) mic1Hit = false;
  }

  if (mic2Hit && !mic1Hit) {
    if (micros() - mic2Time > 20000UL) mic2Hit = false;
  }

  if (mic1Hit && mic2Hit) {
    long tdoa = (long)mic1Time - (long)mic2Time;
    mic1Hit = false;
    mic2Hit = false;

    if (abs(tdoa) <= 816) {
      int targetAngle = tdoaToAngle(tdoa);
      Serial.print("ACO ANGLE: ");
      Serial.println(targetAngle);
      angle = 180 - targetAngle;
      myServo.attach(SERVO_PIN);
      myServo.write(angle);
      delay(500);
      return true;
    } else {
      Serial.print("DISCARDED: ");
      Serial.println(tdoa);
    }
  }

  return false;
}

// ================== BUTTON HANDLING ==================
void checkButtons() {
  bool leftPressed = (digitalRead(BTN_LEFT) == LOW);
  bool rightPressed = (digitalRead(BTN_RIGHT) == LOW);
  bool bothPressed = leftPressed && rightPressed;

  static bool prevBoth = false;
  static unsigned long bothPressStart = 0;
  static bool bothHoldTriggered = false;
  static unsigned long releaseTime = 0;
  static int clickCount = 0;

  if (bothPressed) {
    if (!prevBoth) {
      bothPressStart = millis();
      bothHoldTriggered = false;
    }

    if (!bothHoldTriggered && millis() - bothPressStart > DOUBLE_CLK_MS) {
      Mode nextMode = (Mode)((currentMode + 1) % 4);
      enterMode(nextMode);
      blinkRGB(1, 1, 0, 2);
      setModeColor();

      if (currentMode == MANUAL) {
        Serial.println("MODE: MANUAL");
      } else if (currentMode == SURVEILLANCE) {
        Serial.println("MODE: SURVEILLANCE");
      } else if (currentMode == SERVO_TEST) {
        Serial.println("MODE: SERVO_TEST");
      } else {
        Serial.println("MODE: ACOUSTIC");
      }

      bothHoldTriggered = true;
      clickCount = 0;
    }
  }

  if (!bothPressed && prevBoth) {
    if (!bothHoldTriggered) {
      clickCount++;
      releaseTime = millis();
    }
  }

  if (clickCount > 0 && millis() - releaseTime > DOUBLE_CLK_MS) {
    if (clickCount >= 2) {
      txMode = (txMode == FRIEND) ? ENEMY : FRIEND;

      if (txMode == FRIEND) {
        blinkRGB(0, 1, 0, 2);
        Serial.println("TX: FRIEND");
      } else {
        blinkRGB(1, 0, 0, 2);
        Serial.println("TX: ENEMY");
      }

      setModeColor();
    }

    clickCount = 0;
  }

  if (!bothPressed) {
    bothHoldTriggered = false;
  }

  prevBoth = bothPressed;
}

// ================== SURVEILLANCE ==================
void runSurveillance() {
  int target = random(SERVO_MIN, SERVO_MAX);

  Serial.print("SURV TARGET: ");
  Serial.println(180 - target);

  int step = (target > angle) ? 1 : -1;

  while (angle != target) {
    checkButtons();
    if (currentMode != SURVEILLANCE) return;

    angle += step;
    myServo.write(angle);
    delay(20);
  }

  for (int i = 0; i < 2; i++) {
    checkButtons();
    if (currentMode != SURVEILLANCE) return;

    setRGB(0, 0, 1);
    delay(300);
    setRGB(0, 0, 0);
    delay(300);

    runIFF();
  }

  setModeColor();
}

// ================== SERVO TEST MODE ==================
void runServoTest() {
  if (mic1Hit && !mic2Hit) {
    if (micros() - mic1Time > 20000UL) {
      Serial.println("MIC1 only");
      mic1Hit = false;
    }
  }

  if (mic2Hit && !mic1Hit) {
    if (micros() - mic2Time > 20000UL) {
      Serial.println("MIC2 only");
      mic2Hit = false;
    }
  }

  if (mic1Hit && mic2Hit) {
    long tdoa = (long)mic1Time - (long)mic2Time;
    mic1Hit = false;
    mic2Hit = false;

    if (abs(tdoa) > 816) {
      Serial.print("DISCARDED=");
      Serial.println(tdoa);
    } else {
      int testAngle = tdoaToAngle(tdoa);
      Serial.print("TDOA=");
      Serial.print(tdoa);
      Serial.print("us | Angle=");
      Serial.print(testAngle);
      Serial.println("deg");
      angle = testAngle;
      myServo.write(180 - angle);
    }

    delay(1000);
    mic1Hit = false;
    mic2Hit = false;
  }
}

// ================== ACOUSTIC MODE ==================
void runAcousticMode() {
  bool leftPressed = (digitalRead(BTN_LEFT) == LOW);
  bool rightPressed = (digitalRead(BTN_RIGHT) == LOW);
  bool bothPressed = leftPressed && rightPressed;

  static int holdCount = 0;

  if (!bothPressed && (leftPressed || rightPressed)) {
    manualState = BTN_CONTROL;
    holdCount++;

    int step = (holdCount > 30) ? 3 : 1;

    if (leftPressed && angle < SERVO_MAX) {
      angle += step;
      if (angle > SERVO_MAX) angle = SERVO_MAX;
    }

    if (rightPressed && angle > SERVO_MIN) {
      angle -= step;
      if (angle < SERVO_MIN) angle = SERVO_MIN;
    }

    myServo.write(angle);
    setRGB(1, 0, 1);

    Serial.print("Angle: ");
    Serial.println(angle);

    delay(30);
    return;
  }

  holdCount = 0;

  if (manualState == ACO_WAITING) {
    skyBlueBlink();
    if (checkAcoustic()) {
      runIFF_blocking();
      mic1Hit = false;
      mic2Hit = false;
      manualState = ACO_WAITING;
      Serial.println("ACO READY");
    }
    return;
  }

  if (manualState == BTN_CONTROL && !leftPressed && !rightPressed && !bothPressed) {
    runIFF();
    doLockout();
  }
}

// ================== SETUP ==================
void setup() {
  Serial.begin(9600);

  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);

  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(MIC1, INPUT);
  pinMode(MIC2, INPUT);

  attachInterrupt(digitalPinToInterrupt(MIC1), isr1, RISING);
  attachInterrupt(digitalPinToInterrupt(MIC2), isr2, RISING);

  IrReceiver.begin(IR_RECV);
  IrSender.begin(IR_LED);

  myServo.attach(SERVO_PIN);
  myServo.write(angle);

  delay(500);

  randomSeed(analogRead(A0));

  setModeColor();

  Serial.println("TBAS READY");
  Serial.println("MODE: MANUAL");
  sendStatus();
}

// ================== LOOP ==================
void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "PING") {
      Serial.println("PONG");
      sendStatus();
    } else if (cmd == "STATUS") {
      sendStatus();
    } else if (cmd == "MODE:SURV") {
      enterMode(SURVEILLANCE);
      blinkRGB(1, 1, 0, 2);
      setModeColor();
      Serial.println("MODE: SURVEILLANCE");
    } else if (cmd == "MODE:MANUAL") {
      enterMode(MANUAL);
      blinkRGB(1, 1, 0, 2);
      setModeColor();
      Serial.println("MODE: MANUAL");
    } else if (cmd == "MODE:ACO" || cmd == "MODE:ACOUSTIC") {
      enterMode(ACOUSTIC);
      blinkRGB(1, 1, 0, 2);
      setModeColor();
      Serial.println("MODE: ACOUSTIC");
      Serial.println("ACO READY");
    } else if (cmd == "MODE:TEST" || cmd == "MODE:SERVO_TEST") {
      enterMode(SERVO_TEST);
      blinkRGB(1, 1, 0, 2);
      setModeColor();
      Serial.println("MODE: SERVO_TEST");
      Serial.println("TBAS SERVO TEST READY");
    } else if (cmd == "TX:ENEMY") {
      txMode = ENEMY;
      blinkRGB(1, 0, 0, 2);
      setModeColor();
      Serial.println("TX: ENEMY");
      sendStatus();
    } else if (cmd == "TX:FRIEND") {
      txMode = FRIEND;
      blinkRGB(0, 1, 0, 2);
      setModeColor();
      Serial.println("TX: FRIEND");
      sendStatus();
    } else if (cmd.startsWith("ANGLE:")) {
      int a = cmd.substring(6).toInt();

      if (a >= SERVO_MIN && a <= SERVO_MAX && currentMode == MANUAL) {
        angle = a;
        myServo.attach(SERVO_PIN);
        myServo.write(angle);

        Serial.print("Angle: ");
        Serial.println(angle);
        sendStatus();
      }
    }
  }

  static unsigned long lastHeartbeat = 0;
  if (millis() - lastHeartbeat > 5000UL) {
    sendStatus();
    lastHeartbeat = millis();
  }

  checkButtons();

  if (currentMode == SURVEILLANCE) {
    runSurveillance();
    return;
  }

  if (currentMode == SERVO_TEST) {
    runServoTest();
    return;
  }

  if (currentMode == ACOUSTIC) {
    runAcousticMode();
    return;
  }

  bool leftPressed = (digitalRead(BTN_LEFT) == LOW);
  bool rightPressed = (digitalRead(BTN_RIGHT) == LOW);
  bool bothPressed = leftPressed && rightPressed;

  if (!bothPressed && (leftPressed || rightPressed)) {
    static int holdCount = 0;
    holdCount++;

    int step = (holdCount > 30) ? 3 : 1;

    if (leftPressed && angle < SERVO_MAX) {
      angle += step;
      if (angle > SERVO_MAX) angle = SERVO_MAX;
    }

    if (rightPressed && angle > SERVO_MIN) {
      angle -= step;
      if (angle < SERVO_MIN) angle = SERVO_MIN;
    }

    myServo.write(angle);
    setRGB(1, 0, 1);

    Serial.print("Angle: ");
    Serial.println(angle);

    delay(30);
    return;
  }

  if (!leftPressed && !rightPressed) {
    runIFF();
  }
}