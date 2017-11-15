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
// Current state (0=Closed, 1=Open)
int currentState = 1;
// Open lock state
String openLockState = "open";
// Closed lock state
String closeLockState = "close";
// Current lock state
String lockState = "close";

void setup()
{
   // Register toggleLock function to make it accessible from the cloud
   Particle.function("toggleLock",toggleLock);
   // Register lockState variable to make it accessible from the cloud
   Particle.variable("lockState",&lockState,STRING);
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
    if (digitalRead(dIn) == LOW && currentState == 0){
        changeLockState();
        currentState = 1;
    }
    else if (digitalRead(dIn) == HIGH && currentState == 1){
        changeLockState();
        currentState = 0;
    }
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

void openStateLED() {
    digitalWrite(openLED,HIGH);
    digitalWrite(closedLED,LOW);
}

void closeStateLED() {
    digitalWrite(openLED,LOW);
    digitalWrite(closedLED,HIGH);
}

void toggleLEDs(String lockState) {
    if (lockState == openLockState)
        openStateLED();
    else if (lockState == closeLockState)
        closeStateLED();
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
    delay(1000);
    // Detach servo to prevent buzzing
    gateKeeper.detach();
    return 200;
}