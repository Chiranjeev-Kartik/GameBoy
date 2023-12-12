/*
Author: Kartikay Chiranjeev Gupta
Last Modified: 24-05-23

Acknowledgements:
- Code snippets borrowed from various sources as indicated below:

1. Rui Santos
   - Title/Description: Pong
   - Source: https://randomnerdtutorials.com/

2. itdxer
   - Title/Description: ArduinoGame (Space Invaders)
   - Source: https://github.com/itdxer/ArduinoGame
*/

#include <LedControl.h>
// define the LED matrix pins
// DATAIN - 12
// CLOCK - 11
// CS - 10
// Operating at 5V
LedControl lc = LedControl(12, 11, 10, 1);  // initialize the LED matrix library

#define BUTTON_1 2
#define BUTTON_2 3
#define BUTTON_3 4
#define BUTTON_4 5
#define BUTTON_5 6
#define BUTTON_6 7
#define BUTTON_7 8
#define BUTTON_8 9
#define POTENT_1 A0
#define POTENT_2 A1
#define BUZZER A4
#define BATT_PIN A5
#define BATT_BUTTON 13


#define GAME_1 0
#define GAME_2 1
#define GAME_3 2

int INTENSITY = 1;
int SPEED = 150;       // Speed of Menu
int GAME_SPEED = 150;  // Speed of Games
int SCORE = 0;
int currentgame = 0;
bool selected = false;
float voltage, bat_percentage;
const float voltageMax = 8.1;  // Maximum battery voltage
const float voltageMin = 5.6;  // Minimum battery voltage
int sensorValue = 0;



// SPRITES
byte animation[4][8] = { { B11111111, B11111111, B11111111, B11111111, B11111111, B11111111, B11111111, B11111111 }, { B11100111, B11100111, B11100111, B11100111, B11100111, B11100111, B11100111, B11100111 }, { B11000011, B11000011, B11000011, B11000011, B11000011, B11000011, B11000011, B11000011 }, { B10000001, B10000001, B10000001, B10000001, B10000001, B10000001, B10000001, B10000001 } };
byte bat_animation[5][8] = { { B11111111, B10000001, B10000001, B10000001, B10000001, B10000001, B10000001, B11111111 }, { B00000000, B01111110, B01000010, B01000010, B01000010, B01000010, B01111110, B00000000 }, { B00000000, B00000000, B00111100, B00100100, B00100100, B00111100, B00000000, B00000000 }, { B00000000, B00000000, B00000000, B00011000, B00011000, B00000000, B00000000, B00000000 }, { B10000001, B01000010, B00100100, B00011000, B00011000, B00100100, B01000010, B10000001 } };
byte logoPong[] = { B11111100, B11111100, B00000000, B00011000, B00011000, B00000000, B00111111, B00111111 };
byte logoSnake[] = { B01110111, B01010101, B01010101, B01011101, B01000000, B00000000, B01000000, B00000000 };
byte logoSpaceInvaders[] = { B00000111, B00000111, B00000011, B11001011, B10000001, B11010011, B00000111, B00000111 };
//TIME
int OVER_TIME = 3000;
int noteDuration = 250;   // The duration of each note in milliseconds
int pauseDuration = 150;  // The duration of the pause between each note in milliseconds
// SOUND
int game_over[] = { 220, 196, 175, 165, 147, 131 };
int game_start[] = { 220, 294, 330, 392, 440, 523, 587, 659, 784, 880 };
int select_sound = 1200;
int ball_sound = 300;
int eat_sound = 600;
int shoot_sound = 150;

//----------------------------------------------------------------------------------------------------------------------------Assets of SPACE INVADER

byte EMPTY_GRID[8] = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
};

// Rockets
const int MAX_NUMBER_OF_ROCKETS = 20;
const int ALL_ROCKETS_HAVE_BEEN_USED = -1;
bool GAME_OVER = false;

struct Rocket {
  int coordX;
  int coordY;
  long unsigned updateTime;

  bool doesHitTarget(int, int);
};

Rocket ROCKET_NONE = { 0, 0, 0 };
Rocket activeRockets[MAX_NUMBER_OF_ROCKETS] = {};

int findRocketID() {
  for (int i = 0; i < MAX_NUMBER_OF_ROCKETS; i++) {
    if (isRocketNone(activeRockets[i])) {
      return i;
    }
  }

  return ALL_ROCKETS_HAVE_BEEN_USED;
}

bool isRocketNone(Rocket rocket) {
  return (
    rocket.coordX == ROCKET_NONE.coordX && rocket.coordY == ROCKET_NONE.coordY && rocket.updateTime == ROCKET_NONE.updateTime);
}

void deleteRocket(int rocketID) {
  activeRockets[rocketID] = ROCKET_NONE;
}

bool Rocket::doesHitTarget(int targetCoordX, int targetCoordY) {
  return (
    coordX == targetCoordX && coordY == targetCoordY);
}

class Spaceship {
public:
  int coordX;
  int movementSpeed;
  int gunReloadingSpeed;

  bool canMoveLeft();
  bool canMoveRight();
  bool canShoot();

  bool isTouchingCoords(int, int);

  void moveLeft();
  void moveRight();
  void shootLeft();
  void shootRight();

private:
  long unsigned int lastMovementTime;
  long unsigned int lastGunReloadingTime;
};

bool Spaceship::canMoveLeft() {
  return (
    coordX > 1 && (millis() - lastMovementTime) > movementSpeed);
}
bool Spaceship::canMoveRight() {
  return (
    coordX < 6 && (millis() - lastMovementTime) > movementSpeed);
}
bool Spaceship::canShoot() {
  return (millis() - lastGunReloadingTime) > gunReloadingSpeed;
}

void Spaceship::shootLeft() {
  int rocketID = findRocketID();
  if (rocketID != ALL_ROCKETS_HAVE_BEEN_USED) {
    activeRockets[rocketID] = (Rocket){ coordX - 1, 2, millis() };
  }
  lastGunReloadingTime = millis();
}

void Spaceship::shootRight() {
  int rocketID = findRocketID();
  if (rocketID != ALL_ROCKETS_HAVE_BEEN_USED) {
    activeRockets[rocketID] = (Rocket){ coordX + 1, 2, millis() };
  }
  lastGunReloadingTime = millis();
}

bool Spaceship::isTouchingCoords(int placeCoordX, int placeCoordY) {
  bool isTouchingRightSide = (placeCoordX == coordX + 1 && (placeCoordY == 1 || placeCoordY == 2));
  bool isTouchingLeftSide = (placeCoordX == coordX - 1 && (placeCoordY == 1 || placeCoordY == 2));
  bool isTouchingFrontSide = (placeCoordX == coordX && placeCoordY == 1);
  return isTouchingRightSide || isTouchingLeftSide || isTouchingFrontSide;
}

void Spaceship::moveLeft() {
  if (coordX > 1) {
    coordX -= 1;
  }
  lastMovementTime = millis();
}

void Spaceship::moveRight() {
  if (coordX < 6) {
    coordX += 1;
  }
  lastMovementTime = millis();
}
Spaceship spaceship;

const int MAX_NUMBER_OF_METEORS = 64;
const int ALL_MOTEORS_HAVE_BEEN_USED = -1;

class Meteor {
public:
  int coordX;
  int coordY = 7;  // Always starts from the top part of the screen
  int movementSpeed = 2000;
  bool canMove();
  void moveForward();
private:
  long unsigned int lastMovementTime = 0;
};
bool Meteor::canMove() {
  return (millis() - lastMovementTime) > movementSpeed;
}
Meteor liveMeteors[MAX_NUMBER_OF_METEORS] = {};
bool occupiedMeteorPlace[MAX_NUMBER_OF_METEORS] = {};

void Meteor::moveForward() {
  int newCoordX = coordX + random(1);      // Move +1 forward or do not move at all
  int newCoordY = coordY + random(2) - 1;  // Move left, right or do not move at all
  bool isNewPositionOccupied = false;
  Meteor meteor;
  for (int meteorID = 0; meteorID < MAX_NUMBER_OF_METEORS; meteorID++) {
    if (occupiedMeteorPlace[meteorID]) {
      meteor = liveMeteors[meteorID];
      if (meteor.coordX == newCoordX && meteor.coordY == newCoordY) {
        isNewPositionOccupied = true;
        break;
      }
    }
  }
  if (!isNewPositionOccupied) {
    coordX = newCoordX;
    coordY = newCoordY;
    lastMovementTime = millis();
  }
}

long lastCreatedMeteorTime = millis();
int createMeteorSpeed = 1000;
void createMeteor() {
  if ((millis() - lastCreatedMeteorTime) < createMeteorSpeed) {
    return;
  }
  Meteor meteor;
  meteor.coordX = random(8);
  for (int meteorID = 0; meteorID < MAX_NUMBER_OF_METEORS; meteorID++) {
    if (!occupiedMeteorPlace[meteorID]) {
      occupiedMeteorPlace[meteorID] = true;
      liveMeteors[meteorID] = meteor;
      break;
    }
  }
}

byte currentGrid[8] = {};
byte meteorGrid[8] = {};
void drawGrid(byte* grid) {
  for (int row = 0; row < 8; row++) {
    lc.setColumn(0, row, grid[row]);
  }
}
void drawSpaceship(int coord) {
  int shift = coord - 1;
  currentGrid[1] = B10100000 >> shift;
  currentGrid[0] = B11100000 >> shift;
}
void drawRockets() {
  Rocket rocket;
  for (int rocketID = 0; rocketID < MAX_NUMBER_OF_ROCKETS; rocketID++) {
    rocket = activeRockets[rocketID];
    if (!isRocketNone(rocket)) {
      lc.setLed(0, rocket.coordX, rocket.coordY, true);
      if (rocket.coordY < 8) {
        activeRockets[rocketID].coordY += 1;
        activeRockets[rocketID].updateTime = millis();
      } else {
        deleteRocket(rocketID);
      }
    }
  }
}

void drawMeteors() {
  for (int meteorID = 0; meteorID < MAX_NUMBER_OF_METEORS; meteorID++) {
    Meteor meteor = liveMeteors[meteorID];
    if (occupiedMeteorPlace[meteorID]) {
      lc.setLed(0, meteor.coordX, meteor.coordY, true);
      if (meteor.canMove()) {
        liveMeteors[meteorID].moveForward();
        if (meteor.coordX < 0 || meteor.coordX > 7 || meteor.coordY < 0) {
          occupiedMeteorPlace[meteorID] = false;
        }
      }
    }
  }
}

bool isButtonPressed(int buttonID) {
  int buttonState = digitalRead(buttonID);
  return buttonState == LOW;
}

bool checkIfMeteorTouchedSpaceship() {
  for (int meteorID = 0; meteorID < MAX_NUMBER_OF_METEORS; meteorID++) {
    Meteor meteor = liveMeteors[meteorID];

    if (occupiedMeteorPlace[meteorID] && spaceship.isTouchingCoords(meteor.coordX, meteor.coordY)) {
      //GAME_OVER = true;
      return true;
    }
  }
  return false;
}
void checkIfSpaceshipHitMeteor() {
  Rocket rocket;
  Meteor meteor;

  for (int rocketID = 0; rocketID < MAX_NUMBER_OF_ROCKETS; rocketID++) {
    rocket = activeRockets[rocketID];

    if (!isRocketNone(rocket)) {
      for (int meteorID = 0; meteorID < MAX_NUMBER_OF_METEORS; meteorID++) {
        meteor = liveMeteors[meteorID];

        if (occupiedMeteorPlace[meteorID] && rocket.doesHitTarget(meteor.coordX, meteor.coordY)) {
          occupiedMeteorPlace[meteorID] = false;
          tone(BUZZER, shoot_sound, 30);
        }
      }
    }
  }
}

void ReadPotentiometers() {
  INTENSITY = map(analogRead(POTENT_1), 0, 1023, 0, 15);
  lc.setIntensity(0, INTENSITY);
  GAME_SPEED = map(analogRead(POTENT_2), 0, 1023, 5, 50) * 10;
}

void reset() {
  drawGrid(EMPTY_GRID);
  // Set up default position for the spaceship
  spaceship.coordX = 4;
  spaceship.movementSpeed = 50;       // The lower - the faster.
  spaceship.gunReloadingSpeed = 200;  // The lower - the faster.

  // Clear all meteors
  for (int meteorID = 0; meteorID < MAX_NUMBER_OF_METEORS; meteorID++) {
    occupiedMeteorPlace[meteorID] = false;
  }
  // Clear all spaceship rockets
  for (int rocketID = 0; rocketID < MAX_NUMBER_OF_ROCKETS; rocketID++) {
    deleteRocket(rocketID);
  }
}

//-------------------------------------------------------------------------------------------------------------------------------Assets of SNAKE game
int snake[64][2];
int snakeX = 3;
int snakeY = 3;
int snakeDir = 0;  // 0 for right, 1 for down, 2 for left, 3 for up
int snakeLen = 1;
int foodX = 0;
int foodY = 0;

void generateFood() {
  // generate a random food location
  foodX = random(7);
  foodY = random(7);
  // check if the food is on the snake body
  for (int i = 0; i < snakeLen; i++) {
    if (foodX == snake[i][0] && foodY == snake[i][1]) {
      generateFood();
      return;
    }
  }
}

//--------------------------------------------------------------------------------------------------------------------------------Assets of PONG Game
const int RoCo = 7;
const int PADDLE_HEIGHT = 3;
int _wall[] = { 3, 3 };
unsigned int BALL_X = 4;
unsigned int BALL_Y = 4;
unsigned int ANGLE = 180;
const int ADDR = 0;
bool PONG_OVER = false;


void retorted(int angle) {
  ANGLE = angle;
}

bool checkCollision() {
  if (BALL_X == RoCo - 1) {
    tone(BUZZER, ball_sound, 30);
    if (ANGLE == 315 || ANGLE == 0 || ANGLE == 45) {
      if (BALL_Y == _wall[1] || BALL_Y == _wall[1] + 1 || BALL_Y == _wall[1] + 2) {
        if (ANGLE == 0 && BALL_Y == _wall[1]) retorted(225);
        else if (ANGLE == 0 && BALL_Y == _wall[1] + 2) retorted(135);
        else if (ANGLE == 45 && BALL_Y == _wall[1]) retorted(135);
        else if (ANGLE == 45 && BALL_Y == _wall[1] + 1) retorted(180);
        else if (ANGLE == 315 && BALL_Y == _wall[1]) retorted(180);
        else if (ANGLE == 315 && BALL_Y == _wall[1] + 2) retorted(225);
      }
    }
  } else if (BALL_X == 1) {
    tone(BUZZER, ball_sound, 30);
    if (ANGLE == 225 || ANGLE == 180 || ANGLE == 135) {
      if (BALL_Y == _wall[0] || BALL_Y == _wall[0] + 1|| BALL_Y == _wall[0] + 2) {
        if (ANGLE == 180 && BALL_Y == _wall[0]) retorted(315);
        else if (ANGLE == 180 && BALL_Y == _wall[0] + 2) retorted(45);
        else if (ANGLE == 135 && BALL_Y == _wall[0]) retorted(45);
        else if (ANGLE == 135 && BALL_Y == _wall[0] + 1) retorted(0);
        else if (ANGLE == 225 && BALL_Y == _wall[0]) retorted(0);
        else if (ANGLE == 225 && BALL_Y == _wall[0] + 2) retorted(315);
      }
    }
  }
  if (BALL_X == RoCo) {
    gameOver();
    PONG_OVER = true;
    return true;
  } else if (BALL_X == 0) {
    gameOver();
    PONG_OVER = true;
    return true;
  } else if (BALL_Y == RoCo) {
    if (ANGLE == 45) ANGLE = 315;
    else if (ANGLE == 135) ANGLE = 225;
  } else if (BALL_Y == 0) {
    if (ANGLE == 225) ANGLE = 135;
    else if (ANGLE == 315) ANGLE = 45;
  }
  return false;
}

void calcANGELIncrement() {
  if (ANGLE == 0 || ANGLE == 360) {
    BALL_X += 1;
  } else if (ANGLE == 45) {
    BALL_X += 1;
    BALL_Y += 1;
  } else if (ANGLE == 135) {
    BALL_X -= 1;
    BALL_Y += 1;
  } else if (ANGLE == 180) {
    BALL_X -= 1;
  } else if (ANGLE == 225) {
    BALL_X -= 1;
    BALL_Y -= 1;
  } else if (ANGLE == 315) {
    BALL_X += 1;
    BALL_Y -= 1;
  }
}
void calcWall() {
  if (digitalRead(BUTTON_1) == LOW) {           // button for wall 1 moved down
    if (_wall[0] < 8 - PADDLE_HEIGHT) {  // check if wall can be moved down
      _wall[0]++;
    }
  }
  if (digitalRead(BUTTON_3) == LOW) {  // button for wall 1 moved up
    if (_wall[0] > 0) {         // check if wall can be moved up
      _wall[0]--;
    }
  }
  if (digitalRead(BUTTON_5) == LOW) {           // button for wall 2 moved down
    if (_wall[1] < 8 - PADDLE_HEIGHT) {  // check if wall can be moved down
      _wall[1]++;
    }
  }
  if (digitalRead(BUTTON_7) == LOW) {  // button for wall 2 moved up
    if (_wall[1] > 0) {         // check if wall can be moved up
      _wall[1]--;
    }
  }
}

int enterFrameHandler() {
  if (checkCollision()) {
    return 0;
  }
  calcANGELIncrement();
  lc.clearDisplay(0);
  drawPaddle(0, _wall[0], true);
  drawPaddle(7, _wall[1], true);
  drawBall(BALL_X, BALL_Y, true);
}
void drawPaddle(int x, int y, bool draw) {
  for (int i = y; i < y + PADDLE_HEIGHT; i++) {
    lc.setLed(ADDR, x, i, draw);
  }
}

void drawBall(int x, int y, bool draw) {
  lc.setLed(ADDR, x, y, draw);
}

// GAME-FUNCTIONS

void start_animation(byte arr[4][8]) {
  for (int i = 0; i < 4; i++) {
    for (int j = 7; j >= 0; j--) {
      lc.setColumn(0, j, arr[i][j]);
      delay(50);
    }
  }
  lc.clearDisplay(0);
}

void gameOver() {
  playSound(game_over, 6);
  // display the score on the LED matrix
  delay(OVER_TIME);
  lc.clearDisplay(0);
  snakeX = 3;
  snakeY = 3;
  snakeLen = 1;

  BALL_X = random(3, 5);
  BALL_Y = random(3, 5);
  ANGLE = random(0, 2) == 0 ? 0 : 180;
  _wall[0] = 3;
  _wall[1] = 3;
}

void playSound(int sound[], int n) {
  for (int i = 0; i < n; i++) {
    tone(BUZZER, sound[i], noteDuration);
    delay(pauseDuration);
  }
}

void displaySprite(byte arr[]) {
  for (int i = 0; i < 8; i++) {
    lc.setRow(0, i, arr[i]);
  }
}

void bat_checker() {
sensorValue = 0.0;
for (int i = 0; i<10; i++ ) {
  sensorValue += analogRead(BATT_PIN);
}
sensorValue = sensorValue/10;
  voltage = ((sensorValue / 1023.0) * 5.0) * 2.0;                               // Calculate battery voltage
  bat_percentage = (voltage - voltageMin) / (voltageMax - voltageMin) * 100.0;  // Calculate battery percentage

  if (bat_percentage >= 100.0) {
    bat_percentage = 100.0;
  } else if (bat_percentage <= 0.0) {
    bat_percentage = 0.0;
  }
  if (bat_percentage > 80 && bat_percentage <= 100) {
    displaySprite(bat_animation[0]);
  } else if (bat_percentage > 60 && bat_percentage <= 80) {
    displaySprite(bat_animation[1]);
  } else if (bat_percentage > 40 && bat_percentage <= 60) {
    displaySprite(bat_animation[2]);
  } else if (bat_percentage > 20 && bat_percentage <= 40) {
    displaySprite(bat_animation[3]);
  } else if (bat_percentage > 0 && bat_percentage <= 20) {
    displaySprite(bat_animation[4]);
  }
}
// display the menu on the LED matrix

int menu() {
  while (true) {
    if (digitalRead(BATT_BUTTON) == HIGH) {
      bat_checker();
      delay(1000);
    }

    lc.clearDisplay(0);
    if (currentgame == 0) {
      displaySprite(logoPong);
    } else if (currentgame == 1) {
      displaySprite(logoSnake);
    } else if (currentgame == 2) {
      displaySprite(logoSpaceInvaders);
    }
    if (digitalRead(BUTTON_2) == LOW || digitalRead(BUTTON_6) == LOW) {
      tone(BUZZER, select_sound, 30);
      if (currentgame < 2) {
        currentgame += 1;
      } else {
        currentgame = 0;
      }
    } else if (digitalRead(BUTTON_4) == LOW || digitalRead(BUTTON_8) == LOW) {
      tone(BUZZER, select_sound, 30);
      if (currentgame > 0) {
        currentgame -= 1;
      } else {
        currentgame = 2;
      }
    } else if (digitalRead(BUTTON_1) == LOW || digitalRead(BUTTON_3) == LOW || digitalRead(BUTTON_5) == LOW || digitalRead(BUTTON_7) == LOW) {
      tone(BUZZER, select_sound, 30);
      selected = true;
      return 0;
    }
    ReadPotentiometers();
    delay(SPEED);
  }
}

int GameBoyStart() {
  playSound(game_start, 10);
  start_animation(animation);
}

// game 1 function
int game1() {
  PONG_OVER = false;
  while (!PONG_OVER) {
    calcWall();
    enterFrameHandler();
    ReadPotentiometers();
    delay(GAME_SPEED);
  }
  return 0;
}
// game 2 function
int game2() {
  generateFood();
  while (true) {
    // update the direction of the snake based on the push-buttons
    if ((digitalRead(BUTTON_3) == LOW || digitalRead(BUTTON_7) == LOW) && snakeDir != 1) {
      snakeDir = 3;
    }
    if ((digitalRead(BUTTON_4) == LOW || digitalRead(BUTTON_8) == LOW) && snakeDir != 0) {
      snakeDir = 2;
    }
    if ((digitalRead(BUTTON_2) == LOW || digitalRead(BUTTON_6) == LOW) && snakeDir != 2) {
      snakeDir = 0;
    }
    if ((digitalRead(BUTTON_1) == LOW || digitalRead(BUTTON_5) == LOW) && snakeDir != 3) {
      snakeDir = 1;
    }

    // update the position of the snake
    switch (snakeDir) {
      case 0:  // right
        snakeX++;
        break;
      case 1:  // down
        snakeY++;
        break;
      case 2:  // left
        snakeX--;
        break;
      case 3:  // up
        snakeY--;
        break;
    }

    // check for collisions with the walls
    if (snakeX < 0 || snakeX >= 8 || snakeY < 0 || snakeY >= 8) {
      gameOver();
      return 0;
    }

    // check for collisions with the snake body
    for (int i = 0; i < snakeLen; i++) {
      if (snakeX == snake[i][0] && snakeY == snake[i][1]) {
        gameOver();
        return 0;
      }
    }

    // check for collisions with the food
    if (snakeX == foodX && snakeY == foodY) {
      //score++;
      generateFood();
      tone(BUZZER, eat_sound, 30);
      snakeLen++;
    }
    // update the snake body
    for (int i = snakeLen - 1; i > 0; i--) {
      snake[i][0] = snake[i - 1][0];
      snake[i][1] = snake[i - 1][1];
    }
    snake[0][0] = snakeX;
    snake[0][1] = snakeY;

    // update the LED matrix display
    lc.clearDisplay(0);
    lc.setLed(0, foodX, foodY, true);
    for (int i = 0; i < snakeLen; i++) {
      lc.setLed(0, snake[i][0], snake[i][1], true);
    }
    // delay the program based on the current speed
    ReadPotentiometers();
    delay(GAME_SPEED);
  }
}

// game 3 function
int game3() {
  reset();
  GAME_OVER = false;
  while (!GAME_OVER) {
    if (isButtonPressed(BUTTON_4) && spaceship.canMoveLeft()) {
      spaceship.moveLeft();
    }

    if (isButtonPressed(BUTTON_2) && spaceship.canMoveRight()) {
      spaceship.moveRight();
    }

    if (isButtonPressed(BUTTON_8) && spaceship.canShoot()) {
      spaceship.shootLeft();
    }

    if (isButtonPressed(BUTTON_6) && spaceship.canShoot()) {
      spaceship.shootRight();
    }

    if (random(10) < 3) {
      createMeteor();
    }

    memcpy(currentGrid, EMPTY_GRID, 8);

    drawSpaceship(spaceship.coordX);
    drawGrid(currentGrid);
    drawRockets();
    drawMeteors();

    if (checkIfMeteorTouchedSpaceship()) {
      playSound(game_over, 6);
      delay(OVER_TIME);
      return 0;
    }
    checkIfSpaceshipHitMeteor();
    ReadPotentiometers();
    delay(GAME_SPEED);
  }
}

// setup function
void setup() {
  // set up the LED matrix
  lc.shutdown(0, false);          // The MAX72XX is in power-saving mode on startup, we have to do a wakeup call
  lc.setIntensity(0, INTENSITY);  // Set the brightness (0-15)
  lc.clearDisplay(0);             // Clear Display
  Serial.begin(9600);
  GameBoyStart();
  // set up the pushbuttons
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  pinMode(BUTTON_3, INPUT_PULLUP);
  pinMode(BUTTON_4, INPUT_PULLUP);
  pinMode(BUTTON_5, INPUT_PULLUP);
  pinMode(BUTTON_6, INPUT_PULLUP);
  pinMode(BUTTON_7, INPUT_PULLUP);
  pinMode(BUTTON_8, INPUT_PULLUP);
  pinMode(BATT_BUTTON, INPUT);
}

// loop function
void loop() {
  if (!selected) {
    menu();
  } else {
    switch (currentgame) {
      case GAME_1:
        SCORE = 0;
        game1();
        currentgame = 0;
        selected = false;
        break;
      case GAME_2:
        SCORE = 0;
        game2();
        currentgame = 0;
        selected = false;
        break;
      case GAME_3:
        SCORE = 0;
        game3();
        currentgame = 0;
        selected = false;
        break;
    }
  }
  delay(SPEED);
}
