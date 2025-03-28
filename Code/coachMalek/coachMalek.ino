#include <LiquidCrystal.h>

// LCD Pins
#define LCD_RS 12
#define LCD_E  11
#define LCD_D4 5
#define LCD_D5 4
#define LCD_D6 3
#define LCD_D7 2
#define leftButton 8
#define rightButton 9
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// Joystick Pins (analog values)
#define JOY_X A0  // Horizontal movement
#define JOY_Y A1  // Vertical movement
// These thresholds are chosen based on typical center ~512 reading.
// If JOY_X is greater than JOY_HIGH, joystick is pushed right.
// If JOY_X is less than JOY_LOW, joystick is pushed left.
// Same idea for the Y-axis.
#define JOY_LOW 423   
#define JOY_HIGH 600  

// Grid Size (LCD is 16 columns x 2 rows)
#define GRID_WIDTH 16
#define GRID_HEIGHT 2

// Game Constants
const unsigned long delayTime = 1000;         // Generic message delay (ms)
const unsigned long levelUpDelay = 2000;        // Extended delay for LEVEL UP (ms)
const unsigned long baseMoveInterval = 150;     // Initial speed
const int buttonLeft = rightButton;
const int buttonRight = leftButton;

// Debounce delay in milliseconds
const unsigned long debounceDelay = 80;

// Variables to store the last time a button was pressed
unsigned long lastLeftPressTime = 0;
unsigned long lastRightPressTime = 0;

// Variables to store the previous state of the buttons
bool prevLeftState = HIGH;  // Assuming buttons are pulled HIGH when not pressed
bool prevRightState = HIGH;

// The barrier's position (0 to 15, start in the middle)
int barrierPos = 8;
int prevBarrierPos = 8;

// Variables for timing win message
unsigned long winStartTime = 0;
bool showWinMessage = false;
String winMessage = "";

// Timing for "Game Start" message
unsigned long gameStartTime = 0;
bool gameStarted = true;
// --------------------
// Game Mode selection
// --------------------
enum GameMode { MENU, SNAKE, DINOSAUR, BARRIER };
GameMode gameMode = MENU;

// --------------------
// SNAKE GAME VARIABLES
// --------------------
struct Snake {
  int x[20], y[20];
  int length;
  int direction; // 1: up, 2: down, 3: left, 4: right
} snake;

struct Food {
  int x, y;
} food;

enum SnakeState {
  SNAKE_START_MESSAGE,
  SNAKE_RUNNING,
  SNAKE_LEVEL_UP,
  SNAKE_LOST_SHOW,
  SNAKE_LOST_GAME_OVER,
  SNAKE_LOST_COACH,
  SNAKE_LOST_START
} snakeState;

unsigned long snakeStateTime = 0;
unsigned long snakeLastMoveTime = 0;
unsigned long snakeMoveInterval = baseMoveInterval;
int snakeApplesEaten = 0;
int snakeLevel = 1;
const int snakeLevelGoals[4] = {5, 8, 11, 15};

// -----------------------
// DINOSAUR GAME VARIABLES
// -----------------------
enum DinoState {
  DINO_START_MESSAGE,
  DINO_RUNNING,
  DINO_LEVEL_UP,
  DINO_LOST
} dinoState;

unsigned long dinoStateTime = 0;
unsigned long dinoLastMoveTime = 0;
unsigned long dinoMoveInterval = baseMoveInterval; // You can adjust for Dino game
int dinoScore = 0;
int dinoLevel = 1;
const int dinoLevelGoals[4] = {5, 8, 11, 15};  // Number of barriers passed
int dinoY = 1;                     // Dinosaur's row; default bottom (1)
int dinoObstacleX = GRID_WIDTH - 1; // Obstacle starts at right edge
int dinoObstacleY = 1;             // Obstacle row (can be 0 or 1)
unsigned long dinoJumpTime = 0;    // Time when the dinosaur started jumping
const unsigned long dinoJumpDuration = 200; // Duration (ms) for which Dino stays up

// -----------------------
// Function Declarations
// -----------------------
void menuLoop();
void snakeLoop();
void dinoLoop();
void Loop();
// --- Snake Game Functions ---
void resetSnakeGame(bool fullReset = true);
void snakeHandleInput();
void snakeMove();
bool snakeCheckCollision();
void snakeGenerateFood();
void snakeUpdateLCD();
void snakeDisplayGrid();
void snakeLevelUp();
void snakeNextLevel();

// --- Dino Game Functions ---
void resetDinoGame(bool fullReset = true);
void dinoHandleInput();
void dinoMoveObstacle();
bool dinoCheckCollision();
void dinoUpdateLCD();
void dinoDisplayGrid();
void dinoLevelUp();
void dinoNextLevel();

void updateBarrier();
void gameStart();
// Helper: Reads the joystick and returns a direction.
// For Snake: 1=Up, 2=Down, 3=Left, 4=Right, 0=No movement.
// For Menu: Left (3) selects Snake, Right (4) selects Dino.
int getJoystickDirection() {
  int xValue = analogRead(JOY_X);
  int yValue = analogRead(JOY_Y);
  
  // Check horizontal first
  if (xValue > JOY_HIGH) {
    return 4; // Right
  } 
  else if (xValue < JOY_LOW) {
    return 3; // Left
  }
  // Then vertical
  if (yValue < JOY_LOW) {   // Pushed upward (analog value decreases)
    return 1; // Up
  } 
  else if (yValue > JOY_HIGH) {
    return 2; // Down
  }
  return 0; // No significant movement
}

// -----------------------
// Setup and Main Loop
// -----------------------
void setup() {
  pinMode(buttonLeft, INPUT_PULLUP);
  pinMode(buttonRight, INPUT_PULLUP);
  lcd.begin(16, 2);
  // No need to set pinMode for analog inputs.
  Serial.begin(9600);
  gameMode = MENU;
}

void loop() {
  switch (gameMode) {
    case MENU:
      menuLoop();
      break;
    case SNAKE:
      snakeLoop();
      break;
    case DINOSAUR:
      dinoLoop();
      break;
    case BARRIER:
      Loop();
      break;
  }
}

// -----------------------
// Menu Loop Function
// -----------------------
// In the menu, push the joystick left for Snake or right for Dino.
void menuLoop() {
  static bool menuDisplayed = false;
  if (!menuDisplayed) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("L.Snake   R.Dino");
    lcd.setCursor(0, 1);
    lcd.print("   D.Barrier|");
    menuDisplayed = true;
  }
  
  int dir = getJoystickDirection();
  if (dir == 3) { 
    gameMode = SNAKE;
    resetSnakeGame();
    delay(500);
    menuDisplayed = false;
  }
  else if (dir == 4) { 
    gameMode = DINOSAUR;
    resetDinoGame();
    delay(500);
    menuDisplayed = false;
  }
  else if (dir == 2) { // Down -> Barrier
    Serial.println("Starting Barrier Game...");
    gameMode = BARRIER;
    gameStart(); // Initialize barrier game
    delay(500);
    menuDisplayed = false;
  }
}



// -----------------------
// Snake Game Functions
// -----------------------
void snakeLoop() {
  unsigned long currentMillis = millis();
  switch (snakeState) {
    case SNAKE_START_MESSAGE:
      if (currentMillis - snakeStateTime >= delayTime) {
        snakeState = SNAKE_RUNNING;
        lcd.clear();
        snakeDisplayGrid();
      }
      break;
    case SNAKE_RUNNING:
      snakeHandleInput();
      if (currentMillis - snakeLastMoveTime > snakeMoveInterval) {
        snakeMove();
        snakeLastMoveTime = currentMillis;
        if (snakeCheckCollision()) {
          snakeState = SNAKE_LOST_SHOW;
          snakeStateTime = currentMillis;
        }
        snakeUpdateLCD();
      }
      break;
    case SNAKE_LEVEL_UP:
      if (currentMillis - snakeStateTime >= levelUpDelay) {
        snakeNextLevel();
      }
      break;
    case SNAKE_LOST_SHOW:
      if (currentMillis - snakeStateTime >= delayTime) {
        snakeState = SNAKE_LOST_GAME_OVER;
        snakeStateTime = currentMillis;
        lcd.clear();
        lcd.print("Game Over!");
      }
      break;
    case SNAKE_LOST_GAME_OVER:
      if (currentMillis - snakeStateTime >= delayTime) {
        snakeState = SNAKE_LOST_COACH;
        snakeStateTime = currentMillis;
        lcd.clear();
        lcd.print("Supervised by");
        lcd.setCursor(0, 1);
        lcd.print("Coach Malek");
      }
      break;
    case SNAKE_LOST_COACH:
      if (currentMillis - snakeStateTime >= delayTime) {
        gameMode = MENU;  // Return to the menu after showing the coach message
        lcd.clear();
        lcd.print("Returning to Menu...");
      }
      break;
    case SNAKE_LOST_START:
      if (currentMillis - snakeStateTime >= delayTime) {
        gameMode = MENU; // Return to MENU on loss.
      }
      break;
  }
}

// Replace button-based input with joystick direction.
void snakeHandleInput() {
  int dir = getJoystickDirection();
  // Check to ensure we do not reverse the snake direction
  if (dir == 1 && snake.direction != 2)
    snake.direction = 1;
  else if (dir == 2 && snake.direction != 1)
    snake.direction = 2;
  else if (dir == 3 && snake.direction != 4)
    snake.direction = 3;
  else if (dir == 4 && snake.direction != 3)
    snake.direction = 4;
}

void snakeMove() {
  for (int i = snake.length; i > 0; i--) {
    snake.x[i] = snake.x[i - 1];
    snake.y[i] = snake.y[i - 1];
  }
  switch (snake.direction) {
    case 1: snake.y[0]--; break;
    case 2: snake.y[0]++; break;
    case 3: snake.x[0]--; break;
    case 4: snake.x[0]++; break;
  }
  if (snake.x[0] < 0) snake.x[0] = GRID_WIDTH - 1;
  if (snake.x[0] >= GRID_WIDTH) snake.x[0] = 0;
  if (snake.y[0] < 0) snake.y[0] = GRID_HEIGHT - 1;
  if (snake.y[0] >= GRID_HEIGHT) snake.y[0] = 0;
  
  if (snake.x[0] == food.x && snake.y[0] == food.y) {
    snake.length++;
    snakeApplesEaten++;
    Serial.print("Snake Score: "); Serial.println(snakeApplesEaten);
    Serial.print("Level: "); Serial.println(snakeLevel);
    Serial.print("Target: "); Serial.println(snakeLevelGoals[snakeLevel - 1]);
    if (snakeApplesEaten >= snakeLevelGoals[snakeLevel - 1]) {
      snakeLevelUp();
    } else {
      snakeGenerateFood();
    }
  }
}

bool snakeCheckCollision() {
  for (int i = 1; i < snake.length; i++) {
    if (snake.x[0] == snake.x[i] && snake.y[0] == snake.y[i])
      return true;
  }
  return false;
}

void snakeGenerateFood() {
  bool valid = false;
  int rx, ry;
  while (!valid) {
    rx = random(0, GRID_WIDTH);
    ry = random(0, GRID_HEIGHT);
    valid = true;
    for (int i = 0; i < snake.length; i++) {
      if (snake.x[i] == rx && snake.y[i] == ry) {
        valid = false;
        break;
      }
    }
  }
  food.x = rx;
  food.y = ry;
}

void snakeUpdateLCD() {
  lcd.clear();
  for (int i = 0; i < snake.length; i++) {
    lcd.setCursor(snake.x[i], snake.y[i]);
    lcd.print("s");
  }
  lcd.setCursor(food.x, food.y);
  lcd.print("a");
}

void snakeDisplayGrid() {
  lcd.clear();
  lcd.setCursor(food.x, food.y);
  lcd.print("a");
}

void snakeLevelUp() {
  Serial.println("SNAKE LEVEL UP!");
  lcd.clear();
  lcd.print("LEVEL UP!");
  snakeStateTime = millis();
  snakeState = SNAKE_LEVEL_UP;
}

void snakeNextLevel() {
  if (snakeLevel < 4) {
    snakeLevel++;
    snakeMoveInterval = (snakeMoveInterval > 50) ? snakeMoveInterval - 20 : snakeMoveInterval;  // Increase speed, but limit it.

    // Display "LEVEL UP!" for longer
    lcd.clear();
    lcd.print("LEVEL UP!");
    delay(3000); // Extended delay for Level Up message

    // Display "Starting Level X"
    lcd.clear();
    lcd.print("Starting Lvl ");
    lcd.print(snakeLevel);
    delay(1000); // Shorter delay for level start message

    // Reset the snake to avoid immediate collision after level-up.
    resetSnakeGame(false);
  } else {
    lcd.clear();
    lcd.print("Game Ends!");
    lcd.setCursor(0, 1);
    lcd.print("Coach Malek");
    delay(3000); // Show final message before reset
    resetSnakeGame(true);  // Full reset after game ends.
  }
}

void resetSnakeGame(bool fullReset) {
  snake.x[0] = 0;   // Start at the top-left corner
  snake.y[0] = 0;   
  snake.direction = 4;  // Default moving right

  if (fullReset) {
    snake.length = 1; // Only reset length when fully restarting
    snakeLevel = 1;
    snakeApplesEaten = 0;
    snakeMoveInterval = baseMoveInterval; // Reset speed to initial value
  }

  snakeGenerateFood();  // Generate new food
  lcd.clear();
  lcd.print("Snake Game!");
  snakeStateTime = millis();
  snakeState = SNAKE_START_MESSAGE;
}

// -----------------------
// Dino Game Functions
// -----------------------
void dinoLoop() {
  unsigned long currentMillis = millis();
  switch (dinoState) {
    case DINO_START_MESSAGE:
      if (currentMillis - dinoStateTime >= delayTime) {
        dinoState = DINO_RUNNING;
        lcd.clear();
        dinoDisplayGrid();
      }
      break;
    case DINO_RUNNING:
      dinoHandleInput(); // Uses edge-triggered jump logic
      if (currentMillis - dinoLastMoveTime > dinoMoveInterval) {
        dinoMoveObstacle();
        dinoLastMoveTime = currentMillis;
        if (dinoCheckCollision()) {
          dinoState = DINO_LOST;
          dinoStateTime = currentMillis;
        }
        dinoUpdateLCD();
      }
      break;
    case DINO_LEVEL_UP:
      if (currentMillis - dinoStateTime >= levelUpDelay) {
        dinoNextLevel();
      }
      break;
    case DINO_LOST:
      if (currentMillis - dinoStateTime >= delayTime) {
        lcd.clear();
        lcd.print("Game Over!");
        delay(delayTime);
        // After losing in Dino, return to the menu.
        gameMode = MENU;
      }
      break;
  }
}

// For Dino, only vertical movement is needed: Up to jump, Down to force landing.
bool prevJoystickUp = false; // Track previous state of UP direction

void dinoHandleInput() {
  int dir = getJoystickDirection();
  bool currentJoystickUp = (dir == 1); // Check if joystick is currently UP

  // Jump only on rising edge (new press)
  if (currentJoystickUp && !prevJoystickUp) {
    if (dinoY == 1) { // Only jump if on the ground
      dinoY = 0;
      dinoJumpTime = millis();
      Serial.println("Dino Jump!");
    }
  }
  prevJoystickUp = currentJoystickUp; // Update previous state

  // Force dinosaur down if joystick is pressed down
  if (dir == 2) {
    dinoY = 1;
  }

  // Automatically come down after jump duration
  if (dinoY == 0 && (millis() - dinoJumpTime >= dinoJumpDuration)) {
    dinoY = 1;
  }
}

void dinoMoveObstacle() {
  dinoObstacleX--;
  if (dinoObstacleX < 0) {
    dinoObstacleX = GRID_WIDTH - 1;
    dinoObstacleY = random(0, GRID_HEIGHT);
    dinoScore++;
    Serial.print("Dino Score: "); Serial.println(dinoScore);
    Serial.print("Level: "); Serial.println(dinoLevel);
    Serial.print("Target: "); Serial.println(dinoLevelGoals[dinoLevel - 1]);
    if (dinoScore >= dinoLevelGoals[dinoLevel - 1]) {
      dinoLevelUp();
    }
  }
}

bool dinoCheckCollision() {
  // Collision: if obstacle is within the leftmost 2 columns and on the same row as dinosaur.
  if (dinoObstacleX <= 1 && dinoObstacleY == dinoY)
    return true;
  return false;
}

void dinoUpdateLCD() {
  lcd.clear();
  // Draw dinosaur at column 0.
  lcd.setCursor(0, dinoY);
  lcd.print("D");
  // Draw obstacle.
  lcd.setCursor(dinoObstacleX, dinoObstacleY);
  lcd.print("X");
}

void dinoDisplayGrid() {
  lcd.clear();
  lcd.setCursor(dinoObstacleX, dinoObstacleY);
  lcd.print("X");
  lcd.setCursor(0, dinoY);
  lcd.print("D");
}

void dinoLevelUp() {
  Serial.println("DINO LEVEL UP!");
  lcd.clear();
  lcd.print("LEVEL UP!");
  dinoStateTime = millis();
  dinoState = DINO_LEVEL_UP;
}

void dinoNextLevel() {
  if (dinoLevel < 4) {
    dinoLevel++;
    dinoMoveInterval = (dinoMoveInterval > 50) ? dinoMoveInterval - 20 : dinoMoveInterval;
    lcd.clear();
    lcd.print("Starting Lvl ");
    lcd.print(dinoLevel);
    delay(delayTime);
    resetDinoGame(false);
  } else {
    lcd.clear();
    lcd.print("Game Ends!");
    lcd.setCursor(0, 1);
    lcd.print("Coach Malek");
    delay(delayTime);
    resetDinoGame(true);
  }
}

void resetDinoGame(bool fullReset) {
  if (fullReset) {
    dinoLevel = 1;
    dinoScore = 0;
  }
  dinoY = 1; // Dinosaur at bottom
  dinoObstacleX = GRID_WIDTH - 1;
  dinoObstacleY = 1;
  dinoMoveInterval = baseMoveInterval;
  lcd.clear();
  lcd.print("Dino Game!");
  dinoStateTime = millis();
  dinoState = DINO_START_MESSAGE;
}

// -----------------------
// Barrier Game Functions
// -----------------------

void Loop() {
  unsigned long currentMillis = millis();
  
  // Read button states (active LOW due to INPUT_PULLUP)
  bool leftPressed = (digitalRead(buttonLeft) == LOW);
  bool rightPressed = (digitalRead(buttonRight) == LOW);
  
  // Check for left button press (falling edge)
  if (leftPressed && !prevLeftState && (currentMillis - lastLeftPressTime >= debounceDelay)) {
    if (barrierPos > 0) {
      barrierPos--;
      updateBarrier();
    }
    lastLeftPressTime = currentMillis;
  }

  // Check for right button press
  if (rightPressed && !prevRightState && (currentMillis - lastRightPressTime >= debounceDelay)) {
    if (barrierPos < 15) {
      barrierPos++;
      updateBarrier();
    }
    lastRightPressTime = currentMillis;
  }

  // Save current state for next loop
  prevLeftState = leftPressed;
  prevRightState = rightPressed;
  
  // Check win condition
  if (barrierPos == 0 || barrierPos == 15) {
    if (!showWinMessage) {
      winMessage = (barrierPos == 0) ? "RIGHT WIN!" : "LEFT WIN!";
      winStartTime = currentMillis;
      showWinMessage = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(winMessage);
    }
  }

  // Return to menu after win message
  if (showWinMessage && currentMillis - winStartTime >= 2000) {
    gameMode = MENU;
    showWinMessage = false;
  }
}

void gameStart() {
  barrierPos = 8;
  prevBarrierPos = 8;
  showWinMessage = false;
  lcd.clear();
  lcd.print("Game Start");
  delay(2000);
  lcd.clear();
  updateBarrier();
}

void updateBarrier() {
  if (showWinMessage) return;

  // Clear previous position
  lcd.setCursor(prevBarrierPos, 0);
  lcd.print(" ");
  
  // Draw new position
  lcd.setCursor(barrierPos, 0);
  lcd.print("|");
  
  prevBarrierPos = barrierPos;
  
  // Update instructions
  lcd.setCursor(0, 1);
  lcd.print("L:Left   R:Right");
}
