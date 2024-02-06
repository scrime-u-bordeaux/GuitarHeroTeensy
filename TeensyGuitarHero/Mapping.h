#include "utilities.h"

// MIDI events from Guitar controls and Accelerometer /////////////////////////

// main MIDI channel
const int channel = 1;

// Button class utility -------------------------------------------------------

class Button {
  bool value;
  bool prevValue;

public:
  Button() : value(false), prevValue(false) {}

  bool setValue(bool newValue) {
    bool changed = (newValue != value);
    prevValue = value;
    value = newValue;
    return changed;
  }

  bool getValue() { return value; }
};

// Continuous control class utility -------------------------------------------

class FilteredControl {
  float value;
  OnePole filter;

public:
  FilteredControl() {}

  void setCoefficient(float a) {
    filter.setAlpha(a);
  }

  void setValue(float newValue) {
    filter.process(newValue);
    value = filter.getLowpass();
  }

  float getValue() { return value; }
};

// Main central mapping class -------------------------------------------------

// pure virtual core mapping class to allow for different mapping strategies --

class MIDIMapping {
public:
  virtual void onPluckDown(bool v)                = 0;
  virtual void onPluckUp(bool v)                  = 0;
  virtual void onPlus(bool v)                     = 0;
  virtual void onMinus(bool v)                    = 0;
  virtual void onFret(int i, bool v)              = 0;
  virtual void onFrets(bool* frets)               = 0;
  virtual void onJoyX(float v)                    = 0;
  virtual void onJoyY(float v)                    = 0;
  virtual void onVib(float v)                     = 0;
  virtual void onAccel(float x, float y, float z) = 0;
  virtual void onUpdate()                         = 0;
};

// actual central mapping class -----------------------------------------------

class MIDITransformer {
  Button pluckDown;
  Button pluckUp;
  Button plus;
  Button minus;
  Button frets[5];
  bool fretsState[5];

  // vib and joy filters are for smoothing of low resolution controller values

  FilteredControl joyX;
  FilteredControl joyY;
  FilteredControl vib;

  // accel filters are for HF noise removal

  FilteredControl accelX;
  FilteredControl accelY;
  FilteredControl accelZ;

  MIDIMapping* mapping;

public:
  MIDITransformer(MIDIMapping* m) : mapping(m) {
    float joyCoeff = 0.75;
    float vibCoeff = 0.5;
    float accelCoeff = 0.92;

    joyX.setCoefficient(joyCoeff);
    joyY.setCoefficient(joyCoeff);
    vib.setCoefficient(vibCoeff);
    accelX.setCoefficient(accelCoeff);
    accelY.setCoefficient(accelCoeff);
    accelZ.setCoefficient(accelCoeff);
  }

  void setPluckDown(bool v) {
    if (pluckDown.setValue(v)) { mapping->onPluckDown(v); }
  }
  void setPluckUp(bool v) {
    if (pluckUp.setValue(v)) { mapping->onPluckUp(v); }
  }
  void setPlus(bool v) {
    if (plus.setValue(v)) { mapping->onPlus(v); }
  }
  void setMinus(bool v) {
    if (minus.setValue(v)) { mapping->onMinus(v); }
  }
  void setFret(int i, bool v) {
    if (frets[i].setValue(v)) {
      fretsState[i] = v;
      mapping->onFret(i, v);
      mapping->onFrets(&fretsState[0]);
    }
  }
  void setJoyX(float v) {
    joyX.setValue(v);
    mapping->onJoyX(joyX.getValue());
  }
  void setJoyY(float v) {
    joyY.setValue(v);
    mapping->onJoyY(joyY.getValue());
  }
  void setVib(float v) {
    vib.setValue(v);
    mapping->onVib(vib.getValue());
  }

  void setAccel(float x, float y, float z) {
    accelX.setValue(x);
    accelY.setValue(y);
    accelZ.setValue(z);
    mapping->onAccel(accelX.getValue(), accelY.getValue(), accelZ.getValue());
  }

  void update() { mapping->onUpdate(); }
};

// implementation of virtual MIDIMapping class used by MIDITransformer ////////

class Mapping : public MIDIMapping {
private:
  float azi;
  float ele;
  bool frets[5];
  bool soundingFrets[5];
  bool playSoundingFrets;
  unsigned long lastPluckDate;
  int velocity;
  float bend, vib;

  void onPluck() {
    lastPluckDate = millis();
    for (int i = 0; i < 5; ++i) {
      if (frets[i]) {
        if (soundingFrets[i]) {
          if (playSoundingFrets) {
            usbMIDI.sendNoteOff(i, 0, channel);
            usbMIDI.sendNoteOn(i, velocity, channel);
          }
        } else {
          soundingFrets[i] = true;
          usbMIDI.sendNoteOn(i, velocity, channel);
        }
      }
    }
  }

  void sendNoteEvent(bool on, int note, int velocity, int channel) {
    if (on) { usbMIDI.sendNoteOn(note, velocity, channel); }
    else { usbMIDI.sendNoteOff(note, 0, channel); }
  }

public:
  Mapping(bool replay) : playSoundingFrets(replay) {}

  void onPluckDown(bool v) override {
    if (v) onPluck();
  }

  void onPluckUp(bool v) override {
    if (v) onPluck();
  }
  
  void onPlus(bool v) override {
    playSoundingFrets = true;
    sendNoteEvent(v, 72, 127, channel);
  }
  
  void onMinus(bool v) override {
    playSoundingFrets = false;
    sendNoteEvent(v, 73, 127, channel);
  }

  void onFret(int i, bool v) override {
    if (v) {
      unsigned long date = millis();
      if (date - lastPluckDate < 50 && !soundingFrets[i]) {
        soundingFrets[i] = true;
        usbMIDI.sendNoteOn(i, velocity, channel);
      }
    } else {
      if (soundingFrets[i]) {
        soundingFrets[i] = false;
        usbMIDI.sendNoteOff(i, 0, channel);
      }
    }
  }

  void onFrets(bool* v) override {
    for (int i = 0; i < 5; ++i) {
      frets[i] = *(v + i);
    }
  }

  void onJoyX(float v) override {
    usbMIDI.sendControlChange(14, map(v, 0.f, 1.f, 0, 127), channel);
  }
  
  void onJoyY(float v) override {
    usbMIDI.sendControlChange(15, map(v, 0.f, 1.f, 0, 127), channel);
  }
  
  void onVib(float v) override {
    vib = v;
  }

  void onAccel(float x, float y, float z) override {
    float cx = constrain(x, -1.f, 1.f);
    float cy = constrain(y, -1.f, 1.f);
    float cz = constrain(z, -1.f, 1.f);

    getAzimuthElevation(cx, -cz, cy, azi, ele);
    azi *= -1;
    ele *= -1;

    velocity = constrain(map(azi, -0.3f, 0.15f, 1, 127), 1, 127);
    bend = constrain(map(ele, 0.5f, 0.75f, 0.f, 1.f), 0.f, 1.f);

    usbMIDI.sendPitchBend(map(azi, -1.f, 1.f, -8192, 8191), 2);
    usbMIDI.sendPitchBend(map(ele, -1.f, 1.f, -8192, 8191), 3);
  }

  void onUpdate() {
    usbMIDI.sendPitchBend(map(-vib + bend, -1.f, 1.f, -8192, 8191), channel);
  }
};
