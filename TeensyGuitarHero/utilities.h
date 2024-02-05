#include <math.h>

// simple onepole filter including cheap highpass /////////////////////////////

// TODO : implement a one euro filter instead (more responsive)
class OnePole {
  float alpha;
  float oneMinusAlpha;
  float prevOut;
  float highpass;

public:
  OnePole(float a = 1.f) :
    alpha(a),
    oneMinusAlpha(1.f - a),
    prevOut(0.f),
    highpass(0.f) {}

  void setAlpha(float a) {
    alpha         = a;
    oneMinusAlpha = 1 - a;
  }

  void process(float in) {
    prevOut   = in * alpha - prevOut * oneMinusAlpha;
    highpass  = in - prevOut;
  }

  float getLowpass()  { return prevOut;   }
  float getHighpass() { return highpass;  }
};

// get azimuth / elevation from accelerometer values //////////////////////////

void getAzimuthElevation(float x, float y, float z, float& azi, float& ele) {
  ele = asinf(x) * 2 / PI;
  azi = atan2f(y, z);
}

// MIDI events from Guitar controls and Accelerometer /////////////////////////

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
};

// Continuous control class utility -------------------------------------------

class FilteredControl {
  float f;
  OnePole filter;

public:
  FilteredControl() {}

  void setValue(float v) {
    filter.process(v);
    f = filter.getLowpass();
  }

  float getValue() { return f; }
};

// Main central mapping class -------------------------------------------------

// pure virtual core mapping class to allow for different mapping strategies
class MIDIMapping {
public:
  virtual void onPluckDown(bool v)                = 0;
  virtual void onPluckUp(bool v)                  = 0;
  virtual void onPlus(bool v)                     = 0;
  virtual void onMinus(bool v)                    = 0;
  virtual void onFret(int i, bool v)              = 0;
  virtual void onJoyX(float v)                    = 0;
  virtual void onJoyY(float v)                    = 0;
  virtual void onVib(float v)                     = 0;
  virtual void onAccel(float x, float y, float z) = 0;
};

// actual central mapping class
class MIDITransformer {
  Button pluckDown;
  Button pluckUp;
  Button plus;
  Button minus;
  Button frets[5];

  FilteredControl joyX;
  FilteredControl joyY;
  FilteredControl vib;

  FilteredControl accelX;
  FilteredControl accelY;
  FilteredControl accelZ;

  MIDIMapping* mapping;

public:
  // TODO : set optimal filtering coefficients for vib, joy and accel in constructor :
  // - vib and joy filters are for smoothing of low resolution controller values
  // - accel filters are for HF noise removal

  MIDITransformer(MIDIMapping* m) : mapping(m) {}

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
    if (frets[i].setValue(v)) { mapping->onFret(i, v); }
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
};
