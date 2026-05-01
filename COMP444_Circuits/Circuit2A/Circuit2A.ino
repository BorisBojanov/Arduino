

int BuzzerPin = 10;

void setup() {
  // put your setup code here, to run once:
  pinMode(BuzzerPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  // SuperMarioTheme();
  // RickRoll();
  TetrisThemeKorobeiniki();
  // PiratesOfTheCaribian();
}

/* CHART OF FREQUENCIES FOR NOTES IN C MAJOR
  Note      Frequency (Hz)
  c        131
  d        147
  e        165
  f        175
  g        196
  a        220
  b        247
  C        262
  D        294
  E        330
  F        349
  G        392
  A        440
  B        494
*/

void play(char note, int beats) {


  // //this array is used to look up the notes
  // char notes[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C', 'D', 'E', 'F', 'G', 'A', 'B', ' '};
  // //this array matches frequencies with each letter (e.g. the 4th note is 'f', the 4th frequency is 175)
  // int frequencies[] = {131, 147, 165, 175, 196, 220, 247, 262, 294, 330, 349, 392, 440, 494, 0};
  
  //                              s=F#
  char notes[] = {'c','d','e','f','s','g','a','b','C','D','E','F','G','A','B',' '};
  int frequencies[] = {131,147,165,175,185,196,220,247,262,294,330,349,370,392,440,494};
  
  
  int len = sizeof(notes) / sizeof(notes[0]);
  // int len2 = sizeof(frequencies) /  sizeof(frequencies[0]);
  // Serial.print("length of Notes Array: ");
  // Serial.print(len);
  // Serial.print(" |  length of Frequencies Array:  ");
  // Serial.println(len2);

  int numNotes = len - 1; // 16 notes in the array, but the array starts at 0 


  int currentFrequency = 0;
  int noteLength = 150;

  //look up the frequency that corresponds to the note
  for (int i = 0; i < numNotes; i++ ){
    if (notes[i] == note){
      currentFrequency = frequencies[i];
    }
  }

  tone(BuzzerPin, currentFrequency, beats * noteLength);
  delay(beats * noteLength);
  delay(50);

}

void RickRoll(){
  //VERSE1
  // Intro
  // "We're no strangers to love"
  // Intro riff
  play('A', 1); play('A', 1); play('B', 1); play('A', 1);
  play('F', 2); play('F', 2);
  play('A', 1); play('A', 1); play('B', 1); play('A', 1);
  play('G', 2); play('G', 2);

  //Verse melody
  play('A', 1); play('A', 1); play('B', 1); play('A', 1);
  play('G', 2); play('F', 1); play('E', 1);
  play('D', 2); play('D', 1); play('E', 1);
  play('F', 1); play('F', 1); play('E', 2);
  play('D', 2);


  //CHORUS
  play('D', 1); play('E', 1); play('G', 2); play('E', 2);
  play('B', 2); play('B', 1); play('A', 3);
  play(' ', 1); // rest
  play('D', 1); play('E', 1); play('G', 2); play('E', 2);
  play('A', 2); play('A', 1); play('G', 3);
  play(' ', 1);
  play('D', 1); play('E', 1); play('G', 2); play('E', 2);
  play('G', 2); play('A', 1); play('F', 1); play('E', 1); play('D', 1);


}

void SuperMarioTheme(){
  // Main Theme - Super Mario Bros (transposed to C major)
  
  // === MAIN THEME ===
  play('E', 1); play('E', 1); play(' ', 1); play('E', 1);
  play(' ', 1); play('C', 1); play('E', 1); play(' ', 1);
  play('G', 2); play(' ', 2); play('g', 2);

  // === FIRST PHRASE ===
  play('C', 1); play(' ', 1); play('g', 1); play(' ', 1);
  play('e', 1); play(' ', 1); play('a', 1); play('b', 1);
  play(' ', 1); play('A', 1); play('a', 1); play(' ', 1);
  play('g', 1); play('E', 1); play('G', 1);
  play('A', 1); play('F', 1); play('G', 1);
  play(' ', 1); play('E', 1); play('C', 1);
  play('D', 1); play('B', 1);

  // === REPEAT PHRASE ===
  play('C', 1); play(' ', 1); play('g', 1); play(' ', 1);
  play('e', 1); play(' ', 1); play('a', 1); play('b', 1);
  play(' ', 1); play('A', 1); play('a', 1); play(' ', 1);
  play('g', 1); play('E', 1); play('G', 1);
  play('A', 1); play('F', 1); play('G', 1);
  play(' ', 1); play('E', 1); play('C', 1);
  play('D', 1); play('B', 1);

  // === UNDERGROUND / FAST RUN SECTION ===
  play('G', 1); play('F#', 1); play('F', 1); play('D', 1);
  play('E', 1); play(' ', 1); play('g', 1); play('a', 1);
  play('C', 1); play(' ', 1); play('a', 1); play('C', 1); play('D', 1);

  play('G', 1); play('F#', 1); play('F', 1); play('D', 1);
  play('E', 1); play(' ', 1); play('C', 2); play('C', 1); play('C', 1);

  play('G', 1); play('F#', 1); play('F', 1); play('D', 1);
  play('E', 1); play(' ', 1); play('g', 1); play('a', 1);
  play('C', 1); play(' ', 1); play('a', 1); play('C', 1); play('D', 1);

  play('D', 1); play('C', 2);

  // === STAR / INVINCIBLE SECTION ===
  play('C', 1); play('C', 2); play('C', 1); play('C', 1);
  play('C', 1); play('D', 1); play(' ', 1);
  play('D', 1); play('D', 2); play('D', 1); play('D', 1);
  play('D', 1); play('E', 1); play(' ', 1);
  play('E', 1); play('E', 2); play('E', 1); play('E', 1);
  play('E', 1); play('E', 1); play(' ', 1);
  play('C', 1); play('E', 1); play('G', 2); play('G', 2);

  // === ENDING PHRASE ===
  play('C', 1); play(' ', 1); play('g', 1); play(' ', 1);
  play('e', 1); play('a', 1); play('F#', 1); play('G', 1);
  play('E', 1); play('C', 1); play('D', 1); play('B', 1);

  // Repeat
  delay(500);
}

void TetrisThemeKorobeiniki() {


  delay(500); // pause before repeating
}


void PiratesOfTheCaribian(){
  play('e', 1); play('g', 1); play('a', 2); play('a', 1); play(' ', 1);
  play('a', 1); play('b', 1); play('C', 2); play('C', 1); play(' ', 1);
  play('C', 1); play('D', 1); play('b', 2); play('b', 1); play(' ', 1);
  play('a', 1); play('g', 1); play('a', 3);  play(' ', 1);

  play('e', 1); play('g', 1); play('a', 2); play('a', 1); play(' ', 1);
  play('a', 1); play('b', 1); play('C', 2); play('C', 1); play(' ', 1);
  play('C', 1); play('D', 1); play('b', 2); play('b', 1); play(' ', 1);
  play('a', 1); play('g', 1); play('a', 3); play(' ', 1);

  play('e', 1); play('g', 1); play('a', 2); play('a', 1); play(' ', 1);
  play('a', 1); play('C', 1); play('D', 2); play('D', 1); play(' ', 1);
  play('D', 1); play('E', 1); play('F', 2); play('F', 1); play(' ', 1);
  play('E', 1); play('D', 1); play('E', 1); play('A', 2); play(' ', 1);

  play('a', 1); play('b', 1); play('C', 2); play('C', 1); play(' ', 1);
  play('D', 2); play('E', 1); play('a', 2);  play(' ', 1);
  play('a', 1); play('C', 1); play('b', 2); play('b', 1); play(' ', 1);
  play('C', 1); play('a', 1); play('b', 3); play(' ', 3);

  play('a', 2); play('a', 1);
  //Repeat of first part
  play('a', 1); play('b', 1); play('C', 2); play('C', 1); play(' ', 1);
  play('a', 1); play('C', 1); play('b', 2); play('b', 1); play(' ', 1);
  play('C', 1); play('a', 1); play('b', 3); play(' ', 1);

  play('e', 1); play('g', 1); play('a', 2); play('a', 1); play(' ', 1);
  play('a', 1); play('b', 1); play('C', 2); play('C', 1); play(' ', 1);
  play('C', 1); play('D', 1); play('b', 2); play('b', 1); play(' ', 1);
  play('a', 1); play('g', 1); play('a', 3); play(' ', 1);

  play('e', 1); play('g', 1); play('a', 2); play('a', 1); play(' ', 1);
  play('a', 1); play('C', 1); play('D', 2); play('D', 1); play(' ', 1);
  play('D', 1); play('E', 1); play('F', 2); play('F', 1); play(' ', 1);
  play('E', 1); play('D', 1); play('E', 1); play('A', 2); play(' ', 1);

  play('a', 1); play('b', 1); play('C', 2); play('C', 1); play(' ', 1);
  play('D', 2); play('E', 1); play('a', 2);  play(' ', 1);
  play('a', 1); play('C', 1); play('b', 2); play('b', 1); play(' ', 1);
  play('C', 1); play('a', 1); play('b', 3); play(' ', 3);

  play('E', 2); play(' ', 1); play(' ', 3); play('F', 2); play(' ', 1); play(' ', 3); 
  play('E', 1); play('E', 1); play(' ', 1); play('G', 1); play(' ', 1); play('E', 1); play('D', 1); play(' ', 1); play(' ', 3);
  play('E', 2); play(' ', 1); play(' ', 3); play('F', 2); play(' ', 1); play(' ', 3); 
  play('b', 1); play('C', 1); play(' ', 1); play('b', 1); play(' ', 1); play('a', 4);

  play('E', 2); play(' ', 1); play(' ', 3); play('F', 2); play(' ', 1); play(' ', 3); 
  play('E', 1); play('E', 1); play(' ', 1); play('G', 1); play(' ', 1); play('E', 1); play('D', 1); play(' ', 1); play(' ', 3);
  play('E', 2); play(' ', 1); play(' ', 3); play('F', 2); play(' ', 1); play(' ', 3); 
  play('b', 1); play('C', 1); play(' ', 1); play('b', 1); play(' ', 1); play('a', 4);






}