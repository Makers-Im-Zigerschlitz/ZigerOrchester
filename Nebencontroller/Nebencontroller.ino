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
long CursorBlinkInterval = 50;

bool logo[8][16] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1},
  {1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};
bool selmatrix[8][16] = {0};
bool drawmatrix[8][16] = {0};
int cursor[] = {0, 0, 0};

int scancol = 0;
void setup() {
  Serial.begin(31250);
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
  delay(1000);

  Timer1.initialize(500000);
  Timer1.attachInterrupt(advanceScanner); // blinkLED to run every 0.15 seconds


}

void advanceScanner() {
  draw();
  scancol++; 
  if (scancol >= 16) {
    scancol -= 16;
    //changeProgram(0xC5,random(127));
  }
  sendMidi();
}
void changeProgram(int chan, int prog) {
  Serial.write(chan);
  Serial.write(prog);
}
void sendMidi() {
  int i;
  for (i=0;i<8;i++) {
    noteOn(0x85, 60+i, 0x45);
  }
  int r;
  for (r = 0; r < 8; r++) {
    if (selmatrix[r][scancol] == 1) {
      //Auf Kanal 5 Note ab C4 = 60 senden mit mittlerer Velocity
      //Serial.println(map(analogRead(A0),0,1023,0,127));
      noteOn(0x95, 60+r, map(analogRead(A0),0,1023,0,127));
    }
  }  
  
}
void noteOn(int cmd, int pitch, int velocity) {
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
}
void draw() {
  memset(drawmatrix, 0, sizeof(drawmatrix));
  memcpy(drawmatrix, selmatrix, sizeof(drawmatrix));
  
  int r;
  for (r = 0; r < 8; r++) {
    drawmatrix[r][scancol] = 1;
  }
  drawmatrix[cursor[0]][cursor[1]] = cursor[2];
  drawMatrices(drawmatrix);
}
void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisCursor > CursorBlinkInterval) {
    // save the last time you blinked the LED
    previousMillisCursor = currentMillis;

    cursor[2]=!cursor[2];
    draw();
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

