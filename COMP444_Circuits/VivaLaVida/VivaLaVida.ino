int BuzzerPin = 10;

void setup() {
  pinMode(BuzzerPin, OUTPUT);
}

void loop() {
  VivaLaVida();
  delay(2000); // pause before repeat
}

/* -----------------------------------------------
   VIVA LA VIDA - Coldplay
   Transposed from Ab major → C major
   BPM ~138  |  noteLength = 150ms = 1 beat unit

   Note map (from your play() function):
     c=131  d=147  e=165  f=175  s=F#185
     g=196  a=220  b=247
     C=262  D=294  E=330  F=349  G=392  A=440  B=494
     ' '=rest

   play(note, beats) uses beats * 150ms per note
   play(note, beats, durationMs) uses custom duration
   ----------------------------------------------- */

// ── Helper: rest ────────────────────────────────
void rest(int beats) {
  delay(beats * 150);
  delay(50);
}

// ── INTRO / STRING RIFF (repeats throughout) ────
// The iconic 4-bar loop: C  G  A  F
// Played as a descending-then-rising arpeggio line
void introRiff() {
  // Bar 1  (C chord feel)
  play('E', 1); play('D', 1); play('C', 2);
  // Bar 2  (G chord feel)
  play('G', 1); play('G', 1); play('B', 1); play('G', 1);
  // Bar 3  (A minor feel)
  play('A', 1); play('A', 1); play('C', 1); play('A', 1);
  // Bar 4  (F chord feel)
  play('F', 1); play('G', 1); play('A', 2);
}

// ── VERSE MELODY ────────────────────────────────
// "I used to rule the world / Seas would rise when I gave the word"
void verseMelody() {
  // Line 1: "I used to rule the world"
  play('E', 2); play('D', 1); play('C', 1);
  play('G', 2); play('A', 1); play('G', 1);

  // Line 2: "Seas would rise when I gave the word"
  play('F', 1); play('G', 1);
  play('A', 2); play('G', 1); play('F', 1);
  play('E', 1); play('D', 1); play('C', 2);

  // Line 3: "Now in the morning I sleep alone"
  play('C', 1); play('D', 1); play('E', 1); play('F', 1);
  play('G', 2); play('A', 1); play('G', 1);

  // Line 4: "Sweep the streets I used to own"
  play('F', 1); play('G', 1);
  play('A', 2); play('G', 1); play('F', 1);
  play('E', 1); play('D', 1); play('C', 2);
}

// ── PRE-CHORUS ───────────────────────────────────
// "I used to roll the dice / Feel the fear in my enemy's eyes"
void preChorus() {
  // "I used to roll the dice"
  play('G', 1); play('A', 1); play('B', 1); play('A', 1);
  play('G', 2); play('E', 2);

  // "Feel the fear in my enemy's eyes"
  play('E', 1); play('F', 1); play('G', 1); play('A', 1);
  play('B', 2); play('A', 1); play('G', 1);

  // "Listen as the crowd would sing"
  play('F', 1); play('G', 1); play('A', 1); play('B', 1);
  play('C', 2); play('B', 1); play('A', 1); // high C

  // "Now the old king is dead long live the king"
  play('G', 1); play('A', 1); play('B', 1); play('A', 1);
  play('G', 2); play('E', 2);
}

// ── CHORUS ──────────────────────────────────────
// "I hear Jerusalem bells a-ringing / Roman cavalry choirs are singing"
void chorus() {
  // "I hear Jerusalem bells a-ringing"
  play('C', 1); play('E', 1); play('G', 1); play('A', 1);
  play('G', 2); play('E', 1); play('D', 1);

  // "Roman cavalry choirs are singing"
  play('C', 1); play('E', 1); play('G', 1); play('A', 1);
  play('G', 2); play('E', 1); play('D', 1);

  // "Be my mirror my sword and shield"
  play('F', 1); play('G', 1); play('A', 1); play('G', 1);
  play('F', 2); play('E', 1); play('D', 1);

  // "My missionaries in a foreign field"
  play('C', 1); play('D', 1); play('E', 1); play('F', 1);
  play('G', 2); play('A', 2);

  // "For some reason I can't explain"
  play('G', 1); play('A', 1); play('B', 1); play('A', 1);
  play('G', 2); play('E', 2);

  // "Once you go there was never / Never an honest word"
  play('F', 1); play('G', 1); play('A', 1); play('B', 1);
  play('A', 2); play('G', 1); play('F', 1);

  // "And that was when I ruled the world"
  play('E', 1); play('F', 1); play('G', 1); play('A', 1);
  play('G', 4);
}

// ── BRIDGE ──────────────────────────────────────
// "It was the wicked and wild wind..."
void bridge() {
  // "It was the wicked and wild wind"
  play('E', 1); play('G', 1); play('A', 1); play('G', 1);
  play('E', 2); play('D', 2);

  // "Blew down the doors to let me in"
  play('C', 1); play('D', 1); play('E', 1); play('F', 1);
  play('G', 2); play('A', 2);

  // "Shattered windows and the sound of drums"
  play('G', 1); play('A', 1); play('B', 1); play('A', 1);
  play('G', 2); play('E', 1); play('D', 1);

  // "People couldn't believe what I'd become"
  play('F', 1); play('G', 1); play('A', 1); play('B', 1);
  play('C', 2); play('B', 1); play('A', 1);

  // "Revolutionaries wait"
  play('G', 1); play('A', 1); play('G', 1); play('F', 1);
  play('E', 2); play('D', 2);

  // "For my head on a silver plate"
  play('C', 1); play('D', 1); play('E', 1); play('F', 1);
  play('G', 2); play('A', 2);

  // "Just a puppet on a lonely string"
  play('G', 1); play('A', 1); play('B', 1); play('A', 1);
  play('G', 2); play('E', 2);

  // "Oh who would ever want to be king?"
  play('F', 1); play('G', 1); play('A', 1); play('B', 1);
  play('C', 4);
}

// ── FULL SONG ────────────────────────────────────
void VivaLaVida() {

  // ── INTRO (riff x2) ──
  introRiff();
  introRiff();

  // ── VERSE 1 ──
  verseMelody();

  // ── PRE-CHORUS ──
  preChorus();

  // ── CHORUS ──
  chorus();

  // ── RIFF (between sections) ──
  introRiff();

  // ── VERSE 2 ──
  // "I used to roll the dice..." (same melody, different lyrics)
  verseMelody();

  // ── PRE-CHORUS ──
  preChorus();

  // ── CHORUS ──
  chorus();

  // ── BRIDGE ──
  bridge();

  // ── CHORUS x2 (final) ──
  chorus();
  chorus();

  // ── OUTRO (riff fades) ──
  introRiff();
  introRiff();
}

/* -----------------------------------------------
   NOTE FREQUENCIES (your original chart)
   c=131  d=147  e=165  f=175  s(F#)=185
   g=196  a=220  b=247
   C=262  D=294  E=330  F=349  G=392  A=440  B=494
   ' '=rest (0 Hz)
   ----------------------------------------------- */

void play(char note, int beats) {
  int numNotes = 15;
  char notes[]       = {'c','d','e','f','s','g','a','b','C','D','E','F','G','A','B',' '};
  int  frequencies[] = {131,147,165,175,185,196,220,247,262,294,330,349,392,440,494,  0};

  int currentFrequency = 0;
  int noteLength = 150;

  for (int i = 0; i < numNotes; i++) {
    if (notes[i] == note) {
      currentFrequency = frequencies[i];
    }
  }

  tone(BuzzerPin, currentFrequency, beats * noteLength);
  delay(beats * noteLength);
  delay(50);
}
