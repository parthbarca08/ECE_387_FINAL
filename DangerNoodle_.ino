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

// true,false
#define true 1;
#define false 0;


// software reset of arduino
void(* reset)(void) = 0;


//-------------------For Scroll Text ---------------------//
#define F2(progmem_ptr) (const __FlashStringHelper *)progmem_ptr
const char str[] PROGMEM = "DANGER NOODLE!! Place Controller On Table!";
int textX = matrix.width();
#define F2(progmem_ptr) (const __FlashStringHelper *)progmem_ptr
const char str_1[] PROGMEM = "DANGER NOODLE!! GAME OVER!!";
int    textX_1 = matrix.width();
int textMin = sizeof(str) * -12;
int hue = 0;
//---------------------------------------------------------//

int len = 3;       // Length, initial of 3 is arbitrary, 3 because Noodle™ cannot kill itself quicker than the first 3 links of body
int count = 0;     // For keeping track of Score
int noodleX[30];   // Danger Noodle™ max Length.
int noodleY[30];   // Danger Noodle™ max Length. 
int applePosX = 1; // Food Positions
int applePosY = 0; // Food Positions.
int dangerPos_1[2]; // Danger Apple™ Variables
int dangerPos_2[2]; // Danger Apple™ Variables
int dangerPos_3[2]; // Danger Apple™ Variables
int dangerPos_4[2]; // Danger Apple™ Variables
int positionX = 6; // initial X position of Danger Noodle™
int positionY = 2; // initial Y position of Danger Noodle™

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
  
unsigned long appleDanger; // counter for Danger Apple™ placement.
  

void setup() {
  Serial.begin(38400);
  Wire.begin();
  matrix.begin();
  matrix.setTextWrap(false); // Allow text to run off right edge
  matrix.setTextSize(2);
  
  // Calibrating the IMU bias' 
  myIMU.calibrateMPU9250(myIMU.gyroBias, myIMU.accelBias);
}

void loop() { 
    myIMU.readAccelData(myIMU.accelCount);  // Read the x/y/z adc values
    myIMU.getAres();
    myIMU.ax = myIMU.accelCount[0]; // using raw values w/o converting to g's 
    myIMU.ay = myIMU.accelCount[1];

 //counter for changing position of danger apple
 //changes position evry 2 seconds, as represented by one count = 100ms, times that by 20 which gives you 2000ms = 2sec
  if(appleDanger == 20) {
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
  } appleDanger = appleDanger+1;

// calling methods
    drawDanger();
    drawFood();
    drawSnake();
    delay(100); //usefull to determines how fast the snake goes.
    Direction();
    moveCheck();
    clear(); // the reason the clear() method is called is so that the screen is not constantly accululating the trail of the Danger Noodle™
              // without this, the screen would have no refresh rate.

}  
//--------------- Drirectional Controll ---------------//
// Consider the plane of the directional movement as a cartisean coordinate system, 
// in this case I had to figure out how to prevent the snake from moving diagonally, so i created a check 
// that made sure if the accelerometer was tilted in a certain direction to only change direction if it
// was inline with the axis of tilt.!!!!   
void Direction(void) {
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
  
// This allows the Noodle to wrap around the LED's so it can wrap infinitly.
  if (dir == down) { // DOWN
    positionY -= 1;
    if(positionY < 0){
      positionY = 15;
    }
  } else if (dir == up) { // UP
    positionY += 1;
    if(positionY > 15){
      positionY = 0;
    }
  } else if (dir == left) { //LEFT
    positionX -= 1;
    if(positionX < 0){
      positionX = 31; 
    }
  } else if (dir == right) { // RIGHT
    positionX += 1;
    if(positionX > 31){
      positionX = 0;
    }
  }
}
//---------------- Apple Placement -------------//
void placeFood(void) { 
    boolean check;
      while(check == 0) { // this is for checking if the internal code has been completed pending on the if-statement
        int newX = random(1,31); // generates the ramdom x position
        int newY = random(1,15); // generates the ramdom y position
      for (int i = 0; i < len; i++) { // loop that monitors the whole length of the current Noodle™. 
       if (positionX != noodleX[i] && positionY != noodleY[i]) { // this check if the apple will be placed in the position of the snake
        applePosX = newX; // sets the random positon to the food location as determined by the random function.
        applePosY = newY;
        check = 1; // indicates wheather the if-statement has been executed
      }
    }  
  }
} 
//---------------- Danger Placement -------------//
// same concept as placeFood().
void placeDanger(void) { 
    boolean check;
  while(check == 0) {
    int deathX = random(0,31);
    int deathY = random(0,15);
  for (int i = 0; i < len; i++) {
    if(positionX != noodleX[i] && positionY != noodleY[i]) { // this checks if the Danger Apple will be placed in the position of the snake.. 
      dangerPos_1[0] = deathX;
      dangerPos_1[1] = deathY;
      check = 1;
      }
     else if (positionX != noodleX[i] && positionY != noodleY[i]) {
      dangerPos_2[0] = deathX;
      dangerPos_2[1] = deathY;
      check = 1;
      }
     else if (positionX != noodleX[i] && positionY != noodleY[i]) {
      dangerPos_3[0] = deathX;
      dangerPos_3[1] = deathY;
      check = 1;
      }
     else if (positionX != noodleX[i] && positionY != noodleY[i]) {
      dangerPos_4[0] = deathX;
      dangerPos_4[1] = deathY;
      check = 1;
      }
    }
  }
}
//------------------ Snake Movement ----------------------//
/*
*This is where the program can tell if it has first, hit the snake itself, next it performs
*checks to see if it has either collided with a Food Apple, or if it has collided with a danger apple,
*the mechanics of these two are very similar, where one adds to the snake, the other resets the game and starts a new game
*also if none of the conditions are met, it moves to the else statement which tells it that for each int i, that loops, the snake is itself
* and that to increment the noodles reference position with each section of its body.
*/
void moveCheck(void) {
   for (int i = 0; i < len; i++) {
  if (positionX == noodleX[i] && positionY == noodleY[i]) { // This checks to see if the movement of the snake is hitting itself,
   death();   // if so, then it prompts the Game Over Screens, and does a system rst.
   resetSnek();
   reset(); // this is in reference to the software reset of the Arduino
   }
  else if (positionX == applePosX && positionY == applePosY) {                      // This increases the length of the snake if an apple is hit,
   noodleX[len] = positionX;                       // also places a new apple on the screen.
   noodleY[len] = positionY;
   len = len + 1;
   count = count + 1;
   placeFood();
  }
  else if (positionX == dangerPos_1[0] && positionY == dangerPos_1[1]) {                    // This detects if the snake hits a Danger Apple, and prompts the reset screens
   noodleX[len] = positionX;
   noodleY[len] = positionY;                    
   resetSnek();
   delay(2000);
   SCORE();                                        // also prompts the score
   reset();
  }
  else if (positionX == dangerPos_2[0] && positionY == dangerPos_2[1]) {                    // This detects if the snake hits a Danger Apple, and prompts the reset screens
   noodleX[len] = positionX;
   noodleY[len] = positionY;                    
   resetSnek();
   delay(2000);
   SCORE();                                        // also prompts the score
   reset();
  }
  else if (positionX == dangerPos_3[0] && positionY == dangerPos_3[1]) {                    // This detects if the snake hits a Danger Apple, and prompts the reset screens
   noodleX[len] = positionX;
   noodleY[len] = positionY;                    
   resetSnek();
   delay(2000);
   SCORE();                                        // also prompts the score
   reset();
  }
  else if (positionX == dangerPos_4[0] && positionY == dangerPos_4[1]) {                    // This detects if the snake hits a Danger Apple, and prompts the reset screens
   noodleX[len] = positionX;
   noodleY[len] = positionY;                    
   resetSnek();
   delay(2000);
   SCORE();                                        // also prompts the score
   reset();
  }
  else{
      for(int i = 0; i < len; i++) {                // if nothing is hit, the snake is just going to keep moving.
      noodleX[i] = noodleX[i+1];
      noodleY[i] = noodleY[i+1];
      }
      noodleX[len-1] = positionX;
      noodleY[len-1] = positionY;
  }
 }
}  
//-------------- Death State -----------------//
void death(void) {
  SCORE();                                               // This is the death screen.
  clear();                                               // uses Library provided to display characters on Matrix
  
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
void clear(void) {
  matrix.fillScreen(matrix.Color333(0, 0, 0));  
}
//------------- Draw Snake -----------------//
void drawSnake(void) {
  for (int i = 0; i < len; i++) {         // This is just adding color/drawing the snake.
    matrix.drawPixel(noodleX[i], noodleY[i], matrix.Color333(0 , 255, 255));
    matrix.drawPixel(noodleX[len-1], noodleY[len-1], matrix.Color333(255, 0, 0));
  }
}
//-------------- Draw Apple --------------//
void drawFood(void) {
  matrix.drawPixel(applePosX, applePosY, matrix.Color333(0,7,0));            // draws the apple at the generated position
}
//--------------Draw Danger Apple ---------------//
void drawDanger(void) {                 // draws the Danger Apples at the generated positions
  matrix.drawPixel(dangerPos_1[0], dangerPos_1[1], matrix.Color333(random(0,255) ,random(0,255) ,random(0,255)));
  matrix.drawPixel(dangerPos_2[0], dangerPos_2[1], matrix.Color333(random(0,255) ,random(0,255) ,random(0,255)));
  matrix.drawPixel(dangerPos_3[0], dangerPos_3[1], matrix.Color333(random(0,255) ,random(0,255) ,random(0,255)));
  matrix.drawPixel(dangerPos_4[0], dangerPos_4[1], matrix.Color333(random(0,255) ,random(0,255) ,random(0,255)));
}
//-----------------Displays # of Apples Hit----------------//
void SCORE(void) {
  clear();                 // This takes the score, and displays it.
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
void DANGER (void) {
  clear();                 // This is Scroll test for the reset screens
  int flag = 1;
  float start_time = millis();
  float buffer_time = 5000;
  while(flag){
  clear();
  matrix.setTextColor(matrix.ColorHSV(hue, 255, 255, true));
  matrix.setCursor(textX, 1);
  matrix.print(F2(str_1));
  
  if((--textX) < textMin) textX = matrix.width();
  hue+= 7;
  if(hue >= 1536){
    hue -= 1536;
  }
  if(millis()>start_time+buffer_time) {
     flag = 0; 
  }
  matrix.swapBuffers(true);
  delay(10);
  }
}
//--------------- Displays Reset Instruction Text Scroll -----------------//
void resetSnek(void) {        // This is Scroll test for the reset screens
  clear();
  int flag = 1;
  float start_time = millis();
  float buffer_time = 6000;
  
  while(flag){
    clear();
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
  if(millis()>start_time+buffer_time){      // Added a counter so the scroll method could be broken out of after a certain 
     flag = 0;                              // ammount of time
  }
  matrix.swapBuffers(true);
  delay(10);
  }
}
