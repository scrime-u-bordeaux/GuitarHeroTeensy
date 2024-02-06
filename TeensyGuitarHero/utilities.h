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
    prevOut   = in * oneMinusAlpha + prevOut * alpha;
    highpass  = in - prevOut;
  }

  float getLowpass()  { return prevOut;   }
  float getHighpass() { return highpass;  }
};

// get azimuth / elevation from accelerometer values //////////////////////////

void getAzimuthElevation(float x, float y, float z, float& azi, float& ele) {
  ele = asinf(x) * 2 / PI;
  azi = atan2f(z, y) / PI;
}
