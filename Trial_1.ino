#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <RGBmatrixPanel.h>

#include <MPU9250.h>
#include <quaternionFilters.h>
#include <SPI.h>
#include<Wire.h>

MPU9250 myIMU;

#define CLK 8  // MUST be on PORTB! (Use pin 11 on Mega)
#define LAT A3
#define OE  9
#define A   A0
#define B   A1
#define C   A2
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);


// software reset of arduino
void(* reset)(void) = 0;


//-------------------For Scroll Text ---------------------//
#define F2(progmem_ptr) (const __FlashStringHelper *)progmem_ptr
const char str[] PROGMEM = "DANGER NOODLE!! Place Controller On Table!";
int    textX = matrix.width(),
       textMin = sizeof(str) * -12,
       hue = 0;
#define F2(progmem_ptr) (const __FlashStringHelper *)progmem_ptr
const char str_1[] PROGMEM = "DANGER NOODLE!! GAME OVER!!";
int    textX_1 = matrix.width();
//---------------------------------------------------------//

int dirX;
int dirY;
int len = 3;
int count = 0;
int snake[30][2]; 
int applePos[2] = {1,0};
int dangerPos_1[2];
int dangerPos_2[2];
int dangerPos_3[2];
int dangerPos_4[2];// = {random(0,31), random(0,15)};
int newCoord[2] = {6,2};

// Enumerated type to store position values.
enum direction {
  none,
  right, 
  left, 
  up,
  down
};

direction dir = down;
int threshold = 5000; // IMU tilt threshold. (lower # = sensitive, higher # = less sensitive)
  
unsigned long hope;
unsigned long time;
  

void setup() {
  Serial.begin(38400);
  Wire.begin();
  matrix.begin();
  matrix.setTextWrap(false); // Allow text to run off right edge
  matrix.setTextSize(2);
  
  // Calibrating the IMY bias' 
  myIMU.calibrateMPU9250(myIMU.gyroBias, myIMU.accelBias); 
  placeDanger();
  placeApple();
}

void loop() { 
    myIMU.readAccelData(myIMU.accelCount);  // Read the x/y/z adc values
    myIMU.getAres();
    myIMU.ax = myIMU.accelCount[0]; // using raw values w/o converting to g's 
    myIMU.ay = myIMU.accelCount[1];

 //counter for changing position of danger apple
  if(hope == 20) {
    //top
    dangerPos_1[0] = random(0,14);
    dangerPos_1[1] = random(0,7);
    dangerPos_2[0] = random(15,31);
    dangerPos_2[1] = random(0,7);
    // bottom
    dangerPos_3[0] = random(0,14);
    dangerPos_3[1] = random(8,15);
    dangerPos_4[0] = random(15,31);
    dangerPos_4[1] = random(8,15);
    hope = 0;
  } hope = hope+1;

// calling methods
    drawDanger();
    drawApple();
    drawSnake();
    delay(100); // Inadvertantly determines how fast the snake goes.
    newDirection();
    moveSnake();
    erase();

}  
//--------------- Drirectional Controll ---------------//
void newDirection() {
  if (myIMU.ay <= myIMU.ax && myIMU.ay <= (-1 * threshold)) { // This allows the Snake to travel in the downward direction.
    if (dir != up){
      dir = down;
      Serial.println("normal:"); 
    }
    Serial.println("double back:"); 
  } else if (myIMU.ay >= myIMU.ax && myIMU.ay >= threshold) {
    if (dir != down){
      dir = up;
      Serial.println("normal:"); 
    }
    Serial.println("double back:"); 
  } else if (myIMU.ax >= myIMU.ay && myIMU.ax >= threshold) {
    if (dir != right){
      dir = left;
      Serial.println("normal:"); 
    }
    Serial.print("double back:"); 
  } else if (myIMU.ax <= myIMU.ay && myIMU.ax <= (-1 * threshold)) {
    if (dir != left){
      dir = right;
      Serial.println("normal:"); 
    }
    Serial.println("double back:"); 
  }
  
  if (dir == down) { // DOWN
    newCoord[1] -= 1;
    if(newCoord[1] < 0){
      newCoord[1] = 15;
    }
  } else if (dir == up) { // UP
    newCoord[1] += 1;
    if(newCoord[1] > 15){
      newCoord[1] = 0;
    }
  } else if (dir == left) { //LEFT
    newCoord[0] -= 1;
    if(newCoord[0] < 0){
      newCoord[0] = 31; 
    }
  } else if (dir == right) { // RIGHT
    newCoord[0] += 1;
    if(newCoord[0] > 31){
      newCoord[0] = 0;
    }
  }
}
//---------------- Apple Placement -------------//
void placeApple() { 
 bool done = false;
  while(done == false) {
   byte newX = random(1,31);
   byte newY = random(1,15);
  if (!(snakeCheck(newX,newY))) { // this check if the apple will be placed in the position of the snake
   applePos[0] = newX;
   applePos[1] = newY;
   done = true;
  }
 }
} 
//---------------- Danger Placement -------------//
void placeDanger() {
  bool done = false;
  while(done == false) {
    int deathX = random(0,31);
    int deathY = random(0,15);
  if(!(snakeCheck(deathX,deathY))) { // this checks if the Danger Apple will be placed in the position of the snake.. 
    dangerPos_1[0] = deathX;
    dangerPos_1[1] = deathY;
    done = true;
    }
   else if (!(snakeCheck(deathX,deathY))) {
    dangerPos_2[0] = deathX;
    dangerPos_2[1] = deathY;
    done = true;
    }
   else if (!(snakeCheck(deathX,deathY))) {
    dangerPos_3[0] = deathX;
    dangerPos_3[1] = deathY;
    done = true;
    }
   else if (!(snakeCheck(deathX,deathY))) {
    dangerPos_4[0] = deathX;
    dangerPos_4[1] = deathY;
    done = true;
    }
  }
}
//----------------- Detecting Apple Hit -------------------// 
bool appleHit() {
  if (newCoord[0] == applePos[0] && newCoord[1] == applePos[1]) { // Detects if the head of the snake is in the position of the apple,
    return true;                                                  // if it is, then it returns a true val, else its false.
  }
  else{
    return false;
  }
}
//------------------ Detecting Danger Apple Hit ----------------------//
bool dangerHit() {
  if (newCoord[0] == dangerPos_1[0] && newCoord[1] == dangerPos_1[1]) { // Essentially does the same as appleHit().
    return true;
  }
  else if (newCoord[0] == dangerPos_2[0] && newCoord[1] == dangerPos_2[1]) {
    return true;
  }
  else if (newCoord[0] == dangerPos_3[0] && newCoord[1] == dangerPos_3[1]) {
    return true;
  }
    else if (newCoord[0] == dangerPos_4[0] && newCoord[1] == dangerPos_4[1]) {
    return true;
    }
  else{
    return false;
  }  
}
//-------------- Checking if Snake hits itself------------// 
bool snakeCheck(int checkX, int checkY){
  for (int i = 0; i < len; i++) {
    if (checkX == snake[i][0] && checkY == snake[i][1]) { // This checks if the snake hits itself, and returns a true or false value
      return true;
    }
  }
  return false;
}
//------------------ Snake Movement ----------------------//
void moveSnake() {
  if (snakeCheck(newCoord[0], newCoord[1] ) == true) { // This checks to see if the movement of the snake is hitting itself,
   youdied();                                          // if so, then it prompts the Game Over Screens, and does a system rst.
   resetSnek();
   reset();
   }
  else if (appleHit() == true) {                      // This increases the length of the snake if an apple is hit,
   snake[len][0] = newCoord[0];                       // also places a new apple on the screen.
   snake[len][1] = newCoord[1];
   len = len + 1;
   count = count + 1;
   placeApple();
  }
  else if (dangerHit() == true) {                    // This detects if the snake hits a Danger Apple, and prompts the reset screens
   snake[len][0] = newCoord[0];
   snake[len][1] = newCoord[1];                      // also prompts the score
   resetSnek();
   delay(2000);
   SCORE();
   reset();
  }
  else{
      for(int i = 0; i < len; i++) {                // if nothing is hit, the snake is just going to keep moving.
      snake[i][0] = snake[i+1][0];
      snake[i][1] = snake[i+1][1];
    }
      snake[len-1][0] = newCoord[0];
      snake[len-1][1] = newCoord[1];
  }
}  
//-------------- Death State -----------------//
void youdied() {
  SCORE();                                               // This is the death screen.
  erase();
  
  matrix.setCursor(5,0);
  matrix.setTextSize(1);
  matrix.setTextColor(matrix.Color333(255, 0 , 0));
  matrix.print('G');
  matrix.setTextColor(matrix.Color333(255, 0 , 0));
  matrix.print('A');
  matrix.setTextColor(matrix.Color333(255, 0 , 0));
  matrix.print('M');
  matrix.setTextColor(matrix.Color333(255, 0 , 0));
  matrix.print('E');
   
  matrix.setCursor(5,9);
  matrix.setTextColor(matrix.Color333(255, 0 , 0));
  matrix.print('O');
  matrix.setTextColor(matrix.Color333(255, 0 , 0));
  matrix.print('V');
  matrix.setTextColor(matrix.Color333(255, 0 , 0));
  matrix.print('E');
  matrix.setTextColor(matrix.Color333(255, 0 , 0));
  matrix.print('R');
  delay(2000);
}

//---------- Erase Screen -----------//
void erase() {
  matrix.fillScreen(matrix.Color333(0, 0, 0));  
}
//------------- Draw Snake -----------------//
void drawSnake() {
  for (int i = 0; i < len; i++) {                                                  // This is just adding color/drawing the snake.
    matrix.drawPixel(snake[i][0], snake[i][1], matrix.Color333(0 , 255, 255));
    matrix.drawPixel(snake[len-1][0], snake[len-1][1], matrix.Color333(255, 0, 0));
  }
  
}
//-------------- Draw Apple --------------//
void drawApple() {
  matrix.drawPixel(applePos[0], applePos[1], matrix.Color333(0,7,0));            // draws the apple at the generated position
}
//--------------Draw Danger Apple ---------------//
void drawDanger() {                                                              // draws the Danger Apples at the generated positions
  matrix.drawPixel(dangerPos_1[0], dangerPos_1[1], matrix.Color333(random(0,255) ,random(0,255) ,random(0,255)));
  matrix.drawPixel(dangerPos_2[0], dangerPos_2[1], matrix.Color333(random(0,255) ,random(0,255) ,random(0,255)));
  matrix.drawPixel(dangerPos_3[0], dangerPos_3[1], matrix.Color333(random(0,255) ,random(0,255) ,random(0,255)));
  matrix.drawPixel(dangerPos_4[0], dangerPos_4[1], matrix.Color333(random(0,255) ,random(0,255) ,random(0,255)));
}
//-----------------Displays # of Apples Hit----------------//
void SCORE() {
  erase();                                                      // This takes the score, and displays it.
  matrix.setCursor(1,0);
  matrix.setTextSize(1);
  matrix.setTextColor(matrix.Color333(255, 0 , 0));
  matrix.print('S');
  matrix.setTextColor(matrix.Color333(255, 0 , 0));
  matrix.print('C');
  matrix.setTextColor(matrix.Color333(255, 0 , 0));
  matrix.print('O');
  matrix.setTextColor(matrix.Color333(255, 0 , 0));
  matrix.print('R');
  matrix.setTextColor(matrix.Color333(255, 0 , 0));
  matrix.print('E');
  matrix.setCursor(13,9);
  matrix.setTextSize(1);
  matrix.setTextColor(matrix.Color333(255, 0 , 0));
  matrix.print(count);
  delay(2000);
}
//--------------- Displays Danger Text Scroll -----------------//
void DANGER () {
  erase();                                                                // This is Scroll test for the reset screens
  int flag = 1;
  float start_time = millis();
  float buffer_time = 5000;
  while(flag){
  erase();
  matrix.setTextColor(matrix.ColorHSV(hue, 255, 255, true));
  matrix.setCursor(textX, 1);
  matrix.print(F2(str_1));
  
  if((--textX) < textMin) textX = matrix.width();
  hue+= 7;
  if(hue >= 1536) hue -= 1536;
  
  if(millis()>start_time+buffer_time){
     flag = 0; 
  }
  
  matrix.swapBuffers(true);
  delay(10);
  }
}
//--------------- Displays Reset Instruction Text Scroll -----------------//
void resetSnek() {                                                     // This is Scroll test for the reset screens
  erase();
  int flag = 1;
  float start_time = millis();
  float buffer_time = 12000;
  
  while(flag){
    erase();
    matrix.setTextColor(matrix.ColorHSV(hue, 255, 255, true));
    matrix.setCursor(textX, 1);
    matrix.print(F2(str));
  
  if((--textX) < textMin) {
    textX = matrix.width();
  }
  hue+= 7;
  if(hue >= 1536) {
    hue -= 1536;
  }
  if(millis()>start_time+buffer_time){                                  // Added a counter so the scroll method could be broken out of after a certain 
     flag = 0;                                                          // ammount of time
  }
  matrix.swapBuffers(true);
  delay(10);
  }
}
