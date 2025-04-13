#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// IR sensor pins (Top-Left, Top-Right, Bottom-Right, Bottom-Left)
const int IR_PINS[] = {4, 16, 17, 5};
bool lastState[4] = {HIGH, HIGH, HIGH, HIGH};
unsigned long lastChangeTime = 0;

// Circle detection variables
int sequence[10] = {-1};
unsigned long lastSeqUpdate = 0;

// Function declarations
void detectGestures(bool state[4]);
void updateSequence(int triggeredSensor);
void detectCircles();
bool containsSequence(int a, int b, int c, int d);
void showGestureOLED(String gesture);

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 4; i++) {
    pinMode(IR_PINS[i], INPUT_PULLUP);
  }

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Starting...");
  display.display();
  delay(1000);
}

void loop() {
  bool currentState[4];

  // Read IR sensor states
  for (int i = 0; i < 4; i++) {
    currentState[i] = digitalRead(IR_PINS[i]);
  }

  // Check for changes
  for (int i = 0; i < 4; i++) {
    if (currentState[i] != lastState[i]) {
      lastChangeTime = millis();
      lastState[i] = currentState[i];

      if (currentState[i] == LOW) {
        updateSequence(i);  // i=0:TL, 1:TR, 2:BR, 3:BL
      }

      detectGestures(currentState);
    }
  }

  // Detect circles
  detectCircles();

  // Detect "No Movement" hover gesture
  if (millis() - lastChangeTime > 1500) {
    Serial.println("GESTURE: No Movement");
    showGestureOLED("No Movement");
    lastChangeTime = millis();
  }

  delay(50);
}

void updateSequence(int triggeredSensor) {
  for (int i = 0; i < 9; i++) {
    sequence[i] = sequence[i + 1];
  }
  sequence[9] = triggeredSensor;
  lastSeqUpdate = millis();
}

void detectCircles() {
  if (millis() - lastSeqUpdate > 500) return;

  // Clockwise (BL -> TL -> TR -> BR)
  if (containsSequence(3, 0, 1, 2)) {
    Serial.println("GESTURE: Circle CW");
    showGestureOLED("Circle CW");
    memset(sequence, -1, sizeof(sequence));
    return;
  }

  // Counter-clockwise (BL -> BR -> TR -> TL)
  if (containsSequence(3, 2, 1, 0)) {
    Serial.println("GESTURE: Circle CCW");
    showGestureOLED("Circle CCW");
    memset(sequence, -1, sizeof(sequence));
    return;
  }
}

bool containsSequence(int a, int b, int c, int d) {
  for (int i = 0; i < 7; i++) {
    if (sequence[i] == a &&
        sequence[i + 1] == b &&
        sequence[i + 2] == c &&
        sequence[i + 3] == d) {
      return true;
    }
  }
  return false;
}

void detectGestures(bool state[4]) {
  // Up: Top sensors triggered
  if (!state[0] && !state[1] && state[2] && state[3]) {
    Serial.println("GESTURE: Up");
    showGestureOLED("Up");
    return;
  }
  // Down: Bottom sensors triggered
  if (state[0] && state[1] && !state[2] && !state[3]) {
    Serial.println("GESTURE: Down");
    showGestureOLED("Down");
    return;
  }
  // Left: Left sensors triggered
  if (!state[0] && state[1] && state[2] && !state[3]) {
    Serial.println("GESTURE: Left");
    showGestureOLED("Left");
    return;
  }
  // Right: Right sensors triggered
  if (state[0] && !state[1] && !state[2] && state[3]) {
    Serial.println("GESTURE: Right");
    showGestureOLED("Right");
    return;
  }
  // Diagonal Gestures
  if (!state[0] && state[2] && state[1] && !state[3]) {
    Serial.println("GESTURE: Right-Diagonal");
    showGestureOLED("Right-Diag");
    return;
  }
  if (state[0] && !state[1] && !state[2] && state[3]) {
    Serial.println("GESTURE: Left-Diagonal");
    showGestureOLED("Left-Diag");
    return;
  }
  if (state[0] && !state[1] && !state[2] && !state[3]) {
    Serial.println("GESTURE: Right-Triangle");
    showGestureOLED("Right-Tri");
    return;
  }
  if (!state[0] && state[1] && !state[2] && !state[3]) {
    Serial.println("GESTURE: Left-Triangle");
    showGestureOLED("Left-Tri");
    return;
  }
}

void showGestureOLED(String gesture) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("Gesture:");
  display.setTextSize(2);
  display.setCursor(0, 30);
  display.println(gesture);
  display.display();
}
