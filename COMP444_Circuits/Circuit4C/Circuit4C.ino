

#include <LiquidCrystal.h>            //the liquid crystal library contains commands for printing to the display
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);     // tell the RedBoard what pins are connected to the display

int buttonPin = 2;                    //pin that the button is connected to
int buzzerPin = 6;                    //pin for driving the buzzer
int buttonPressTime = 0;              //variable to show how much time the player has left to guess the word (and press the button)

long timeLimit = 15000;               //time limit for the player to guess each word
long startTime = 0;                   //used to measure time that has passed for each word
int roundNumber = 0;                        //keeps track of the roundNumber so that it can be displayed at the end of the game
const int arraySize = 25;

const char* words[arraySize] = {"giraffe", "wallrus", "bear", "goose", "dog", "cat", "squirrel", "bird", "elephant", "horse",
                                "octopus", "giraffe", "seal", "bat", "skunk", "turtle", "whale", "rhino", "lion", "monkey",
                                "frog", "Donkey", "Loin-seal", "hippo", "hair"
                               };

// the start value in the sequence array must have a value that could never be an index of an array
// or at least a value outside the range of 0 to the size of the words array (- 1) in this case, it can't be between 0 to 24
int sequence[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}; //start with an array full of -1's

// Note Frequencies (Hz)
#define NOTE_C5  523
#define NOTE_G5  784
#define NOTE_C6  1047
#define NOTE_E6  1319
#define NOTE_G6  1568
#define NOTE_C7  2093

void setup() {

  pinMode(buttonPin, INPUT_PULLUP);       //set the button pin as an input

  lcd.begin(16, 2);                       //tell the LCD library the size of the screen

  generateRandomOrder();                  //generate an array of random numbers from 0 to 24 that will determine which order the words are shown in

  showStartSequence();                    //print the start sequence text

}

void loop() {

  for (int i = 0; i < arraySize; i++) {          //for each of the 25 words in the sequence

    lcd.clear();                        //clear off the array

    roundNumber = i + 1;                    //the array starts at 0, but the roundNumber will start counting from 1
    lcd.print(roundNumber);                 //print the roundNumber (this is the current round number)
    lcd.print(": ");                        //spacer between the number and the word
    lcd.print(words[sequence[i]]);        //print a random word from the word array

    startTime = millis();  //record the time that this round started

    while (digitalRead(buttonPin) == HIGH) {         //do this until the button is pressed...

      int roundedTime = round((timeLimit - (millis() - startTime)) / 1000); //calculate the time left in the round (dividing by 1000 converts the number to seconds
      lcd.setCursor(14, 1);                                                 //set the cursor in the lower right corner of the screen
      lcd.print("  ");
      lcd.setCursor(14, 1);                                                 //set the cursor in the lower right corner of the screen
      lcd.print(roundedTime);                                               //print the time left in the time limit
      delay(15);

      if (millis() - startTime > timeLimit) {       //if the time limit is up before the button is pressed
        gameOver();                                 //end the game
      }

      if (digitalRead(buttonPin) == LOW) {
        tone(buzzerPin, 272, 10);                   //emit a short beep when the button is pressed
      }

    }                       //exit this loop when the button is pressed

    delay(500);             //delay for a moment before going onto the next round, so that the button press doesn't get registered twice

  }
  //if you finish all 25 words
  winner();   //show the you win message
}


//--------------FUNCTIONS------------------------------

//DISPLAYS A COUNTDOWN TO START THE GAME
void showStartSequence() {

  lcd.clear();                  //clear the screen

  lcd.setCursor(0, 0);          //move the cursor to the top left corner
  lcd.print("Category:");       //print "Category:"

  lcd.setCursor(0, 1);          //move the cursor to the bottom left corner
  lcd.print("Animals");         //print "Animals:"

  delay(2000);                  //Wait 2 seconds

  lcd.clear();                  //clear the screen
  lcd.print("Get ready!");      //print "Get ready!"
  delay(1000);                  //wait 1 second

  lcd.clear();                  //clear the screen
  lcd.print("3");               //print "3"
  delay(1000);                  //wait 1 second

  lcd.clear();                  //clear the screen
  lcd.print("2");               //print "3"
  delay(1000);                  //wait 1 second

  lcd.clear();                  //clear the screen
  lcd.print("1");               //print "3"
  delay(1000);                  //wait 1 second
}

//GENERATES A RANDOM ORDER FOR THE WORDS TO BE DISPLAYED
void generateRandomOrder() {

  randomSeed(analogRead(0));            //reset the random seed (Arduino needs this to generate truly random numbers

  for (int i = 0; i < arraySize; i++) {        //do this until all 25 positions are filled

    int currentNumber = 0;              //variable to hold the current number
    boolean match = false;              //does the currentNumber match any of the previous numbers?

    //generate random numbers until you've generated one that doesn't match any of the other numbers in the array
    do {
      currentNumber = random(0, arraySize);            //generate a random number from 0 to 24
      match = false;                            //we haven't checked for matches yet, so start by assuming that it doesn't match
      for (int i = 0; i < arraySize; i++) {            //for all 25 numbers in the array
        if (currentNumber == sequence[i]) {     //does the currentNumber match any of the numbers?
          match = true;                         //if so, set the match variable to true
        }
      }
    } while (match == true);                    //if the match variable is true, generate another random number and try again
    sequence[i] = currentNumber;                //if the match variable is false (the new number is unique) then add it to the sequence
  }
}

//GAME OVER
void gameOver() {
  lcd.clear();                    //clear the screen

  lcd.setCursor(0, 0);            //move the cursor the top left corner
  lcd.print("Game Over");         //print "Game Over"

  lcd.setCursor(0, 1);            //move to the bottom row
  lcd.print("Score: ");           //print a label for the score
  lcd.print(roundNumber-1);       //print the score (the score is equal to the previous level/round number)

  //play the losing fog horn
  tone(buzzerPin, 130, 250);      //E6
  delay(275);
  tone(buzzerPin, 73, 250);       //G6
  delay(275);
  tone(buzzerPin, 65, 150);       //E7
  delay(175);
  tone(buzzerPin, 98, 500);       //C7
  delay(500);
  superMarioTheme();
  while (true) {}                 //get stuck in this loop forever
}

//WINNER
void winner() {
  lcd.clear();                    //clear the screen

  lcd.setCursor(7, 0);            //move the cursor to the top center of the screen
  lcd.print("YOU");               //print "You"

  lcd.setCursor(7, 1);            //move the cursor to the bottom center of the screen
  lcd.print("WIN!");              //print "WIN!"

  playLevelUp();
  //play the 1Up noise
  // tone(buzzerPin, 1318, 150);     //E6
  // delay(175);
  // tone(buzzerPin, 1567, 150);     //G6
  // delay(175);
  // tone(buzzerPin, 2637, 150);     //E7
  // delay(175);
  // tone(buzzerPin, 2093, 150);     //C7
  // delay(175);
  // tone(buzzerPin, 2349, 150);     //D7
  // delay(175);
  // tone(buzzerPin, 3135, 500);     //G7
  // delay(500);

  while (true) {}                 //get stuck in this loop forever
}

void playLevelUp() {
  // Sequence of notes for the 1-Up/Level Up sound
  int melody[] = {
    NOTE_C5, NOTE_G5, NOTE_C6, NOTE_E6, NOTE_G6, NOTE_C7
  };

  // Note durations: 100ms each for a fast sound
  int noteDuration = 100;

  for (int i = 0; i < 6; i++) {
    tone(buzzerPin, melody[i], noteDuration);
    
    // Pause between notes
    delay(noteDuration * 1.1);
  }
  
  noTone(buzzerPin);
}

void superMarioTheme(){
  /*
    Super Mario Bros - Overworld theme 
    Connect a piezo buzzer or speaker to pin 11 or select a new pin.
    More songs available at https://github.com/robsoncouto/arduino-songs         
  */
  #define NOTE_B0  31
  #define NOTE_C1  33
  #define NOTE_CS1 35
  #define NOTE_D1  37
  #define NOTE_DS1 39
  #define NOTE_E1  41
  #define NOTE_F1  44
  #define NOTE_FS1 46
  #define NOTE_G1  49
  #define NOTE_GS1 52
  #define NOTE_A1  55
  #define NOTE_AS1 58
  #define NOTE_B1  62
  #define NOTE_C2  65
  #define NOTE_CS2 69
  #define NOTE_D2  73
  #define NOTE_DS2 78
  #define NOTE_E2  82
  #define NOTE_F2  87
  #define NOTE_FS2 93
  #define NOTE_G2  98
  #define NOTE_GS2 104
  #define NOTE_A2  110
  #define NOTE_AS2 117
  #define NOTE_B2  123
  #define NOTE_C3  131
  #define NOTE_CS3 139
  #define NOTE_D3  147
  #define NOTE_DS3 156
  #define NOTE_E3  165
  #define NOTE_F3  175
  #define NOTE_FS3 185
  #define NOTE_G3  196
  #define NOTE_GS3 208
  #define NOTE_A3  220
  #define NOTE_AS3 233
  #define NOTE_B3  247
  #define NOTE_C4  262
  #define NOTE_CS4 277
  #define NOTE_D4  294
  #define NOTE_DS4 311
  #define NOTE_E4  330
  #define NOTE_F4  349
  #define NOTE_FS4 370
  #define NOTE_G4  392
  #define NOTE_GS4 415
  #define NOTE_A4  440
  #define NOTE_AS4 466
  #define NOTE_B4  494
  #define NOTE_C5  523
  #define NOTE_CS5 554
  #define NOTE_D5  587
  #define NOTE_DS5 622
  #define NOTE_E5  659
  #define NOTE_F5  698
  #define NOTE_FS5 740
  #define NOTE_G5  784
  #define NOTE_GS5 831
  #define NOTE_A5  880
  #define NOTE_AS5 932
  #define NOTE_B5  988
  #define NOTE_C6  1047
  #define NOTE_CS6 1109
  #define NOTE_D6  1175
  #define NOTE_DS6 1245
  #define NOTE_E6  1319
  #define NOTE_F6  1397
  #define NOTE_FS6 1480
  #define NOTE_G6  1568
  #define NOTE_GS6 1661
  #define NOTE_A6  1760
  #define NOTE_AS6 1865
  #define NOTE_B6  1976
  #define NOTE_C7  2093
  #define NOTE_CS7 2217
  #define NOTE_D7  2349
  #define NOTE_DS7 2489
  #define NOTE_E7  2637
  #define NOTE_F7  2794
  #define NOTE_FS7 2960
  #define NOTE_G7  3136
  #define NOTE_GS7 3322
  #define NOTE_A7  3520
  #define NOTE_AS7 3729
  #define NOTE_B7  3951
  #define NOTE_C8  4186
  #define NOTE_CS8 4435
  #define NOTE_D8  4699
  #define NOTE_DS8 4978
  #define REST      0


  // change this to make the song slower or faster
  int tempo = 200;

  // change this to whichever pin you want to use

  // notes of the moledy followed by the duration.
  // a 4 means a quarter note, 8 an eighteenth , 16 sixteenth, so on
  // !!negative numbers are used to represent dotted notes,
  // so -4 means a dotted quarter note, that is, a quarter plus an eighteenth!!
  int melody[] = {

    // Super Mario Bros theme
    // Score available at https://musescore.com/user/2123/scores/2145
    // Theme by Koji Kondo
    
    
    NOTE_E5,8, NOTE_E5,8, REST,8, NOTE_E5,8, REST,8, NOTE_C5,8, NOTE_E5,8, //1
    NOTE_G5,4, REST,4, NOTE_G4,8, REST,4, 
    NOTE_C5,-4, NOTE_G4,8, REST,4, NOTE_E4,-4, // 3
    NOTE_A4,4, NOTE_B4,4, NOTE_AS4,8, NOTE_A4,4,
    NOTE_G4,-8, NOTE_E5,-8, NOTE_G5,-8, NOTE_A5,4, NOTE_F5,8, NOTE_G5,8,
    REST,8, NOTE_E5,4,NOTE_C5,8, NOTE_D5,8, NOTE_B4,-4,
    NOTE_C5,-4, NOTE_G4,8, REST,4, NOTE_E4,-4, // repeats from 3
    NOTE_A4,4, NOTE_B4,4, NOTE_AS4,8, NOTE_A4,4,
    NOTE_G4,-8, NOTE_E5,-8, NOTE_G5,-8, NOTE_A5,4, NOTE_F5,8, NOTE_G5,8,
    REST,8, NOTE_E5,4,NOTE_C5,8, NOTE_D5,8, NOTE_B4,-4,

    
    REST,4, NOTE_G5,8, NOTE_FS5,8, NOTE_F5,8, NOTE_DS5,4, NOTE_E5,8,//7
    REST,8, NOTE_GS4,8, NOTE_A4,8, NOTE_C4,8, REST,8, NOTE_A4,8, NOTE_C5,8, NOTE_D5,8,
    REST,4, NOTE_DS5,4, REST,8, NOTE_D5,-4,
    NOTE_C5,2, REST,2,

    REST,4, NOTE_G5,8, NOTE_FS5,8, NOTE_F5,8, NOTE_DS5,4, NOTE_E5,8,//repeats from 7
    REST,8, NOTE_GS4,8, NOTE_A4,8, NOTE_C4,8, REST,8, NOTE_A4,8, NOTE_C5,8, NOTE_D5,8,
    REST,4, NOTE_DS5,4, REST,8, NOTE_D5,-4,
    NOTE_C5,2, REST,2,

    NOTE_C5,8, NOTE_C5,4, NOTE_C5,8, REST,8, NOTE_C5,8, NOTE_D5,4,//11
    NOTE_E5,8, NOTE_C5,4, NOTE_A4,8, NOTE_G4,2,

    NOTE_C5,8, NOTE_C5,4, NOTE_C5,8, REST,8, NOTE_C5,8, NOTE_D5,8, NOTE_E5,8,//13
    REST,1, 
    NOTE_C5,8, NOTE_C5,4, NOTE_C5,8, REST,8, NOTE_C5,8, NOTE_D5,4,
    NOTE_E5,8, NOTE_C5,4, NOTE_A4,8, NOTE_G4,2,
    NOTE_E5,8, NOTE_E5,8, REST,8, NOTE_E5,8, REST,8, NOTE_C5,8, NOTE_E5,4,
    NOTE_G5,4, REST,4, NOTE_G4,4, REST,4, 
    NOTE_C5,-4, NOTE_G4,8, REST,4, NOTE_E4,-4, // 19
    
    NOTE_A4,4, NOTE_B4,4, NOTE_AS4,8, NOTE_A4,4,
    NOTE_G4,-8, NOTE_E5,-8, NOTE_G5,-8, NOTE_A5,4, NOTE_F5,8, NOTE_G5,8,
    REST,8, NOTE_E5,4, NOTE_C5,8, NOTE_D5,8, NOTE_B4,-4,

    NOTE_C5,-4, NOTE_G4,8, REST,4, NOTE_E4,-4, // repeats from 19
    NOTE_A4,4, NOTE_B4,4, NOTE_AS4,8, NOTE_A4,4,
    NOTE_G4,-8, NOTE_E5,-8, NOTE_G5,-8, NOTE_A5,4, NOTE_F5,8, NOTE_G5,8,
    REST,8, NOTE_E5,4, NOTE_C5,8, NOTE_D5,8, NOTE_B4,-4,

    NOTE_E5,8, NOTE_C5,4, NOTE_G4,8, REST,4, NOTE_GS4,4,//23
    NOTE_A4,8, NOTE_F5,4, NOTE_F5,8, NOTE_A4,2,
    NOTE_D5,-8, NOTE_A5,-8, NOTE_A5,-8, NOTE_A5,-8, NOTE_G5,-8, NOTE_F5,-8,
    
    NOTE_E5,8, NOTE_C5,4, NOTE_A4,8, NOTE_G4,2, //26
    NOTE_E5,8, NOTE_C5,4, NOTE_G4,8, REST,4, NOTE_GS4,4,
    NOTE_A4,8, NOTE_F5,4, NOTE_F5,8, NOTE_A4,2,
    NOTE_B4,8, NOTE_F5,4, NOTE_F5,8, NOTE_F5,-8, NOTE_E5,-8, NOTE_D5,-8,
    NOTE_C5,8, NOTE_E4,4, NOTE_E4,8, NOTE_C4,2,

    NOTE_E5,8, NOTE_C5,4, NOTE_G4,8, REST,4, NOTE_GS4,4,//repeats from 23
    NOTE_A4,8, NOTE_F5,4, NOTE_F5,8, NOTE_A4,2,
    NOTE_D5,-8, NOTE_A5,-8, NOTE_A5,-8, NOTE_A5,-8, NOTE_G5,-8, NOTE_F5,-8,
    
    NOTE_E5,8, NOTE_C5,4, NOTE_A4,8, NOTE_G4,2, //26
    NOTE_E5,8, NOTE_C5,4, NOTE_G4,8, REST,4, NOTE_GS4,4,
    NOTE_A4,8, NOTE_F5,4, NOTE_F5,8, NOTE_A4,2,
    NOTE_B4,8, NOTE_F5,4, NOTE_F5,8, NOTE_F5,-8, NOTE_E5,-8, NOTE_D5,-8,
    NOTE_C5,8, NOTE_E4,4, NOTE_E4,8, NOTE_C4,2,
    NOTE_C5,8, NOTE_C5,4, NOTE_C5,8, REST,8, NOTE_C5,8, NOTE_D5,8, NOTE_E5,8,
    REST,1,

    NOTE_C5,8, NOTE_C5,4, NOTE_C5,8, REST,8, NOTE_C5,8, NOTE_D5,4, //33
    NOTE_E5,8, NOTE_C5,4, NOTE_A4,8, NOTE_G4,2,
    NOTE_E5,8, NOTE_E5,8, REST,8, NOTE_E5,8, REST,8, NOTE_C5,8, NOTE_E5,4,
    NOTE_G5,4, REST,4, NOTE_G4,4, REST,4, 
    NOTE_E5,8, NOTE_C5,4, NOTE_G4,8, REST,4, NOTE_GS4,4,
    NOTE_A4,8, NOTE_F5,4, NOTE_F5,8, NOTE_A4,2,
    NOTE_D5,-8, NOTE_A5,-8, NOTE_A5,-8, NOTE_A5,-8, NOTE_G5,-8, NOTE_F5,-8,
    
    NOTE_E5,8, NOTE_C5,4, NOTE_A4,8, NOTE_G4,2, //40
    NOTE_E5,8, NOTE_C5,4, NOTE_G4,8, REST,4, NOTE_GS4,4,
    NOTE_A4,8, NOTE_F5,4, NOTE_F5,8, NOTE_A4,2,
    NOTE_B4,8, NOTE_F5,4, NOTE_F5,8, NOTE_F5,-8, NOTE_E5,-8, NOTE_D5,-8,
    NOTE_C5,8, NOTE_E4,4, NOTE_E4,8, NOTE_C4,2,
    
    //game over sound
    NOTE_C5,-4, NOTE_G4,-4, NOTE_E4,4, //45
    NOTE_A4,-8, NOTE_B4,-8, NOTE_A4,-8, NOTE_GS4,-8, NOTE_AS4,-8, NOTE_GS4,-8,
    NOTE_G4,8, NOTE_D4,8, NOTE_E4,-2,  

  };

  // sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
  // there are two values per note (pitch and duration), so for each note there are four bytes
  int notes = sizeof(melody) / sizeof(melody[0]) / 2;

  // this calculates the duration of a whole note in ms
  int wholenote = (60000 * 4) / tempo;

  int divider = 0, noteDuration = 0;

  // iterate over the notes of the melody.
  // Remember, the array is twice the number of notes (notes + durations)
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(buzzerPin, melody[thisNote], noteDuration * 0.9);

    // Wait for the specief duration before playing the next note.
    delay(noteDuration);

    // stop the waveform generation before the next note.
    noTone(buzzerPin);
  }
}



