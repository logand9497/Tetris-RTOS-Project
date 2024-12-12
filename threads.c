// G8RTOS_Threads.c
// Date Created: 2023-07-25
// Date Updated: 2024-12-11
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
static void RotatePiece(bool clockwise);
static void CheckAndClearLines(void);
static void DrawPauseScreen(void);
static void DrawStartScreen(void);

/*********************************Global Variables**********************************/

// Game state semaphore
semaphore_t sem_GameState;

// Game state
static GameState gameState;

// Drop speed variable
uint32_t dropSpeed = 250;

// Fast drop flag
static bool fastDrop = false;

// Seperate array to store currently dropping piece
static uint8_t currentPiece[PIECE_SIZE][PIECE_SIZE];

// Current score tracker
static uint32_t currentScore = 0;

// Game over screen drawn flag
static bool gameOverScreenDrawn = false;

// Pause screen drawn flag
static bool pauseScreenDrawn = false;

// Unpaused flag
static bool unpaused = false;

// Game started flag
static bool gameStarted = false;

// Start screen drawn flag
static bool startScreenDrawn = false;

/*********************************Global Variables**********************************/

/********************************Public Functions***********************************/

void Threads_Init(void) {
    //Ensure clear screen
    ST7789_Fill(ST7789_BLACK);

    // Initialize game state semaphore
    G8RTOS_InitSemaphore(&sem_GameState, 1);
    G8RTOS_InitSemaphore(&sem_UART, 1);
    G8RTOS_InitSemaphore(&sem_I2CA, 1);
    G8RTOS_InitSemaphore(&sem_SPIA, 1);
    G8RTOS_InitSemaphore(&sem_PCA9555_Debounce, 0);

    // Initialize FIFOs
    G8RTOS_InitFIFO(BUTTONS_FIFO);
    G8RTOS_InitFIFO(JOYSTICK_FIFO);

    // Initialize flags
    gameStarted = false;
    startScreenDrawn = false;
    gameState.gameOver = false;
    gameState.pauseGame = false;
    
    // Reset score
    currentScore = 0;
    
    // Add background threads
    G8RTOS_AddThread(Tetris_Game_Thread, GAME_THREAD_PRIORITY, "Game Thread");
    G8RTOS_AddThread(Tetris_Joystick_Thread, JOYSTICK_THREAD_PRIORITY, "Joystick Thread");
    G8RTOS_AddThread(Read_Buttons, READ_BUTTONS_PRIORITY, "Read Buttons");
    G8RTOS_AddThread(Tetris_Button_Thread, BUTTON_THREAD_PRIORITY, "TETRIS_BUTTON");
    G8RTOS_AddThread(Idle_Thread, 255, "Idle Thread");

    // Add periodic threads
    G8RTOS_Add_PeriodicEvent(Tetris_Display_Thread, DISPLAY_PERIOD, 0);
    G8RTOS_Add_PeriodicEvent(Read_Joystick, JOYSTICK_PERIOD, 1);

    // Add aperiodic events
    G8RTOS_Add_APeriodicEvent(Button_Handler, 1, BUTTON_INTERRUPT);
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

    IBit_State = StartCriticalSection();
    ST7789_Fill(ST7789_BLACK);
    ST7789_DrawRectangle(0, 0, 35, Y_MAX, ST7789_GREY);
    ST7789_DrawRectangle(X_MAX - 35, 0, 35, Y_MAX, ST7789_GREY);
    i = 0;
    for (i = 0; i < 16; i++) {
        ST7789_DrawRectangle(0, 17 + (i * 17), 35, 1, ST7789_WHITE);
    }
    for (i = 0; i < 16; i++) {
        ST7789_DrawRectangle(205, 17 + (i * 17), 35, 1, ST7789_WHITE);
    }
    ST7789_DrawRectangle(34, 0, 1, Y_MAX, ST7789_WHITE);
    ST7789_DrawRectangle(18, 0, 1, Y_MAX, ST7789_WHITE);
    ST7789_DrawRectangle(1, 0, 1, Y_MAX, ST7789_WHITE);
    ST7789_DrawRectangle(205, 0, 1, Y_MAX, ST7789_WHITE);
    ST7789_DrawRectangle(222, 0, 1, Y_MAX, ST7789_WHITE);
    ST7789_DrawRectangle(239, 0, 1, Y_MAX, ST7789_WHITE);
    EndCriticalSection(IBit_State);
    
    gameState.nextPieceType = 0xFF;
    gameOverScreenDrawn = false;
    pauseScreenDrawn = false;
    G8RTOS_SignalSemaphore(&sem_GameState);
}

static void SpawnNewPiece(void) {
    int i, j;
    gameState.currentPieceX = BOARD_WIDTH / 2 - PIECE_SIZE / 2;
    gameState.currentPieceY = 0;
    
    // Use the next piece as current piece if it exists, otherwise generate random
    if (gameState.nextPieceType != 0xFF) {  // 0xFF is initialization value
        gameState.currentPieceType = gameState.nextPieceType;
    } else {
        gameState.currentPieceType = ((rand() * 5) % 7);
    }
    
    // Generate next piece
    gameState.nextPieceType = ((rand() * 5) % 7);

    // Set all LEDs in 4x4 grid to show next piece preview
    G8RTOS_WaitSemaphore(&sem_I2CA);
    PCA9556b_SetLED(4, 0xFF, PIECES[gameState.nextPieceType][0][0] ? 0xFF : 0x00);
    PCA9556b_SetLED(8, 0xFF, PIECES[gameState.nextPieceType][0][1] ? 0xFF : 0x00);
    PCA9556b_SetLED(12, 0xFF, PIECES[gameState.nextPieceType][0][2] ? 0xFF : 0x00);
    PCA9556b_SetLED(16, 0xFF, PIECES[gameState.nextPieceType][0][3] ? 0xFF : 0x00);
    PCA9556b_SetLED(5, 0xFF, PIECES[gameState.nextPieceType][1][0] ? 0xFF : 0x00);
    PCA9556b_SetLED(9, 0xFF, PIECES[gameState.nextPieceType][1][1] ? 0xFF : 0x00);
    PCA9556b_SetLED(13, 0xFF, PIECES[gameState.nextPieceType][1][2] ? 0xFF : 0x00);
    PCA9556b_SetLED(17, 0xFF, PIECES[gameState.nextPieceType][1][3] ? 0xFF : 0x00);
    PCA9556b_SetLED(6, 0xFF, PIECES[gameState.nextPieceType][2][0] ? 0xFF : 0x00);
    PCA9556b_SetLED(10, 0xFF, PIECES[gameState.nextPieceType][2][1] ? 0xFF : 0x00);
    PCA9556b_SetLED(14, 0xFF, PIECES[gameState.nextPieceType][2][2] ? 0xFF : 0x00);
    PCA9556b_SetLED(18, 0xFF, PIECES[gameState.nextPieceType][2][3] ? 0xFF : 0x00);
    PCA9556b_SetLED(7, 0xFF, PIECES[gameState.nextPieceType][3][0] ? 0xFF : 0x00);
    PCA9556b_SetLED(11, 0xFF, PIECES[gameState.nextPieceType][3][1] ? 0xFF : 0x00);
    PCA9556b_SetLED(15, 0xFF, PIECES[gameState.nextPieceType][3][2] ? 0xFF : 0x00);
    PCA9556b_SetLED(19, 0xFF, PIECES[gameState.nextPieceType][3][3] ? 0xFF : 0x00);
    G8RTOS_SignalSemaphore(&sem_I2CA);
    // Copy piece data to currentPiece array
    for (i = 0; i < PIECE_SIZE; i++) {
        for (j = 0; j < PIECE_SIZE; j++) {
            currentPiece[i][j] = PIECES[gameState.currentPieceType][i][j];
        }
    }
    
    // Check if piece can be placed at starting position
    if (!CanMovePiece(gameState.currentPieceX, gameState.currentPieceY)) {
        G8RTOS_WaitSemaphore(&sem_GameState);
        gameState.gameOver = true;
        G8RTOS_SignalSemaphore(&sem_GameState);

        G8RTOS_WaitSemaphore(&sem_UART);
        UARTprintf("Final score: %d\n", currentScore);
        G8RTOS_SignalSemaphore(&sem_UART);
        
        // Force immediate display update to show game over screen
        UpdateTetrisDisplay();
    }
}

static bool CanMovePiece(int newX, int newY) {
    int i = 0;
    int j = 0;
    
    // Find leftmost, rightmost, and bottom filled positions
    int leftmost = PIECE_SIZE;
    int rightmost = -1;
    int bottommost = -1;
    
    // Find the actual dimensions of the current piece orientation
    for (i = 0; i < PIECE_SIZE; i++) {
        for (j = 0; j < PIECE_SIZE; j++) {
            if (currentPiece[i][j]) {
                if (j < leftmost) leftmost = j;
                if (j > rightmost) rightmost = j;
                if (i > bottommost) bottommost = i;
            }
        }
    }
    
    // Check boundaries using actual piece dimensions
    // Left boundary
    if (newX < -leftmost) return false;
    
    // Right boundary
    if (newX + rightmost >= BOARD_WIDTH) return false;
    
    // Bottom boundary
    if (newY + bottommost >= BOARD_HEIGHT) return false;
    
    // Top boundary
    if (newY < 0) return false;
    
    // Check collision with existing pieces
    for (i = 0; i < PIECE_SIZE; i++) {
        for (j = 0; j < PIECE_SIZE; j++) {
            if (currentPiece[i][j]) {
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
        CheckAndClearLines();
        // Spawn a new piece
        SpawnNewPiece();
    }
}

static void UpdateTetrisDisplay(void) {
    // Check if game hasn't started
    if (!gameStarted) {
        DrawStartScreen();
        return;
    }
    
    // Check game over first
    if (gameState.gameOver) {
        DrawGameOverScreen();
        return;
    }
    
    // Check pause state
    if (gameState.pauseGame) {
        DrawPauseScreen();
        return;
    }

    if (unpaused) {
        G8RTOS_WaitSemaphore(&sem_SPIA);
        ST7789_Fill(ST7789_BLACK);
        ST7789_DrawRectangle(0, 0, 35, Y_MAX, ST7789_GREY);
        ST7789_DrawRectangle(X_MAX - 35, 0, 35, Y_MAX, ST7789_GREY);
        int i = 0;
        for (i = 0; i < 16; i++) {
            ST7789_DrawRectangle(0, 17 + (i * 17), 35, 1, ST7789_WHITE);
        }
        for (i = 0; i < 16; i++) {
            ST7789_DrawRectangle(205, 17 + (i * 17), 35, 1, ST7789_WHITE);
        }
        ST7789_DrawRectangle(34, 0, 1, Y_MAX, ST7789_WHITE);
        ST7789_DrawRectangle(18, 0, 1, Y_MAX, ST7789_WHITE);
        ST7789_DrawRectangle(1, 0, 1, Y_MAX, ST7789_WHITE);
        ST7789_DrawRectangle(205, 0, 1, Y_MAX, ST7789_WHITE);
        ST7789_DrawRectangle(222, 0, 1, Y_MAX, ST7789_WHITE);
        ST7789_DrawRectangle(239, 0, 1, Y_MAX, ST7789_WHITE);
        G8RTOS_SignalSemaphore(&sem_SPIA);
        unpaused = false;
    }
    
    // Clear both flags if game is running
    gameOverScreenDrawn = false;
    pauseScreenDrawn = false;
    
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
            if (currentPiece[i][j]) {
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
            ST7789_DrawRectangle((x * cellWidth) + 35, displayY,
                               cellWidth - 1, cellHeight - 1, color);
        }
    }
}

static void PlacePieceOnBoard(void) {
    int i = 0;
    int j = 0;
    
    for (i = 0; i < PIECE_SIZE; i++) {
        for (j = 0; j < PIECE_SIZE; j++) {
            if (currentPiece[i][j]) {
                gameState.board[gameState.currentPieceY + i][gameState.currentPieceX + j] = 
                    gameState.currentPieceType + 1;  // +1 because 0 is empty
            }
        }
    }
}

static void DrawGameOverScreen(void) {
    if (!gameOverScreenDrawn) {  // Only draw if we haven't already
        PCA9956b_SetAllOff();
        ST7789_DrawRectangle(0, 0, X_MAX, Y_MAX, ST7789_RED);

        //Letter G
        int G_base_x = 30;
        int G_base_y = (Y_MAX/2) + 30;
        ST7789_DrawRectangle(G_base_x, G_base_y + 5, 5, 40, ST7789_WHITE);
        ST7789_DrawRectangle(G_base_x + 5, G_base_y + 45, 25, 5, ST7789_WHITE);
        ST7789_DrawRectangle(G_base_x + 5, G_base_y, 25, 5, ST7789_WHITE);
        ST7789_DrawRectangle(G_base_x + 20, G_base_y + 24, 10, 5, ST7789_WHITE);
        ST7789_DrawRectangle(G_base_x + 30, G_base_y + 5, 5, 19, ST7789_WHITE);
        ST7789_DrawRectangle(G_base_x + 30, G_base_y + 40, 5, 5, ST7789_WHITE);

        //Letter A
        int A_base_x = 75;
        int A_base_y = (Y_MAX/2) + 30;
        ST7789_DrawRectangle(A_base_x, A_base_y, 5, 45, ST7789_WHITE);
        ST7789_DrawRectangle(A_base_x + 30, A_base_y, 5, 45, ST7789_WHITE);
        ST7789_DrawRectangle(A_base_x + 5, A_base_y + 45, 25, 5, ST7789_WHITE);
        ST7789_DrawRectangle(A_base_x + 5, A_base_y + 24, 25, 5, ST7789_WHITE);

        //Letter M
        int M_base_x = 120;
        int M_base_y = (Y_MAX/2) + 30;
        ST7789_DrawRectangle(M_base_x, M_base_y, 5, 45, ST7789_WHITE);
        ST7789_DrawRectangle(M_base_x + 5, M_base_y + 45, 10, 5, ST7789_WHITE);
        ST7789_DrawRectangle(M_base_x + 15, M_base_y, 5, 45, ST7789_WHITE);
        ST7789_DrawRectangle(M_base_x + 20, M_base_y + 45, 10, 5, ST7789_WHITE);
        ST7789_DrawRectangle(M_base_x + 30, M_base_y, 5, 45, ST7789_WHITE);

        //Letter E
        int E_base_x = 165;
        int E_base_y = (Y_MAX/2) + 30;
        ST7789_DrawRectangle(E_base_x + 5, E_base_y, 30, 5, ST7789_WHITE);
        ST7789_DrawRectangle(E_base_x + 5, E_base_y + 25, 25, 5, ST7789_WHITE);
        ST7789_DrawRectangle(E_base_x + 5, E_base_y + 45, 30, 5, ST7789_WHITE);
        ST7789_DrawRectangle(E_base_x, E_base_y, 5, 50, ST7789_WHITE);

        //Letter O
        int O_base_x = 30;
        int O_base_y = (Y_MAX/2) - 80;
        ST7789_DrawRectangle(O_base_x, O_base_y + 5, 5, 40, ST7789_WHITE);
        ST7789_DrawRectangle(O_base_x + 30, O_base_y + 5, 5, 40, ST7789_WHITE);
        ST7789_DrawRectangle(O_base_x + 5, O_base_y, 25, 5, ST7789_WHITE);
        ST7789_DrawRectangle(O_base_x + 5, O_base_y + 45, 25, 5, ST7789_WHITE);

        //Letter V
        int V_base_x = 75;
        int V_base_y = (Y_MAX/2) - 80;
        ST7789_DrawRectangle(V_base_x, V_base_y + 20, 5, 30, ST7789_WHITE);
        ST7789_DrawRectangle(V_base_x + 5, V_base_y + 10, 8, 10, ST7789_WHITE);
        ST7789_DrawRectangle(V_base_x + 13, V_base_y, 9, 10, ST7789_WHITE);
        ST7789_DrawRectangle(V_base_x + 22, V_base_y + 10, 8, 10, ST7789_WHITE);
        ST7789_DrawRectangle(V_base_x + 30, V_base_y + 20, 5, 30, ST7789_WHITE);

        //Second Letter E
        int E2_base_x = 120;
        int E2_base_y = (Y_MAX/2) - 80;
        ST7789_DrawRectangle(E2_base_x + 5, E2_base_y, 30, 5, ST7789_WHITE);
        ST7789_DrawRectangle(E2_base_x + 5, E2_base_y + 25, 25, 5, ST7789_WHITE);
        ST7789_DrawRectangle(E2_base_x + 5, E2_base_y + 45, 30, 5, ST7789_WHITE);
        ST7789_DrawRectangle(E2_base_x, E2_base_y, 5, 50, ST7789_WHITE);

        //Letter R
        int R_base_x = 165;
        int R_base_y = (Y_MAX/2) - 80;
        ST7789_DrawRectangle(R_base_x, R_base_y, 5, 50, ST7789_WHITE);
        ST7789_DrawRectangle(R_base_x + 5, R_base_y + 45, 25, 5, ST7789_WHITE);
        ST7789_DrawRectangle(R_base_x + 5, R_base_y + 24, 25, 5, ST7789_WHITE);
        ST7789_DrawRectangle(R_base_x + 30, R_base_y, 5, 24, ST7789_WHITE);
        ST7789_DrawRectangle(R_base_x + 30, R_base_y + 29, 5, 16, ST7789_WHITE);

        gameOverScreenDrawn = true;  // Set flag to prevent redrawing

    }
}

static void RotatePiece(bool clockwise) {
    int i = 0;
    int j = 0;
    uint8_t rotatedPiece[PIECE_SIZE][PIECE_SIZE] = {0};
    uint8_t tempPiece[PIECE_SIZE][PIECE_SIZE];
    
    // Copy current piece to temp array
    for (i = 0; i < PIECE_SIZE; i++) {
        for (j = 0; j < PIECE_SIZE; j++) {
            tempPiece[i][j] = currentPiece[i][j];
        }
    }
    
    // Perform rotation
    for (i = 0; i < PIECE_SIZE; i++) {
        for (j = 0; j < PIECE_SIZE; j++) {
            if (clockwise) {
                rotatedPiece[j][PIECE_SIZE-1-i] = tempPiece[i][j];
            } else {
                rotatedPiece[PIECE_SIZE-1-j][i] = tempPiece[i][j];
            }
        }
    }
    
    // Temporarily store the rotated piece
    for (i = 0; i < PIECE_SIZE; i++) {
        for (j = 0; j < PIECE_SIZE; j++) {
            currentPiece[i][j] = rotatedPiece[i][j];
        }
    }
    
    // Check if rotated position is valid
    if (!CanMovePiece(gameState.currentPieceX, gameState.currentPieceY)) {
        // If rotation is not valid, restore the original piece
        for (i = 0; i < PIECE_SIZE; i++) {
            for (j = 0; j < PIECE_SIZE; j++) {
                currentPiece[i][j] = tempPiece[i][j];
            }
        }
    }
}

static void CheckAndClearLines(void) {
    int i, j, k;
    int linesCleared = 0;
    bool isLineFull;
    
    // Check each line from bottom to top
    for (i = BOARD_HEIGHT - 1; i >= 0; i--) {
        isLineFull = true;
        
        // Check if line is full
        for (j = 0; j < BOARD_WIDTH; j++) {
            if (gameState.board[i][j] == CELL_EMPTY) {
                isLineFull = false;
                break;
            }
        }
        
        // If line is full, clear it and move everything down
        if (isLineFull) {
            linesCleared++;
            
            // Move all lines above down
            for (k = i; k > 0; k--) {
                for (j = 0; j < BOARD_WIDTH; j++) {
                    gameState.board[k][j] = gameState.board[k-1][j];
                }
            }
            
            // Clear top line
            for (j = 0; j < BOARD_WIDTH; j++) {
                gameState.board[0][j] = CELL_EMPTY;
            }
            
            // Since we moved everything down, we need to check this line again
            i++;
        }
    }
    
    // Update score based on number of lines cleared
    switch (linesCleared) {
        case 1:
            currentScore += 40;
            G8RTOS_WaitSemaphore(&sem_UART);
            UARTprintf("Current score: %d\n", currentScore);
            G8RTOS_SignalSemaphore(&sem_UART);
            break;
        case 2:
            currentScore += 100;
            G8RTOS_WaitSemaphore(&sem_UART);
            UARTprintf("Current score: %d\n", currentScore);
            G8RTOS_SignalSemaphore(&sem_UART);
            break;
        case 3:
            currentScore += 300;
            G8RTOS_WaitSemaphore(&sem_UART);
            UARTprintf("Current score: %d\n", currentScore);
            G8RTOS_SignalSemaphore(&sem_UART);
            break;
        case 4:
            currentScore += 1200;
            G8RTOS_WaitSemaphore(&sem_UART);
            UARTprintf("Current score: %d\n", currentScore);
            G8RTOS_SignalSemaphore(&sem_UART);
            break;
        default:
            break;
    }
}

static void DrawPauseScreen(void) {
    if (!pauseScreenDrawn) {  // Only draw if we haven't already
        // Fill screen with blue instead of red for pause
        ST7789_DrawRectangle(0, 0, X_MAX, Y_MAX, ST7789_BLUE);

        // Draw pause symbol (two vertical bars)
        int pause_x = X_MAX/2 - 20;  // Center the pause symbol
        int pause_y = Y_MAX/2 - 25;
        ST7789_DrawRectangle(pause_x, pause_y, 10, 50, ST7789_WHITE);
        ST7789_DrawRectangle(pause_x + 30, pause_y, 10, 50, ST7789_WHITE);
        
        pauseScreenDrawn = true;
    }
}

static void DrawStartScreen(void) {
    if (!startScreenDrawn) {
        // Fill screen with green background
        ST7789_DrawRectangle(0, 0, X_MAX, Y_MAX, ST7789_GREEN);
        
        // Letter S
        int S_base_x = 55;
        int S_base_y = Y_MAX/2 + 60;
        ST7789_DrawRectangle(S_base_x, S_base_y, 30, 5, ST7789_WHITE);
        ST7789_DrawRectangle(S_base_x + 30, S_base_y + 5, 5, 20, ST7789_WHITE);
        ST7789_DrawRectangle(S_base_x + 5, S_base_y + 25, 25, 5, ST7789_WHITE);
        ST7789_DrawRectangle(S_base_x, S_base_y + 30, 5, 15, ST7789_WHITE);
        ST7789_DrawRectangle(S_base_x + 5, S_base_y + 45, 30, 5, ST7789_WHITE);

        // Letter W
        int W_base_x = S_base_x + 45;
        int W_base_y = Y_MAX/2 + 60;
        ST7789_DrawRectangle(W_base_x, W_base_y + 5, 5, 45, ST7789_WHITE);
        ST7789_DrawRectangle(W_base_x + 5, W_base_y, 10, 5, ST7789_WHITE);
        ST7789_DrawRectangle(W_base_x + 15, W_base_y + 5, 5, 45, ST7789_WHITE);
        ST7789_DrawRectangle(W_base_x + 20, W_base_y, 10, 5, ST7789_WHITE);
        ST7789_DrawRectangle(W_base_x + 30, W_base_y + 5, 5, 45, ST7789_WHITE);

        // Number 4
        int num_4_base_x = W_base_x + 45;
        int num_4_base_y = Y_MAX/2 + 60;
        ST7789_DrawRectangle(num_4_base_x + 25, num_4_base_y, 5, 50, ST7789_WHITE);
        ST7789_DrawRectangle(num_4_base_x, num_4_base_y + 20, 35, 5, ST7789_WHITE);
        ST7789_DrawRectangle(num_4_base_x, num_4_base_y + 25, 5, 25, ST7789_WHITE);

        // Letter T
        int T_base_x = 75;
        int T_base_y = Y_MAX/2 - 20;
        ST7789_DrawRectangle(T_base_x + 16, T_base_y, 5, 50, ST7789_WHITE);
        ST7789_DrawRectangle(T_base_x, T_base_y + 45, 35, 5, ST7789_WHITE);

        // Letter O
        int O_base_x = T_base_x + 45;
        int O_base_y = Y_MAX/2 - 20;
        ST7789_DrawRectangle(O_base_x, O_base_y + 5, 5, 40, ST7789_WHITE);
        ST7789_DrawRectangle(O_base_x + 5, O_base_y, 25, 5, ST7789_WHITE);
        ST7789_DrawRectangle(O_base_x + 5, O_base_y + 45, 25, 5, ST7789_WHITE);
        ST7789_DrawRectangle(O_base_x + 30, O_base_y + 5, 5, 40, ST7789_WHITE);

        // Letter S2
        int S2_base_x = 12;
        int S2_base_y = Y_MAX/2 - 100;
        ST7789_DrawRectangle(S2_base_x, S2_base_y, 30, 5, ST7789_WHITE);
        ST7789_DrawRectangle(S2_base_x + 30, S2_base_y + 5, 5, 20, ST7789_WHITE);
        ST7789_DrawRectangle(S2_base_x + 5, S2_base_y + 25, 25, 5, ST7789_WHITE);
        ST7789_DrawRectangle(S2_base_x, S2_base_y + 30, 5, 15, ST7789_WHITE);
        ST7789_DrawRectangle(S2_base_x + 5, S2_base_y + 45, 30, 5, ST7789_WHITE);

        // Letter T2
        int T2_base_x = S2_base_x + 45;
        int T2_base_y = Y_MAX/2 - 100;
        ST7789_DrawRectangle(T2_base_x + 16, T2_base_y, 5, 50, ST7789_WHITE);
        ST7789_DrawRectangle(T2_base_x, T2_base_y + 45, 35, 5, ST7789_WHITE);

        // Letter A
        int A_base_x = T2_base_x + 45;
        int A_base_y = Y_MAX/2 - 100;
        ST7789_DrawRectangle(A_base_x, A_base_y, 5, 45, ST7789_WHITE);
        ST7789_DrawRectangle(A_base_x + 30, A_base_y, 5, 45, ST7789_WHITE);
        ST7789_DrawRectangle(A_base_x + 5, A_base_y + 45, 25, 5, ST7789_WHITE);
        ST7789_DrawRectangle(A_base_x + 5, A_base_y + 24, 25, 5, ST7789_WHITE);

        // Letter R
        int R_base_x = A_base_x + 45;
        int R_base_y = Y_MAX/2 - 100;
        ST7789_DrawRectangle(R_base_x, R_base_y, 5, 50, ST7789_WHITE);
        ST7789_DrawRectangle(R_base_x + 5, R_base_y + 45, 25, 5, ST7789_WHITE);
        ST7789_DrawRectangle(R_base_x + 5, R_base_y + 24, 25, 5, ST7789_WHITE);
        ST7789_DrawRectangle(R_base_x + 30, R_base_y, 5, 24, ST7789_WHITE);
        ST7789_DrawRectangle(R_base_x + 30, R_base_y + 29, 5, 16, ST7789_WHITE);

        // Letter T3
        int T3_base_x = R_base_x + 45;
        int T3_base_y = Y_MAX/2 - 100;
        ST7789_DrawRectangle(T3_base_x + 16, T3_base_y, 5, 50, ST7789_WHITE);
        ST7789_DrawRectangle(T3_base_x, T3_base_y + 45, 35, 5, ST7789_WHITE);

        startScreenDrawn = true;
    }
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
    while (1) {
        // Don't start game logic until game has started
        if (!gameStarted) {
            sleep(100);
            continue;
        }
        
        // Exit game loop if game is over
        if (gameState.gameOver) {
            sleep(1000);
            continue;
        }
        
        if (!gameState.pauseGame) {
            G8RTOS_WaitSemaphore(&sem_GameState);
            
            // Move piece down
            MovePiece(0, 1);
            G8RTOS_SignalSemaphore(&sem_GameState);
        }
        
        // Sleep for different durations based on fastDrop state
        sleep(fastDrop ? dropSpeed / 4 : dropSpeed);
    }
}

// Uncomment and update the button thread
void Tetris_Button_Thread(void) {
    uint8_t buttons;
    
    while (1) {
        buttons = G8RTOS_ReadFIFO(BUTTONS_FIFO);
        
        // Check for game start
        if (!gameStarted) {
            if (buttons & SW4) {
                gameStarted = true;
                InitializeBoard();
                SpawnNewPiece();
            }
            GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
            sleep(100);
            continue;
        }
        
        // Only process rotation buttons if game is not over and not paused
        if (!gameState.gameOver) {
            G8RTOS_WaitSemaphore(&sem_GameState);
            
            if (!gameState.pauseGame) {
                // SW1: Rotate counterclockwise
                if (buttons & SW1) {
                    RotatePiece(false);
                }
                // SW2: Rotate clockwise
                if (buttons & SW2) {
                    RotatePiece(true);
                }
            }
            
            // SW3: Toggle pause
            if (buttons & SW3) {
                gameState.pauseGame = !gameState.pauseGame;
                // Clear pause screen flag when unpausing
                if (!gameState.pauseGame) {
                    pauseScreenDrawn = false;
                    unpaused = true;
                }
            }
            
            G8RTOS_SignalSemaphore(&sem_GameState);
        }
        GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
        sleep(100);
    }
}

void Tetris_Joystick_Thread(void) {
    // define variables
    uint32_t joystick_xy_val = 0;
    uint16_t joystick_x_val = 0;
    uint16_t joystick_y_val = 0;
    uint16_t deadzone_lower = 2048 - 512;
    uint16_t deadzone_upper = 2048 + 512;

    while(1) 
    {
        if (gameState.gameOver) {
            sleep(1000);  // Sleep to prevent busy waiting
            continue;
        }

        if (!gameState.pauseGame) {
            // read joystick values
            joystick_xy_val = G8RTOS_ReadFIFO(JOYSTICK_FIFO);
            joystick_x_val = (joystick_xy_val >> 16 & 0xFFFF);
            joystick_y_val = (joystick_xy_val >> 0 & 0xFFFF);
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
            
            // Update fast drop state based on joystick position
            fastDrop = (normalized_y < 0);
            
            G8RTOS_SignalSemaphore(&sem_GameState);
        }
        sleep(50);  // Add a small delay to prevent too rapid movement
    }
}

/********************************Periodic Threads***********************************/

void Read_Joystick(void) {
    if (!(gameState.pauseGame)) {
        // read joystick values
        uint32_t joystick_xy_val = JOYSTICK_GetXY();
        // push joystick value to fifo
        G8RTOS_WriteFIFO(JOYSTICK_FIFO, joystick_xy_val);
    }
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
