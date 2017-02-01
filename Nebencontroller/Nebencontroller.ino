#include <TimerOne.h>

#include <Button.h>
#include <ButtonEventCallback.h>
#include <PushButton.h>
#include <Bounce2.h>
#include <LedControl.h>


LedControl lc = LedControl(11, 13, 10, 2);
PushButton buttonSel = PushButton(3, ENABLE_INTERNAL_PULLUP);
PushButton buttonUp = PushButton(4, ENABLE_INTERNAL_PULLUP);
PushButton buttonDown = PushButton(5, ENABLE_INTERNAL_PULLUP);
PushButton buttonLeft = PushButton(7, ENABLE_INTERNAL_PULLUP);
PushButton buttonRight = PushButton(6, ENABLE_INTERNAL_PULLUP);

long previousMillisCursor = 0;
long CursorBlinkInterval = 500;

bool logo[8][16] = {
  {1, 1, 1, 1, 1, 1, 1, 1,     1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 1,     1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 1, 1, 1, 0, 1,     1, 0, 1, 1, 1, 0, 0, 1},
  {1, 0, 0, 1, 0, 0, 0, 1,     1, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 0, 0, 1, 0, 0, 1,     1, 0, 1, 0, 0, 1, 0, 1},
  {1, 0, 1, 1, 1, 1, 0, 1,     1, 0, 1, 1, 1, 0, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 1,     1, 0, 0, 0, 0, 0, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1,     1, 1, 1, 1, 1, 1, 1, 1}
};
bool selmatrix[8][16] = {0};
bool drawmatrix[8][16] = {0};
int cursor[] = {0, 0, 1};

int scancol = 0;

int basenote = 60;

//Clock-Variabeln
byte midi_start = 0xfa;
byte midi_stop = 0xfc;
byte midi_clock = 0xf8;
byte midi_continue = 0xfb;
byte midi_pos = 0xf2;
int play_flag = 0;
byte data;
int clockcount = 0;

void setup() {
  Serial.begin(31250);

  Timer1.initialize(125000);
  Timer1.attachInterrupt(cursorMove); // blinkLED to run every 0.15 seconds

  int devices = lc.getDeviceCount();
  for (int address = 0; address < devices; address++) {
    lc.shutdown(address, false);
    lc.setIntensity(address, 8);
    lc.clearDisplay(address);
  }

  buttonSel.onPress(onBtnPressed);
  buttonUp.onPress(onBtnPressed);
  buttonDown.onPress(onBtnPressed);
  buttonLeft.onPress(onBtnPressed);
  buttonRight.onPress(onBtnPressed);

  drawMatrices(logo);
  delay(2000);
  Serial.write(0xf2);
  Serial.write(0x00);
  Serial.write(0x00);
  draw();
}
void counter() {


  if (clockcount == 0) {
    sendMidi();

  }
  clockcount++;
  if (clockcount > 23) {
    clockcount = 0;
    advanceScanner();
  }
}


void advanceScanner() {
  scancol++;
  if (scancol >= 16) {
    scancol -= 16;
  }
  draw();
}
void changeProgram(int chan, int prog) {
  Serial.write(chan);
  Serial.write(prog);
}
int selNote(int r, int base) {
  switch (r) {
    case 0:
      return base;
      break;
    case 1:
      return base + 2;
      break;
    case 2:
      return base + 4;
      break;
    case 3:
      return base + 5;
      break;
    case 4:
      return base + 7;
      break;
    case 5:
      return base + 9;
      break;
    case 6:
      return base + 11;
      break;
    case 7:
      return base + 12;
      break;
  }
}
void clearNotes() {
  int i;
  //Vorherigen Ton ausschalten
  for (i = 0; i < 8; i++) {
    noteOn(0x85, selNote(i, basenote), 0x45);
  }
}
void sendMidi() {
  clearNotes();
  int r;
  for (r = 0; r < 8; r++) {
    if (selmatrix[r][scancol] == 1) {
      noteOn(0x95, selNote(r, basenote), map(analogRead(A0), 0, 1023, 0, 127));
    }
  }

}
void noteOn(int cmd, int pitch, int velocity) {
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
}
void draw() {
  int sendcol;

  for (int j = 0; j < 16; j++) {
    sendcol = 0;
    if (j == scancol) {
      sendcol = 255;
    } else {
      for (int i = 0; i < 8; i++) {
        sendcol |= selmatrix[7-i][j] << i;
      }
    }

    if (j < 8) {
      lc.setRow(0, j, sendcol);
    } else {
      lc.setRow(1, j - 8, sendcol);
    }
  }

}
void cursorMove() {

  if (cursor[1] < 8) {
    lc.setLed(0, cursor[1], cursor[0], cursor[2]);
  } else {
    lc.setLed(1, cursor[1] - 8, cursor[0], cursor[2]);
  }

  cursor[2] = !cursor[2];
}
void loop() {
  if (Serial.available() > 0) {
    data = Serial.read();
    if (data == midi_start) {
      play_flag = 1;
    }
    else if (data == midi_continue) {
      play_flag = 1;
    }
    else if (data == midi_stop) {
      play_flag = 0;
      clearNotes();
    }
    else if ((data == midi_clock) && (play_flag == 1)) {
      counter();
    }
    else if (data == midi_pos) {
      while (!Serial.available());
      int pos = 0;
      pos += Serial.read();
      while (!Serial.available());
      pos += Serial.read() * 128;
      pos = pos / 4;
      scancol = pos % 16;
      clockcount = 0;
      draw();
    }
  }
  buttonSel.update();
  buttonUp.update();
  buttonDown.update();
  buttonLeft.update();
  buttonRight.update();
}
void drawMatrices(bool data[8][16]) {
  int r, c;
  for (r = 0; r < 8; r++) {
    for (c = 0; c < 16; c++) {
      if (c < 8) {
        lc.setLed(0, c, r, data[r][c]);
      } else {
        lc.setLed(1, c - 8 , r, data[r][c]);
      }
    }
  }
}
void hide() {
  lc.shutdown(0, true);
  lc.shutdown(1, true);
}
void show() {
  lc.shutdown(0, false);
  lc.shutdown(1, false);
}
void onBtnPressed(Button& btn) {
  if (btn.is(buttonSel)) {
    selmatrix[cursor[0]][cursor[1]] = !selmatrix[cursor[0]][cursor[1]];
  } else if (btn.is(buttonUp)) {
    cursor[0]++;
    if (cursor[0] >= 8) {
      cursor[0] -= 8;
    }
  } else if (btn.is(buttonDown)) {
    cursor[0]--;
    if (cursor[0] < 0) {
      cursor[0] += 8;
    }
  } else if (btn.is(buttonLeft)) {
    cursor[1]++;
    if (cursor[1] >= 16) {
      cursor[1] -= 16;
    }
  } else if (btn.is(buttonRight)) {
    cursor[1]--;
    if (cursor[1] < 0) {
      cursor[1] += 16;
    }
  }
  draw();
}
