/* Simon Says Game
Game flashes LEDS in a pattern which the player repeats using four buttons.
 
*/

int RedButton = 2;
int RedLED = 3;
int GreenButton= 4;
int GreenLED= 5;

int YellowButton = 6;
int YellowLED = 7;
int BlueButton = 8;
int BlueLED = 9;

int buzzerPin = 10;

// Buttons
int buttons[] = {RedButton, GreenButton, YellowButton, BlueButton}; //{2, 4, 6, 8}

// Leds
int leds[] = {RedLED, GreenLED, YellowLED, BlueLED}; //{3, 5, 7, 9}

// Buzzer 
int frequencies[] = {196,294,349,494};

// Game settings
int roundCounter = 0;
bool gamerStartFlag = false;

int buttonSequenceList[16]; // The squence that the player need to remember.
int numRoundsToWin = 10; /// number of guesses or rounds the player has to try to play to win the game.
long timeLimit = 10000; // ms time limit to hit a button

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  pinMode(RedButton, INPUT_PULLUP);
  pinMode(GreenButton, INPUT_PULLUP);
  pinMode(YellowButton, INPUT_PULLUP);
  pinMode(BlueButton, INPUT_PULLUP);
  
  pinMode(GreenLED, OUTPUT);
  pinMode(YellowLED, OUTPUT);
  pinMode(RedLED, OUTPUT);
  pinMode(BlueLED, OUTPUT);

  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:

  // Start the game if the game hasn't started yet
  gameStart(gamerStartFlag);
  
  int arraySize = sizeof(buttons) / sizeof(buttons[0]);
  int noButtonPressed = arraySize; // with 4 buttons this should return 4

  //In each round, show the correct sequence and then compare the input
  roundSequence(roundCounter, arraySize);

}
// 
// 

//BUTTONS
// Set the correct button sequence to guess
void setCorrectSequenceList() {

  randomSeed(analogRead(A0)); // sets the seed of random numbers to a floating pin. 
  int arrayLen = sizeof(buttons) / sizeof(buttons[0]);
  
  for(int i = 0; i < 3+1; i++){
    int randomIndex = rand() % arrayLen;
    buttonSequenceList[i] = buttons[randomIndex];
  }
}

// Read each button sequentially
// Return the index value
int readAllButtons() {
  int arraySize = sizeof(buttons) / sizeof(buttons[0]);
  int outOfRange = arraySize;
  for(int i= 0; i<arraySize; i++){
    if (digitalRead(buttons[i])== LOW) {return i;}
  }
  return arraySize; // value for no button being pressed 4
}

// LEDS
//FLASH LED
void setLED (int ledNumber, bool state) {
  Serial.print("Setting LED number: "); Serial.print(ledNumber); Serial.print(" To state: "); Serial.println(state);
  if (state == 0){
    digitalWrite(leds[ledNumber], LOW);
    delay(100);
  }
  if (state == 1){
    digitalWrite(leds[ledNumber], HIGH);
    delay(100);
  }
  // tone(buzzerPin, tones[ledNumber]);
}

// Set all LEDS off or on
void setAllLED(bool state){
  int length = sizeof(leds) / sizeof(leds[0]); //should be 4
  if (state == 0){
    for (int i = 0; i < length; i++) {
      digitalWrite(leds[i], LOW);
      delay(100);
    }
  }
  else if (state == 1) {
    for (int i = 0; i < length; i++) {
      digitalWrite(leds[i], HIGH);
      delay(100);
    }
  }

  // Serial.print("All LEDS set to: ");
  // Serial.println(state);

}

void showCorrectSequenceList() {
  int arrayLen = sizeof(buttonSequenceList) / sizeof(buttonSequenceList[0]);
  for (int i = 0; i<arrayLen; i++){
    setLED(buttonSequenceList[i], 1);
    delay(500);
    setLED(buttonSequenceList[i], 0);
    delay(500);
  }
}

// BUZZER
// plays happy noises
void happyjingle () {
  //play the 1Up noise
  tone(buzzerPin, 1318, 150);   //E6
  delay(175);
  tone(buzzerPin, 1567, 150);   //G6
  delay(175);
  tone(buzzerPin, 2637, 150);   //E7
  delay(175);
  tone(buzzerPin, 2093, 150);   //C7
  delay(175);
  tone(buzzerPin, 2349, 150);   //D7
  delay(175);
  tone(buzzerPin, 3135, 500);   //G7
  delay(500);

}

// GAME Logic
// Innitialize the game
void gameStart (bool gamerBool) {
  if (gamerBool == false){
    int arraySize = sizeof(buttons) / sizeof(buttons[0]);
    roundCounter = 0;
    delay(1000);           // wait 1s
    if (startSquence(arraySize) == 1) {
      gamerStartFlag = true;
    }
  }
  Serial.print("gameStart: gamerStartFlag is:");
  Serial.println(gamerStartFlag);
}

void gameStop (bool gamerBool) {
  if (gamerBool == 1) {
      Serial.println("Stopping now. But not actually ;)");
  }
}

// Use & to reference original value
void roundSequence (int& roundNum, int& lenArray){
  Serial.print("roundSequence: gamerStartFlag is:");
  Serial.println(gamerStartFlag);
  //each round, start by flashing out the sequence to be repeated
  setAllLED(0); // make sure we start from a clean state
  delay(200);
  
  Serial.print("The Correct Button Sequence: ");
  Serial.print("{");
  for (int i = 0; i < lenArray; i++){
    Serial.print(buttonSequenceList[i]);
    Serial.print(" ,");
  }
  Serial.println("}");

  Serial.print("Going through buttonSequenceList upto the current round number: ");
  Serial.println(roundNum);
  for (int i = 0; i <= roundNum; i++) { //go through the array up to the current round number
    Serial.println(buttonSequenceList[i]);

    setLED(buttonSequenceList[i], 1);     //turn on the LED for that array position and play the sound
    delay(500);                           //wait long enough for the player to see it
    setLED(buttonSequenceList[i], 0);     //turn all of the LEDs off
    delay(500);
  }
  setAllLED(0); // Turns all leds off
  //then start going through the sequence one at a time and see if the user presses the correct button
  compareSquences(roundNum, lenArray);

}

// Goes through the sequence to see if the user presses the correct button
// one at a time
void compareSquences(int& roundNum, int& lenArray) {
  Serial.print("compareSquences: gamerStartFlag is:");
  Serial.println(gamerStartFlag);

  for(int i =0; i<=roundNum; i++){
    Serial.print("Round Number: ");
    Serial.println(roundNum);

    int startTime = millis();                 //record the start time

    //main logic loop for the game
    //loop until the player presses a button or the time limit is up (the time limit check is in an if statement)
    while (gamerStartFlag == true) { //loop until the player presses a button or games ends
      //check to see which button is pressed
      int noPressedButton = lenArray;
      int pressedButton = readAllButtons(); // max value pressedButton should be 3
      
      if (pressedButton < noPressedButton){  //if a button is pressed... (4 means that no button is pressed)

        Serial.print("The pressedButton is: ");
        Serial.println(pressedButton);

        setLED(pressedButton, 1);
        delay(500);

        if (pressedButton == buttonSequenceList[i]) { //if the button matches the button in the sequence
          setLED(pressedButton, 0);                       //then turn off all of the lights and
          delay(500);                             //freez LED in current state
          break;
        } else {
          loseSequence();
          break;
        }
      } else { // If no button is pressed turn all LEDs off
        setAllLED(0); 
      }

      //check to see if the time limit is up
      if (millis() - startTime > timeLimit) { //if the time limit is up
        loseSequence();                       //play the lose sequence
        break;                                //break here to start the game over
      }
    }

    setAllLED(0);
  }

  if (gamerStartFlag == true) {
    roundNum = roundNum + 1;      //increase the round number by 1

    if (roundNum >= numRoundsToWin) {    //if the player has gotten to the final round
      winSequence(lenArray);                      //play the winning sequence
    }
    delay(500);                           //wait for half a second between rounds
  }
}

// Start the game squence
int startSquence(int lenArray){

  //Set buttonSequenceList array with random numbers from 0 to 3
  setCorrectSequenceList();

  //flash all of the LEDs when the game starts
  setAllLED(0); // Turns all leds off
  delay(100);
  setAllLED(1); // Turns all leds on
  delay(100);
  
  setAllLED(0); // Turns all leds off
  delay(100);
  setAllLED(1); // Turns all leds on
  delay(100);
  setAllLED(0); // leave LEDs off so the round sequence is visible
  delay(500);

  return 1;
}

void winSequence(int lenArray) {
  // turn all LEDs on
  setAllLED(1);
  happyjingle();
  //wait for button press
  int pressedButton;

  do {
    pressedButton = readAllButtons();
  } while (pressedButton > lenArray-1);

  delay(200);
  gamerStartFlag = false;   //reset the game so that the start sequence will play again.

}

void loseSequence() {
  // turn all LEDs on
  setAllLED(1);
  happyjingle();
  //wait for button press
  int lenArray = sizeof(buttons) / sizeof(buttons[0]);
  int pressedButton;
  do {
    pressedButton = readAllButtons();
  } while (pressedButton > lenArray-1);
  delay(200);
  gamerStartFlag = false;   //reset the game so that the start sequence will play again.

}


