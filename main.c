#include "main.h"

#include <stdio.h>
#include <stdlib.h>

#include "gba.h"
/* TODO: */
// Include any header files for title screen or exit
// screen images generated by nin10kit. Example for the provided garbage
// image:
#include "images/garbage.h"
#include "images/gt.h"
#include "images/garbagedog.h" // LOSE screen image (THE GARBAGE DOG)
#include "images/win.h" // WIN screen image
#include "images/gtngarbage.h" // start image
#include "images/football.h"  // added football image

/* TODO: */
// Add any additional states you need for your app. You are not requried to use
// these specific provided states.
enum gba_state {
  START,
  PLAY,
  WIN,
  LOSE,
};

// Garbage struct for position
typedef struct {
  int row;
  int col;
  int width;
  int height;
  const unsigned short *image;
} Garbage;

#define NUM_FOOTBALLS 5

/* TODO: */
// collision checker
int checkCollision(Garbage a, Garbage b) {
  return a.row < b.row + b.height &&
         a.row + a.height > b.row &&
         a.col < b.col + b.width &&
         a.col + a.width > b.col;
}

int main(void) {
  /* TODO: */
  // Manipulate REG_DISPCNT here to set Mode 3. //
  REG_DISPCNT = MODE3 | BG2_ENABLE;

  // Save current and previous state of button input.
  u32 previousButtons = BUTTONS;
  u32 currentButtons = BUTTONS;

  // Load initial application state
  enum gba_state state = START;

  // Initialize player
  Garbage player = { .row = 64, .col = 104, .width = GT_WIDTH, .height = GT_HEIGHT, .image = gt };

  // Initialize obstacle (garbage)
  Garbage obstacle = { .row = 40, .col = 0, .width = GARBAGE_WIDTH, .height = GARBAGE_HEIGHT, .image = garbage };
  int obstacleDeltaCol = 1; // moving to right

  // Initialize score
  int score = 0;

  // Declare footballs array
  Garbage footballs[NUM_FOOTBALLS];

  // Initialize time counter
  u32 time = 0; // Time in frames

  while (1) {
    currentButtons = BUTTONS; // Load the current state of the buttons

    switch (state) {
      case START:
        waitForVBlank();
        drawFullScreenImageDMA(gtngarbage); // Drawing the start screen image
        drawString(140, 60, "Press A to Start", WHITE);

        if (KEY_JUST_PRESSED(BUTTON_A, currentButtons, previousButtons)) {
          fillScreenDMA(BLACK);
          player.row = 64;
          player.col = 104;
          obstacle.row = 40;
          obstacle.col = 0;
          obstacleDeltaCol = 1;
          state = PLAY;
          time = 0; // Reset the time when starting a new game
          score = 0; // Reset score when starting a new game
          // Initialize football positions randomly
          for (int i = 0; i < NUM_FOOTBALLS; i++) {
            footballs[i].row = randint(0, HEIGHT - FOOTBALL_HEIGHT);
            footballs[i].col = randint(0, WIDTH - FOOTBALL_WIDTH);
            footballs[i].width = FOOTBALL_WIDTH;
            footballs[i].height = FOOTBALL_HEIGHT;
            footballs[i].image = football;
          }
        }
        break;

      case PLAY:
        waitForVBlank();
        fillScreenDMA(BLACK);  // Clear the entire screen

        // Move the player with D-Pad
        if (KEY_DOWN(BUTTON_UP, currentButtons) && player.row > 0)
          player.row--;
        if (KEY_DOWN(BUTTON_DOWN, currentButtons) && player.row < HEIGHT - player.height)
          player.row++;
        if (KEY_DOWN(BUTTON_LEFT, currentButtons) && player.col > 0)
          player.col--;
        if (KEY_DOWN(BUTTON_RIGHT, currentButtons) && player.col < WIDTH - player.width)
          player.col++;

        // Move obstacle left and right
        obstacle.col += obstacleDeltaCol;
        if (obstacle.col <= 0 || obstacle.col >= WIDTH - obstacle.width) {
          obstacleDeltaCol = -obstacleDeltaCol;
        }

        // Check collision for each football and mark as collected immediately if collision occurs
        for (int i = 0; i < NUM_FOOTBALLS; i++) {
          if (footballs[i].row >= 0 && checkCollision(player, footballs[i])) {
            score++;  // Increment score when colliding with football
            footballs[i].row = -footballs[i].height;  // Remove the football completely
          }
        }
        // Draw only the footballs that haven't been collected
        for (int i = 0; i < NUM_FOOTBALLS; i++) {
          if (footballs[i].row >= 0) {
            drawImageDMA(footballs[i].row, footballs[i].col, footballs[i].width, footballs[i].height, footballs[i].image);
          }
        }

        // Draw obstacle and player
        drawImageDMA(obstacle.row, obstacle.col, obstacle.width, obstacle.height, obstacle.image);
        drawImageDMA(player.row, player.col, player.width, player.height, player.image);

        // Always draw "Now Playing..." text
        drawString(5, 5, "Now Playing...", WHITE);

        // Always draw time text (even though it updates every frame)
        {
          char timeText[50];
          int seconds = time / 60;
          sprintf(timeText, "Time: %d", seconds);
          // Clear the text area for time before drawing
          drawRectDMA(HEIGHT - 16, 5, 80, 8, BLACK);
          drawString(HEIGHT - 16, 5, timeText, WHITE);
        }

        // Optional win condition: if player moves to top right corner
        if (player.row < 10 && player.col > 200) {
          state = WIN;
        }

        // Collision detection: player and obstacle (garbage)
        if (checkCollision(player, obstacle)) {
          state = LOSE;
        }

        // Increment time
        time++;

        break;

      case WIN:
        waitForVBlank();
        drawFullScreenImageDMA(win); // Drawing the win image
        drawCenteredString(50, 5, WIDTH, HEIGHT, "You Win!", BLACK);
        {
          char scoreText[50];
          sprintf(scoreText, "Score: %d", score);
          drawString(5, 5, scoreText, WHITE);
        }
        if (KEY_JUST_PRESSED(BUTTON_A, currentButtons, previousButtons)) {
          state = START;
        }
        break;

      case LOSE:
        waitForVBlank();
        drawFullScreenImageDMA(garbagedog); // LOSE image
        drawCenteredString(50, 5, WIDTH, HEIGHT, "Game Over!", RED);
        {
          char scoreText[50];
          sprintf(scoreText, "Score: %d", score);
          drawString(5, 5, scoreText, WHITE);
        }
        if (KEY_JUST_PRESSED(BUTTON_A, currentButtons, previousButtons)) {
          state = START;
        }
        break;
    }

    // Reset to START with SELECT
    if (KEY_JUST_PRESSED(BUTTON_SELECT, currentButtons, previousButtons)) {
      state = START;
    }

    previousButtons = currentButtons; // Store the current state of the buttons
  }

  UNUSED(previousButtons);

  return 0;
}
