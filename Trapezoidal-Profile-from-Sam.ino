// Trapezoidal out-and-back profile based on distance
// L298N: direction uses D5 and D6 (as you discovered)

float distance_ft = 10;
float distance_m  = distance_ft * 0.3048;
float accel_scale = 0.95;
float vmax_rpm    = 220.0;

float wheel_diam_in = 3.0;
float a_max = 0.4032;

const int IN1_PIN = 5;   // D5 must be HIGH for forward
const int IN2_PIN = 6;   // D6 must be LOW  for forward

float t = 0;
float target = 0;
unsigned long t0 = 0;

void setMotorDir(float targetRPM) {
  if (targetRPM > 0) {
    digitalWrite(IN1_PIN, HIGH);
    digitalWrite(IN2_PIN, LOW);
  } else if (targetRPM < 0) {
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, HIGH);
  } else {
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, LOW);  // coast/stop
  }
}

void setup() {
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  t0 = millis();
}

void loop() {

  t = (float)(millis() - t0) * 1e-3;

  float wheel_diam_m = wheel_diam_in * 0.0254;
  float C = PI * wheel_diam_m;                 // m/rev

  float a = accel_scale * a_max;               // m/s^2
  float vmax = (vmax_rpm / 60.0) * C;          // m/s

  float ta  = vmax / a;
  float dad = (vmax * vmax) / a;

  float tc, Tone;

  if (distance_m > dad) {
    tc = (distance_m - dad) / vmax;
    Tone = 2.0 * ta + tc;
  } else {
    ta = sqrt(distance_m / a);
    tc = 0.0;
    Tone = 2.0 * ta;
    vmax = a * ta;
  }

  float Tround = 2.0 * Tone;

  if (t < ta) {
    float v = a * t;
    target = 60.0 * (v / C);
  }
  else if (t < ta + tc) {
    target = 60.0 * (vmax / C);
  }
  else if (t < Tone) {
    float tdec = t - (ta + tc);
    float v = vmax - a * tdec;
    target = 60.0 * (v / C);
  }
  else if (t < Tone + ta) {
    float trel = t - Tone;
    float v = a * trel;
    target = -60.0 * (v / C);
  }
  else if (t < Tone + ta + tc) {
    target = -60.0 * (vmax / C);
  }
  else if (t < Tround) {
    float trel = t - (Tone + ta + tc);
    float v = vmax - a * trel;
    target = -60.0 * (v / C);
  }
  else {
    target = 0;
  }

  // Direction will now flip correctly
  setMotorDir(target);

  // NOTE: This version does NOT do PWM speed control.
  // Your motor will run at whatever speed your hardware enables.

  delay(50);
}
