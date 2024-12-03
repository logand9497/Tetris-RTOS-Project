// G8RTOS_Threads.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for thread functions.

/************************************Includes***************************************/

#include "./threads.h"

#include "./MultimodDrivers/multimod.h"

#include "./G8RTOS/G8RTOS_IPC.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "threads.h"
#include "MultimodDrivers/multimod_ST7789.h"

// Function prototypes for game logic
static void InitializeBoard(void);
static void SpawnNewPiece(void);
static bool CanMovePiece(int newX, int newY);
static void MovePiece(int deltaX, int deltaY);
static void UpdateTetrisDisplay(void);
static void PlacePieceOnBoard(void);
static void DrawGameOverScreen(void);

/*********************************Global Variables**********************************/

// Game state semaphore
semaphore_t sem_GameState;

// Game state
static GameState gameState;

// Drop speed variable
uint32_t dropSpeed = 250;

/*********************************Global Variables**********************************/

/********************************Public Functions***********************************/

void Threads_Init(void) {
    // Initialize game state semaphore
    G8RTOS_InitSemaphore(&sem_GameState, 1);
    G8RTOS_InitSemaphore(&sem_I2CA, 1);
    G8RTOS_InitSemaphore(&sem_SPIA, 1);
    G8RTOS_InitSemaphore(&sem_PCA9555_Debounce, 0);
    G8RTOS_InitSemaphore(&sem_Joystick_Debounce, 0);

    // Initialize FIFOs
    G8RTOS_InitFIFO(BUTTONS_FIFO);
    G8RTOS_InitFIFO(JOYSTICK_FIFO);

    // Initialize game state
    InitializeBoard();
    gameState.gameOver = false;
    gameState.pauseGame = false;
    
    // Add background threads
    G8RTOS_AddThread(Tetris_Game_Thread, GAME_THREAD_PRIORITY, "Game Thread");
    G8RTOS_AddThread(Tetris_Joystick_Thread, JOYSTICK_THREAD_PRIORITY, "Joystick Thread");
    //G8RTOS_AddThread(Read_Buttons, READ_BUTTONS_PRIORITY, "Read Buttons");
    //G8RTOS_AddThread(Tetris_Button_Thread, BUTTON_THREAD_PRIORITY, "TETRIS_BUTTON");
    G8RTOS_AddThread(Idle_Thread, 255, "Idle Thread");

    // Add periodic threads
    G8RTOS_Add_PeriodicEvent(Tetris_Display_Thread, DISPLAY_PERIOD, 0);
    G8RTOS_Add_PeriodicEvent(Read_Joystick, JOYSTICK_PERIOD, 1);

    // Add aperiodic events
    //G8RTOS_Add_APeriodicEvent(Button_Handler, 1, BUTTON_INTERRUPT);
}

static void InitializeBoard(void) {
    int i = 0;
    int j = 0;
    
    G8RTOS_WaitSemaphore(&sem_GameState);
    for (i = 0; i < BOARD_HEIGHT; i++) {
        for (j = 0; j < BOARD_WIDTH; j++) {
            gameState.board[i][j] = CELL_EMPTY;
        }
    }
    G8RTOS_SignalSemaphore(&sem_GameState);
}

static void SpawnNewPiece(void) {
    gameState.currentPieceX = BOARD_WIDTH / 2 - PIECE_SIZE / 2;
    gameState.currentPieceY = 0;
    gameState.currentPieceType = (rand() % 7);  // Randomly select one of the 7 pieces
    
    // Check if piece can be placed at starting postition, if not, game over
    if (!CanMovePiece(gameState.currentPieceX, gameState.currentPieceY)) {
        G8RTOS_WaitSemaphore(&sem_GameState);
        gameState.gameOver = true;
        G8RTOS_SignalSemaphore(&sem_GameState);
        
        // Force immediate display update to show game over screen
        UpdateTetrisDisplay();
    }
}

static bool CanMovePiece(int newX, int newY) {
    int i = 0;
    int j = 0;
    
    // Check boundaries
    if (newX < 0 || newX + PIECE_SIZE > BOARD_WIDTH ||
        newY < 0 || newY + PIECE_SIZE > BOARD_HEIGHT) {
        return false;
    }
    
    // Check collision with existing pieces
    for (i = 0; i < PIECE_SIZE; i++) {
        for (j = 0; j < PIECE_SIZE; j++) {
            if (PIECES[gameState.currentPieceType][i][j]) {
                if (gameState.board[newY + i][newX + j] != CELL_EMPTY) {
                    return false;
                }
            }
        }
    }
    return true;
}

static void MovePiece(int deltaX, int deltaY) {
    int newX = gameState.currentPieceX + deltaX;
    int newY = gameState.currentPieceY + deltaY;
    
    if (CanMovePiece(newX, newY)) {
        gameState.currentPieceX = newX;
        gameState.currentPieceY = newY;
    }
    else if ((newY + 1) > 0) {
            if (CanMovePiece(newX, newY + 1)) {
                gameState.currentPieceX = newX;
                gameState.currentPieceY = newY + 1;
            }
        }
    if ((deltaY > 0) && (!(CanMovePiece(newX, newY)) && !(CanMovePiece(newX, newY + 1)))) {  // If we tried to move down but couldn't
        // Place the current piece on the board
        PlacePieceOnBoard();
        // Spawn a new piece
        SpawnNewPiece();
    }
}

static void UpdateTetrisDisplay(void) {
    // If game is over, just show red screen and return
    if (gameState.gameOver) {
        DrawGameOverScreen();
        return;
    }

    const int cellWidth = Y_MAX / BOARD_HEIGHT;
    const int cellHeight = Y_MAX / BOARD_HEIGHT;
    int x = 0;
    int y = 0;
    int i = 0;
    int j = 0;
    
    // Create a temporary board for display
    uint8_t displayBoard[BOARD_HEIGHT][BOARD_WIDTH];
    
    // Copy the current board state
    for (y = 0; y < BOARD_HEIGHT; y++) {
        for (x = 0; x < BOARD_WIDTH; x++) {
            displayBoard[y][x] = gameState.board[y][x];
        }
    }
    
    // Add the current moving piece to the display board
    for (i = 0; i < PIECE_SIZE; i++) {
        for (j = 0; j < PIECE_SIZE; j++) {
            if (PIECES[gameState.currentPieceType][i][j]) {
                int boardY = gameState.currentPieceY + i;
                int boardX = gameState.currentPieceX + j;
                if (boardY >= 0 && boardY < BOARD_HEIGHT && 
                    boardX >= 0 && boardX < BOARD_WIDTH) {
                    displayBoard[boardY][boardX] = gameState.currentPieceType + 1;
                }
            }
        }
    }
    
    // Array to map cell values to colors
    const uint16_t cellColors[] = {
        COLOR_EMPTY,  // 0
        COLOR_I,      // 1
        COLOR_O,      // 2
        COLOR_T,      // 3
        COLOR_S,      // 4
        COLOR_Z,      // 5
        COLOR_J,      // 6
        COLOR_L       // 7
    };
    
    // Draw the combined board (placed pieces + current moving piece)
    for (y = 0; y < BOARD_HEIGHT; y++) {
        for (x = 0; x < BOARD_WIDTH; x++) {
            uint16_t color = cellColors[displayBoard[y][x]];
            // Invert the y coordinate when drawing
            int displayY = (BOARD_HEIGHT - 1 - y) * cellHeight;
            ST7789_DrawRectangle((x * cellWidth) + 30, displayY,
                               cellWidth - 1, cellHeight - 1, color);
        }
    }
}

static void PlacePieceOnBoard(void) {
    int i = 0;
    int j = 0;
    
    for (i = 0; i < PIECE_SIZE; i++) {
        for (j = 0; j < PIECE_SIZE; j++) {
            if (PIECES[gameState.currentPieceType][i][j]) {
                gameState.board[gameState.currentPieceY + i][gameState.currentPieceX + j] = 
                    gameState.currentPieceType + 1;  // +1 because 0 is empty
            }
        }
    }
}

static void DrawGameOverScreen(void) {
    // Fill entire screen with red
    ST7789_DrawRectangle(0, 0, X_MAX, Y_MAX, ST7789_RED);
}

/********************************Public Functions***********************************/

/*************************************Threads***************************************/

void Idle_Thread(void) {
    while(1);
}

void Read_Buttons(void) {

    // Initialize / declare any variables here
    uint8_t buttons;

    while(1) {
        // wait for button semaphore
        G8RTOS_WaitSemaphore(&sem_PCA9555_Debounce);
        // debounce buttons
        sleep(15);
        // Get buttons
        buttons = ~MultimodButtons_Get();
        // clear button interrupt
        GPIOIntClear(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
        // update current_buttons value
        G8RTOS_WriteFIFO(BUTTONS_FIFO, buttons);
    }
}

void Tetris_Game_Thread(void) {
    SpawnNewPiece();
    
    while (1) {
        // Exit game loop if game is over
        if (gameState.gameOver) {
            sleep(1000);  // Sleep to prevent busy waiting
            continue;
        }
        
        if (!gameState.pauseGame) {
            G8RTOS_WaitSemaphore(&sem_GameState);
            
            // Move piece down
            MovePiece(0, 1);
            
            // Check for completed lines here
            
            G8RTOS_SignalSemaphore(&sem_GameState);
        }
        sleep(dropSpeed);  // Adjust this value to change falling speed
    }
}

// Update the Tetris_Button_Thread to handle rotations
/*void Tetris_Button_Thread(void) {
    uint8_t buttons;
    
    while (1) {
        buttons = G8RTOS_ReadFIFO(BUTTONS_FIFO);
        
        // Only process buttons if game is not over
        if (!gameState.gameOver) {
            G8RTOS_WaitSemaphore(&sem_GameState);
            
            // SW1: Rotate counterclockwise
            if (buttons & SW1) {
                RotatePiece(false);
            }
            // SW2: Rotate clockwise
            else if (buttons & SW2) {
                RotatePiece(true);
            }
            
            G8RTOS_SignalSemaphore(&sem_GameState);
        }
        
        sleep(100);  // Poll input every 100ms
    }
}*/

void Tetris_Joystick_Thread(void) {
    // define variables
    uint32_t joystick_xy_val = 0;
    uint16_t joystick_x_val = 0;
    uint16_t joystick_y_val = 0;
    uint16_t deadzone_lower = 2048 - 512;
    uint16_t deadzone_upper = 2048 + 512;

    while(1) 
    {
        // Exit input processing if game is over
        if (gameState.gameOver) {
            sleep(1000);  // Sleep to prevent busy waiting
            continue;
        }

        if (!gameState.pauseGame) {
            // read joystick values
            joystick_xy_val = G8RTOS_ReadFIFO(JOYSTICK_FIFO);
            joystick_y_val = (joystick_xy_val >> 16 & 0xFFFF);
            joystick_x_val = (joystick_xy_val >> 0 & 0xFFFF);
            // normalize the joystick values
            float normalized_x = 0.0f;
            float normalized_y = 0.0f;
            if (joystick_x_val < deadzone_lower || joystick_x_val > deadzone_upper) {
                normalized_x = (float)(joystick_x_val - 2048.0f) / 2048.0f;
            }
            if (joystick_y_val < deadzone_lower || joystick_y_val > deadzone_upper) {
                normalized_y = (float)(joystick_y_val - 2048.0f) / 2048.0f;
            }
            // update piece position based on normalized_x
            G8RTOS_WaitSemaphore(&sem_GameState);
            if (normalized_x > 0) {
                MovePiece(-1, 0);
            }
            else if (normalized_x < 0) {
                MovePiece(1, 0);
            }
            if (normalized_y < 0) {
                MovePiece(0, 2);
            }
            G8RTOS_SignalSemaphore(&sem_GameState);
        }
        sleep(50);  // Add a small delay to prevent too rapid movement
    }
}

/********************************Periodic Threads***********************************/

void Read_Joystick(void) {
    // read joystick values
    uint32_t joystick_xy_val = JOYSTICK_GetXY();
    // push joystick value to fifo
    G8RTOS_WriteFIFO(JOYSTICK_FIFO, joystick_xy_val);
}

void Tetris_Display_Thread(void) {
    G8RTOS_WaitSemaphore(&sem_GameState);
    UpdateTetrisDisplay();
    G8RTOS_SignalSemaphore(&sem_GameState);
}

/*******************************Aperiodic Threads***********************************/

void Button_Handler() {
    // disable interrupt and signal semaphore
    GPIOIntDisable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
    G8RTOS_SignalSemaphore(&sem_PCA9555_Debounce);
}
