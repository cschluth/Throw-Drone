// ============================================
// THROW DETECTION SYSTEM
// XIAO ESP32-S3
// Drone catch-itself project
// ============================================

// --- PIN DEFINITIONS ---
// NEW - correct for XIAO ESP32-S3
#define BUTTON_PIN    D1    // button on drone body
#define LED_PIN       LED_BUILTIN  // onboard LED
#define FC_TX_PIN     D6    // UART TX to flight controller
#define FC_RX_PIN     D7    // UART RX from flight controller

// --- TUNING CONSTANTS ---
// These values will need adjustment during real world testing
// Start with these defaults and tune from there

#define FREEFALL_THRESHOLD    0.3   // g-force below this = freefall
                                    // 1.0 = normal gravity
                                    // 0.3 = significant freefall detected

#define THROW_ACCEL_THRESHOLD 1.8   // g-force above this = throw motion
                                    // detects the acceleration spike of a throw

#define STABILIZE_TIMEOUT     2000  // milliseconds to attempt stabilization
                                    // before giving up and returning to idle

#define DEBOUNCE_DELAY        50    // milliseconds for button debounce
                                    // prevents false triggers from button noise

// --- STATE MACHINE DEFINITIONS ---
// Drone can only be in one state at a time
// States progress in order during a throw sequence

#define STATE_IDLE          0  // sitting still, waiting for button press
#define STATE_HELD          1  // button held, motors at idle, ready to throw
#define STATE_RELEASED      2  // button just released, checking for throw
#define STATE_FREEFALL      3  // throw confirmed, waiting for tumble detection
#define STATE_STABILIZING   4  // sending stabilize command to flight controller

// --- GLOBAL VARIABLES ---
int currentState = STATE_IDLE;
unsigned long stateEntryTime = 0;    // tracks when we entered current state
unsigned long lastDebounceTime = 0;  // tracks last button state change
bool lastButtonState = HIGH;         // previous button reading
bool buttonState = HIGH;             // current confirmed button state

// Simulated IMU values for bench testing
// Replace with real IMU reads when FC telemetry is connected
float simulated_accel_x = 1.0;
float simulated_accel_y = 0.0;
float simulated_accel_z = 0.0;

// --- UART TO FLIGHT CONTROLLER ---
HardwareSerial FC_Serial(1);  // use UART1 for FC communication

// ============================================
// MSP PROTOCOL FUNCTIONS
// MSP = MultiWii Serial Protocol
// How ESP32 talks to Betaflight
// ============================================

// Calculate checksum for MSP packet
// XOR all bytes together
uint8_t calculateChecksum(uint8_t* data, int length) {
  uint8_t checksum = 0;
  for (int i = 0; i < length; i++) {
    checksum ^= data[i];
  }
  return checksum;
}

// Send MSP command to flight controller
// command = MSP command code
// data = optional payload bytes
// dataLength = number of payload bytes
void sendMSPCommand(uint8_t command, 
                    uint8_t* data = nullptr, 
                    int dataLength = 0) {
  
  // MSP packet structure:
  // $ M < dataLength command [data bytes] checksum
  
  uint8_t packet[dataLength + 6];
  packet[0] = '$';          // header byte 1
  packet[1] = 'M';          // header byte 2
  packet[2] = '<';          // direction: to FC
  packet[3] = dataLength;   // payload size
  packet[4] = command;      // command code
  
  // copy data payload if any
  for (int i = 0; i < dataLength; i++) {
    packet[5 + i] = data[i];
  }
  
  // calculate and append checksum
  packet[5 + dataLength] = calculateChecksum(
    &packet[3], dataLength + 2
  );
  
  // send over UART to flight controller
  FC_Serial.write(packet, sizeof(packet));
}

// Tell Betaflight to enter angle mode and stabilize
// This is the key command sent after throw detection
void sendStabilizeCommand() {
  // MSP_SET_RAW_RC = 200
  // Sends fake RC input to FC telling it to stabilize
  // Channel values: roll, pitch, yaw, throttle
  // 1500 = center/neutral for roll pitch yaw
  // 1100 = low throttle to start, FC manages from here
  
  uint8_t channels[16];
  
  // Roll: center (1500)
  channels[0] = 0xDC;
  channels[1] = 0x05;
  
  // Pitch: center (1500)
  channels[2] = 0xDC;
  channels[3] = 0x05;
  
  // Yaw: center (1500)
  channels[4] = 0xDC;
  channels[5] = 0x05;
  
  // Throttle: low (1100) — FC handles the rest
  channels[6] = 0x4C;
  channels[7] = 0x04;
  
  // remaining channels set to 1000 (off)
  for (int i = 8; i < 16; i++) {
    channels[i] = 0;
  }
  
  sendMSPCommand(200, channels, 16);
  
  Serial.println(">> Stabilize command sent to FC");
}

// Tell Betaflight to arm motors
void sendArmCommand() {
  // Arming is done by setting throttle low
  // and yaw full right briefly
  // then returning to center
  // Betaflight handles the actual arm sequence
  
  Serial.println(">> Arm command sent to FC");
  // Full implementation added when FC arrives
  // and we can test the exact arm sequence
}

// ============================================
// IMU FUNCTIONS
// Currently simulated for bench testing
// Replace with real MSP telemetry reads
// when flight controller is connected
// ============================================

// Read total acceleration magnitude from IMU
// Returns value in g-force units
// 1.0g = stationary on ground
// ~0.0g = freefall
// >1.5g = significant acceleration (throw)
float readAccelMagnitude() {
  
  // ---- SIMULATION MODE ----
  // Remove this block when FC is connected
  // and replace with real MSP telemetry read
  
  // Simulate different conditions for testing:
  // Comment/uncomment to test different scenarios
  
  return 1.0;   // normal gravity — stationary
  // return 0.2;   // freefall simulation
  // return 2.1;   // throw acceleration simulation
  
  // ---- END SIMULATION ----
  
  // REAL IMPLEMENTATION (uncomment when FC arrives):
  // sendMSPCommand(102);  // MSP_RAW_IMU = 102
  // wait for response
  // parse acceleration values
  // return calculated magnitude
}

// Detect if drone is in freefall
// Called immediately after button release
bool detectFreefall() {
  float accel = readAccelMagnitude();
  Serial.print("Accel magnitude: ");
  Serial.println(accel);
  return accel < FREEFALL_THRESHOLD;
}

// Detect throw acceleration spike
// Called to confirm this was a throw not just a drop
bool detectThrowAccel() {
  // In real implementation reads peak acceleration
  // from the moment of throw
  // For now simulated as always true after button release
  return true;
}

// ============================================
// LED STATUS FUNCTIONS
// Visual feedback for each drone state
// ============================================

void ledIdle() {
  // Slow single blink — waiting
  static unsigned long lastBlink = 0;
  static bool ledOn = false;
  
  if (millis() - lastBlink > 1000) {
    ledOn = !ledOn;
    digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
    lastBlink = millis();
  }
}

void ledHeld() {
  // Solid on — button held, ready to throw
  digitalWrite(LED_PIN, HIGH);
}

void ledReleased() {
  // Fast blink — processing throw detection
  static unsigned long lastBlink = 0;
  static bool ledOn = false;
  
  if (millis() - lastBlink > 100) {
    ledOn = !ledOn;
    digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
    lastBlink = millis();
  }
}

void ledStabilizing() {
  // Very fast blink — motors spinning up
  static unsigned long lastBlink = 0;
  static bool ledOn = false;
  
  if (millis() - lastBlink > 50) {
    ledOn = !ledOn;
    digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
    lastBlink = millis();
  }
}

// ============================================
// BUTTON READING WITH DEBOUNCE
// Debounce prevents false triggers from
// electrical noise on button press/release
// ============================================

// Returns true if button is currently held
// Returns false if button is released
bool readButton() {
  bool reading = digitalRead(BUTTON_PIN);
  
  // if reading changed, reset debounce timer
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  // only accept reading if stable for DEBOUNCE_DELAY
  if (millis() - lastDebounceTime > DEBOUNCE_DELAY) {
    buttonState = reading;
  }
  
  lastButtonState = reading;
  
  // button is active LOW (pressed = LOW)
  // because of PULL_UP resistor
  return buttonState == LOW;
}

// ============================================
// STATE MACHINE
// Core logic of the throw detection system
// Each state has entry behavior and 
// transition conditions to next state
// ============================================

void enterState(int newState) {
  currentState = newState;
  stateEntryTime = millis();
  
  // Print state transition for debugging
  switch (newState) {
    case STATE_IDLE:
      Serial.println("STATE: Idle — waiting for button");
      break;
    case STATE_HELD:
      Serial.println("STATE: Held — ready to throw");
      break;
    case STATE_RELEASED:
      Serial.println("STATE: Released — detecting throw");
      break;
    case STATE_FREEFALL:
      Serial.println("STATE: Freefall — throw confirmed");
      break;
    case STATE_STABILIZING:
      Serial.println("STATE: Stabilizing — catching drone");
      break;
  }
}

void updateStateMachine() {
  bool buttonHeld = readButton();
  
  switch (currentState) {
    
    // ---- IDLE ----
    // Waiting for button to be pressed
    case STATE_IDLE:
      ledIdle();
      if (buttonHeld) {
        enterState(STATE_HELD);
      }
      break;
    
    // ---- HELD ----
    // Button is held — drone in hand ready to throw
    // Motors at idle in Betaflight
    // Waiting for button release
    case STATE_HELD:
      ledHeld();
      if (!buttonHeld) {
        // Button released — start throw detection
        enterState(STATE_RELEASED);
      }
      break;
    
    // ---- RELEASED ----
    // Button just released
    // Check for freefall within short window
    // If no freefall detected → return to idle
    // (prevents accidental trigger from just setting drone down)
    case STATE_RELEASED:
      ledReleased();
      
      if (detectFreefall()) {
        // Freefall detected — this was a real throw
        enterState(STATE_FREEFALL);
      }
      
      // If no freefall detected within 500ms
      // assume drone was just set down, return to idle
      if (millis() - stateEntryTime > 500) {
        Serial.println("No throw detected — returning to idle");
        enterState(STATE_IDLE);
      }
      break;
    
    // ---- FREEFALL ----
    // Throw confirmed by freefall detection
    // Now waiting for tumble/orientation data
    // Then send stabilize command
    case STATE_FREEFALL:
      ledReleased();
      
      // Small delay to let drone fully clear hand
      // Then immediately command stabilization
      if (millis() - stateEntryTime > 100) {
        enterState(STATE_STABILIZING);
      }
      break;
    
    // ---- STABILIZING ----
    // Send stabilize command to flight controller
    // FC takes over and catches the drone
    case STATE_STABILIZING:
      ledStabilizing();
      sendArmCommand();
      sendStabilizeCommand();
      
      // After stabilize timeout return to idle
      // FC should have drone hovering by now
      if (millis() - stateEntryTime > STABILIZE_TIMEOUT) {
        Serial.println("Stabilization complete");
        enterState(STATE_IDLE);
      }
      break;
  }
}

// ============================================
// SETUP AND LOOP
// ============================================

void setup() {
  // Initialize serial monitor for debugging
  Serial.begin(115200);
  Serial.println("=====================================");
  Serial.println("Throw Detection System Starting...");
  Serial.println("=====================================");
  
  // Initialize pins
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  // Initialize UART to flight controller
  FC_Serial.begin(115200, SERIAL_8N1, 
                  FC_RX_PIN, FC_TX_PIN);
  Serial.println("FC Serial initialized");
  
  // Start in idle state
  enterState(STATE_IDLE);
  
  Serial.println("System ready");
  Serial.println("Hold button and throw to test");
}

void loop() {
  // Run state machine every loop iteration
  // This executes hundreds of times per second
  // Fast enough to catch any throw motion
  updateStateMachine();
  
  // Small delay to prevent overwhelming serial monitor
  // Remove this when timing becomes critical
  delay(10);
}