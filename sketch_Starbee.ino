#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

const int SDA_PIN =6;
const int SCL_PIN =7;
const int BUTTON_PIN =4;

Adafruit_SSD1306 screen(128, 64, &Wire, -1);
Adafruit_MPU6050 mpu;

const int EYE_WIDTH =22;
const int EYE_HEIGHT =28;
const int EYE_SPACING =14;
const int EYE_RADIUS =7;

float faceX = 64.0;
const int faceY = 36;

enum Mood{NORMAL, HAPPY, SHOCKED};
Mood currentMood = NORMAL;
uint32_t moodTimer = 0;

uint32_t nextBlinkTime = 0;
bool isBlinking = false;
uint32_t blinkStartTime = 0;

//gamee
float starX = 64.0;
float starY = 0.0;
float starSpeed = 1.2;
int score = 0;
int lives = 3;
bool isGameOver = false;

void drawEyes(int centerX, int centerY, Mood mood) {
  int leftEyeX = centerX - (EYE_SPACING/2) - EYE_WIDTH;
  int rightEyeX = centerX + (EYE_SPACING/2);

  if (mood == HAPPY) {
    screen.fillRoundRect(leftEyeX, centerY - 6, EYE_WIDTH, 8, 4, SSD1306_WHITE);
    screen.fillRoundRect(rightEyeX, centerY - 6,EYE_WIDTH, 8, 4, SSD1306_WHITE);
    return;
  }

  int currentHeight = EYE_HEIGHT;
  if (isBlinking) {
    currentHeight = 3;
  }
  int eyeY = centerY - (currentHeight/2);
  screen.fillRoundRect(leftEyeX, eyeY, EYE_WIDTH, currentHeight, EYE_RADIUS, SSD1306_WHITE);
  screen.fillRoundRect(rightEyeX, eyeY, EYE_WIDTH, currentHeight, EYE_RADIUS, SSD1306_WHITE);
  if (!isBlinking && currentMood == NORMAL) {
    int pupilY = eyeY +4;
    screen.fillCircle(leftEyeX + 11, pupilY, 3, SSD1306_BLACK);
    screen.fillCircle(rightEyeX + 11, pupilY, 3, SSD1306_BLACK);
  }
}
  void drawStar (int x, int y) {
    screen.drawPixel(x, y, SSD1306_WHITE);
    screen.drawPixel(x - 1, y, SSD1306_WHITE);
    screen.drawPixel(x+1, y, SSD1306_WHITE);
    screen.drawPixel(x, y-1, SSD1306_WHITE);
    screen.drawPixel(x, y+1, SSD1306_WHITE);
  }

  void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    Wire.begin(SDA_PIN, SCL_PIN);

    screen.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    screen.clearDisplay();
    screen.setTextColor(SSD1306_WHITE);
    screen.setTextSize(1);

    mpu.begin(0x68, &Wire);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    nextBlinkTime = millis() + random(2000, 4500);

    drawEyes(64, 32, NORMAL);
    screen.display();
    delay(1200);

  }
  void loop(){
    uint32_t now = millis();
    //button
    if (digitalRead(BUTTON_PIN) == LOW) {
      score =0;
      lives =3;
      starSpeed =1.2;
      starY =0;
      starX = random(15,110);
      isGameOver = false;
      currentMood = NORMAL;
      delay(200);
    }

    if(isGameOver) {
      screen.clearDisplay();
      screen.setCursor(38, 16);
      screen.print("O W N O");
      screen.setCursor(30, 34);
      screen.print("SCORE: ");
      screen.print(score);
      screen.setCursor(14, 50);
      screen.print("PRESS BTN TO PLAY");
      screen.display();
      return;
    }

    if (now > nextBlinkTime && !isBlinking) {
      isBlinking=true;
      blinkStartTime = now;
    }
    if(isBlinking && (now - blinkStartTime > 120)) {
      isBlinking = false;
      nextBlinkTime = now + random(2500, 5000);
    }
    if(currentMood == HAPPY && (now - moodTimer > 450)) {
      currentMood = NORMAL;
    }

    sensors_event_t accel, gyro, temp;
    mpu.getEvent(&accel, &gyro, &temp);

    faceX += accel.acceleration.x * 1.7;

    int totalFaceWidth = (EYE_WIDTH * 2) + EYE_SPACING;
    if (faceX < (totalFaceWidth/2)) {
      faceX = totalFaceWidth/2;
    }
    if(faceX > (128 - totalFaceWidth/2)) {
      faceX = 128 - totalFaceWidth/2;
    }

    starY += starSpeed;

    int faceLeft = faceX - (totalFaceWidth/2);
    int faceRight = faceX + (totalFaceWidth/2);

    if (starY >= (faceY -14) && starY <= (faceY + 14)) {
      if (starX >= faceLeft && starX <= faceRight) {
        score++;
        currentMood = HAPPY;
        moodTimer = now;
        starSpeed += 0.07;
        starY = 0;
        starX = random(12, 116);
      }
    }

    if(starY > 64) {
      lives--;
      starY = 0;
      starX =random(12, 116);
      if (lives <= 0) isGameOver = true;
    }

    screen.clearDisplay();

    drawEyes((int)faceX, faceY, currentMood);
    drawStar((int)starX, (int)starY);

    screen.setCursor(2,2);
    screen.print(score);

    screen.display();
    delay(30);
  }