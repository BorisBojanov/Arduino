// Digital Trumpet
int GreenButton = 2;
int YellowButton = 3;
int RedButton = 4;

int buzzerPin = 10;

int green   = 0;
int yellow  = 0;
int red     = 0;
//                              s=F#
char notes[] = {'c','d','e','f','s','g','a','b','C','D','E','F','G','A','B',' '};
int frequencies[] = {131,147,165,175,185,196,220,247,262,294,330,349,370,392,440,494};

int noteCombos[][3] = {
  // {0,0,0} no buttons = no tone
  {1,0,0},  // index 0 -> 'c'
  {0,1,0},  // index 1 -> 'd'
  {1,1,0},  // index 2 -> 'e'
  {0,0,1},  // index 3 -> 'f'
  {1,0,1},  // index 4 -> 's' (F#)
  {0,1,1},  // index 5 -> 'g'
  {1,1,1},  // index 6 -> 'a'
};

int trumpetState[3] = {0, 0, 0};  // {green, yellow, red}

int findNote(int* trumpetState) {
  int numCombos = sizeof(noteCombos) / sizeof(noteCombos[0]);
  
  for (int i = 0; i < numCombos; i++) {
    if (memcmp(trumpetState, noteCombos[i], 3 * sizeof(int)) == 0) {
      return i; // return the matching index
    }
  }
  return -1; // no match
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(buzzerPin,    OUTPUT);
  pinMode(GreenButton,  INPUT_PULLUP);
  pinMode(YellowButton, INPUT_PULLUP);
  pinMode(RedButton,    INPUT_PULLUP);

}

void loop() {
  // put your main code here, to run repeatedly:
  
  // INPUT_PULLUP is LOW when pressed.
  trumpetState[0] = !digitalRead(GreenButton);
  trumpetState[1] = !digitalRead(YellowButton);
  trumpetState[2] = !digitalRead(RedButton);
  
  int noteIndex = findNote(trumpetState);
  
  if (noteIndex != -1) {
    tone(buzzerPin, frequencies[noteIndex]);
    Serial.print("Playing: ");
    Serial.println(notes[noteIndex]);
  } else {
    noTone(buzzerPin);
  }
  delay(50); // small debounce delay
}

