// -----------------------------------
// Controlling Locks over the Internet
// -----------------------------------

// Servo
Servo gateKeeper;
// Locked Position
int lockedPos=90;
// Unlocked Position
int unlockedPos=180;
// Analog Output Pin
int analogOutput = A7;
// Closed LED
int closedLED = D3;
// Open LED
int openLED = D4;
// Digital Input Pin for Switch
int dIn = D1;
// Piezo Speaker Pin
int spkr = D6;
// Current state - used for manually toggling lock state
int currentState = 1;
// Open lock state
String openLockState = "open";
// Closed lock state
String closeLockState = "close";
// Current lock state
String lockState = "close";
// Timestamp of last time sync with Particle
long lastSync = millis();
// Timestamp of last lock state change
long lastLockStateChange = millis();
// Timestamp of last lock state reminder
long lastLockStateReminder = millis();
// Constant representing five seconds in milliseconds
long FIVE_SEC_MILLIS = (5 * 1000);
// Constant representing five minutes in milliseconds
long FIVE_MIN_MILLIS = (5 * 60 * 1000);
// Constant representing one day in milliseconds
long ONE_DAY_MILLIS = (24 * 60 * 60 * 1000);

void setup()
{
   // Turn off Bluz LED to conserve battery power
   RGB.control(true);
   RGB.color(0, 0, 0);
   // Register toggleLock function to make it accessible from the cloud
   Particle.function("toggleLock",toggleLock);
   // Register lockState variable to make it accessible from the cloud
   Particle.variable("lockState",lockState);
   // Set LED, dIn, & Piezo Pin Modes
   pinMode(closedLED,OUTPUT);
   pinMode(openLED,OUTPUT);
   pinMode(dIn,INPUT_PULLDOWN);
   pinMode(spkr,OUTPUT);
   // Start with our servo in the locked position by default
   closeLock();
}

void loop()
{
    // Request time synchronization from the Particle Cloud once a day
    if (millis() - lastSync > ONE_DAY_MILLIS) {
        Particle.syncTime();
        lastSync = millis();
    }
    // Flash one of the LEDs as a lock state reminder every five seconds
    if (millis() - lastLockStateReminder > FIVE_SEC_MILLIS) {
        if (lockState == openLockState) {
            flashOpenStateLED();
        }
        else {
            flashCloseStateLED();
        }
    }
    // Auto-lock after five minutes
    if (lockState == openLockState && millis() - lastLockStateChange > FIVE_MIN_MILLIS) {
        toggleLock(closeLockState);
    }
    else if ((digitalRead(dIn) == LOW && currentState == 0) || (digitalRead(dIn) == HIGH && currentState == 1)){
        changeCurrentState();
        changeLockState();
    }
    // Place CPU in sleep mode to conserve battery power
    System.sleep(SLEEP_MODE_CPU);
}

void beep() {
    digitalWrite(spkr,HIGH);
    delay(100);
    digitalWrite(spkr,LOW);
}

void beepTwice() {
    beep();
    delay(50);
    beep();
}

void flashOpenStateLED() {
    digitalWrite(openLED,HIGH);
    delay(100);
    digitalWrite(openLED,LOW);
    lastLockStateReminder = millis();
}

void flashCloseStateLED() {
    digitalWrite(closedLED,HIGH);
    delay(100);
    digitalWrite(closedLED,LOW);
    delay(50);
    digitalWrite(closedLED,HIGH);
    delay(100);
    digitalWrite(closedLED,LOW);
    lastLockStateReminder = millis();
}

void toggleLEDs(String lockState) {
    if (lockState == openLockState)
        flashOpenStateLED();
    else if (lockState == closeLockState)
        flashCloseStateLED();
}

void openLock() {
    // Open Sesame
    gateKeeper.write(unlockedPos);
    toggleLEDs(openLockState);
    beep();
    lockState = openLockState;
}

void closeLock() {
    // Close Sesame
    gateKeeper.write(lockedPos);
    toggleLEDs(closeLockState);
    beepTwice();
    lockState = closeLockState;
}

void changeLockState() {
    if (lockState == openLockState)
        toggleLock(closeLockState);
    else if (lockState == closeLockState)
        toggleLock(openLockState);
}

void changeCurrentState() {
    if (currentState == 0)
        currentState = 1;
    else if (currentState == 1)
        currentState = 0;
}

// This function will be called through an API request to toggle the lock's position

int toggleLock(String command) {
    /* Spark.functions always take a string as an argument and return an integer.
    Since we can pass a string, it means that we can give the program commands on how the function should be used.
    In this case, telling the function "open" will move our servo to the unlocked position and telling it "close" will move it to the locked position.
    Then, the function returns a value to us to let us know what happened.
    In this case, it will return 200 for all valid arguments
    and -1 if we received a totally bogus command that didn't do anything to our servo.
    */
    if (command==openLockState || command==closeLockState)
    {
        // Attach servo to analog pin A0
        gateKeeper.attach(analogOutput);   
    }
    if (command==openLockState)
        openLock();
    else if (command==closeLockState)
        closeLock();
    else
        return -1; // Bad command
    // Update last lock state change timestamp
    lastLockStateChange = millis();
    delay(1000);
    // Detach servo to prevent buzzing
    gateKeeper.detach();
    return 200;
}