// Arda Ozdogru
// Download necessary board (your board or equivalent) from boards manager //NodeMCU 1.0
// Setup OB-XD26 virtual synth, https://www.discodsp.com/obxd/
// loopMIDI, https://www.tobias-erichsen.de/software/loopmidi.html
// hairlessmidi.exe https://projectgus.github.io/hairless-midiserial/

// MIDI commands and info https://ccrma.stanford.edu/~craig/articles/linuxmidi/misc/essenmidi.html

///// First, upload the program to the microprocessor and run OB-XD26 synth.
// (This program only sends 3 bytes and through serial so something in the pc should listen to the port 
// but OB-XD26 expects hardware MIDIport, so loopMIDI creates a virtual MIDI port.
// hairlessMIDI interprets the MIDI messages)
///// run loopMIDI and create a port.
///// run hairlessmidi.exe and connect from the serialport to loopMIDI virtual port
// match hairlessmidi baud rate (File -> preferences) with Arduino


// MIDI commands
// further MIDI commands are given in obxd.pdf
int noteON = 144;//144 = 1001000 in binary, note on command
int noteOFF = 0x80 ; // 128 note off command

int pitchbend = 224;//224 = 11100000 in binary, pitchbend command
int lsb = 0;//least siginificant bit of pitchbend message
int msb = 64;//most significant bit of pitchbend message

// MIDI values
int C = 60;//middle c4
int Cs = 61;//middle C#4
int D = 62;//middle D4
int Ds = 63;//middle D#4
int E = 64;//middle E4
int F = 65;//middle F4
int Fs = 66;//middle F#4
int G = 67;//middle G4
int Gs = 68;//middle G#4
int A = 69;//middle A4
int As = 70;//middle A#4
int B = 71;//middle B4
int oct = 12; // MIDI shift for each octave

int note_current = C; 

//send MIDI message as 3 byte (Check MIDI commands)
void MIDImessage(int command, int data1, int data2) {
  Serial.write(command);//send command byte
  Serial.write(data1);//send data byte #1
  Serial.write(data2);//send data byte #2
}

// Press note
void note_on(int note,int velocity){
  MIDImessage(noteON, note, velocity);//turn note on
  note_current = note; //last pressed note
}

// release note
void note_off(int note, int velocity){
  MIDImessage(noteON, note, velocity);//turn note off
}

// 0-128 full triangle LFO to control MIDI CC 
void lfo(int tool, int value, int vel){

  if (value<(128-vel)){
    value = value + vel;
  }
  else if (value >= 128){
    value = value - vel;
  }
  else if(value <= 0 ){
    value = value + vel;
  }
  else{
    value = value + 1;
  }

  MIDImessage(0xB0,tool ,value);
}

// Pitchbend function to pitchbend to given note from note_current
// for a given sustaintime, with steps of bend_time
// not a complete the pitchbend (sth I did for gliding chords)
void pitchbend_func(int note_to, int bend_time, int sustainTime){
  
  int bend_t = 0;
  int bend_k = 3;
  int msb_to = 0;

    // upbend
    if (note_to > note_current){
      msb_to = 64 + ((note_to - note_current)*bend_k); //calculate msb 
      bend_t = bend_time; // calculate bend timing step
      for (msb=64;msb<=msb_to ;msb++){//decrease pitchbend msb from 64 (no pitchbend) to msb_to;
        MIDImessage(pitchbend, lsb, msb);//send pitchbend message
        delay(bend_t);
      }
    }
  // downbend
    else if (note_to < note_current){
      msb_to = 64 - ((note_current - note_to)*bend_k) ;
      bend_t = bend_time;
      for (msb=64;msb>=msb_to ;msb--){ //decrease pitchbend msb from 64 (no pitchbend) to msb_to;
        MIDImessage(pitchbend, lsb, msb);
        delay(bend_t);
      }
    }
    else{
      delay(bend_t);
    }
    delay(sustainTime);
    note_current = note_to;

}


// play given notes at corresponding durations (simple note on and off)
// give tabs to play, length of notes array is needed and it should be same with durations[]
void bass_line(int notes[],int durations[],int array_size){
  int vel = 0;
  for (int i=0; i<=array_size;i++){
    if (notes[i]!=0){
      vel = random(90,105);
      note_on(notes[i], vel);
      note_current = notes[i];
      delay(durations[i]);
      note_off(notes[i],0); 
    }
    // 0 is for stops
    else{
      delay(durations[i]);
    }
  } 
}

// use with strings or organ kind of long sustained sounds
void gliding_chords(){
  // press chords at random velocity each time
    int vel = random(90, 105);
  note_on(C, vel);
  note_on(Ds,vel);
  note_on(G, vel);
  delay(500);
  // Pitchbend to create melody
  pitchbend_func(G+3,100,900);
  pitchbend_func(G+7,100, 900);
  pitchbend_func(G-2,50,450);
  pitchbend_func(G,50,450);
  note_off(C, 0);
  note_off(Ds,0);
  note_off(G, 0);

  // lfo used async`ly  
  lfo(74,10,8); 
  lfo(38, 60, 13);
  
}

void setup() {
  //  Set MIDI baud rate:
  Serial.begin(128000); //128000

}
  
  int q = 176 ; //  timing for the notes 1/16 85bpm == 176ms, 
  int h = q*2; // 1/8
  int w = q*4; // 1/4
  
  // tabs with durations example
  int DoIWannaKnow_bassnotes[] = {G,0,0,F,G,B-1,F+oct,E-1+oct,0,D+oct,C+oct,0,0,B-1,C+oct,D+oct,C+oct,0,0,B-1,C+oct,D+oct,D+oct,G,0,0,F,G,B-1,G,0,0,F,G,B-1,F+oct,E-1+oct,0,D+oct,C+oct,0,0,B-1,C+oct,D+oct,C+oct,0,0,B-1,C+oct,D+oct,D+oct,G,0};
  int DoIWannaKnow_durations[] = {w,h,q,q,q,h,h,w,h,h,w,h,q,q,q,q,h,w,h,q,q,h,h,w,w,w,q,q,h,w,h,q,q,q,h,h,w,h,h,w,h,q,q,q,q,h,w,h,q,q,h,h,w,3*w};
  int notes_length = (sizeof(DoIWannaKnow_bassnotes))/(sizeof(DoIWannaKnow_bassnotes[0]));
  
void loop() {

  // functions in loop
   
  // gliding_chords();
   bass_line(DoIWannaKnow_bassnotes,DoIWannaKnow_durations,notes_length);  

  
}
