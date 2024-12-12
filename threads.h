// threads.h
// Date Created: 2023-07-26
// Date Updated: 2024-12-11
// Threads

#ifndef THREADS_H_
#define THREADS_H_

/************************************Includes***************************************/

#include <stdint.h>
#include <stdbool.h>
#include "./G8RTOS/G8RTOS.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/
// Game Constants
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 16
#define PIECE_SIZE 4

// Game board cell states
#define CELL_EMPTY 0
#define CELL_I 1
#define CELL_O 2
#define CELL_T 3
#define CELL_S 4
#define CELL_Z 5
#define CELL_J 6
#define CELL_L 7

// Piece colors (16-bit RGB565 format)
#define COLOR_EMPTY 0x0000   // Black
#define COLOR_I 0xFFE0      // Cyan
#define COLOR_O 0x07FF      // Yellow
#define COLOR_T 0xF81F      // Magenta
#define COLOR_S 0x07E0      // Green
#define COLOR_Z 0x001F      // Red
#define COLOR_J 0xF800      // Blue
#define COLOR_L 0x04BF      // Orange

// Thread priorities
#define GAME_THREAD_PRIORITY 2
#define BUTTON_THREAD_PRIORITY 4
#define READ_BUTTONS_PRIORITY 3
#define JOYSTICK_THREAD_PRIORITY 1

// Periodic events
#define JOYSTICK_PERIOD 50
#define DISPLAY_PERIOD 50

// FIFOs
#define BUTTONS_FIFO        0
#define JOYSTICK_FIFO       1

/*************************************Defines***************************************/

/***********************************Semaphores**************************************/

semaphore_t sem_I2CA;
semaphore_t sem_UART;
semaphore_t sem_SPIA;
semaphore_t sem_PCA9555_Debounce;
semaphore_t sem_GameState;

/***********************************Semaphores**************************************/

/***********************************Structures**************************************/

// Game state structure
typedef struct {
    uint8_t board[BOARD_HEIGHT][BOARD_WIDTH];
    int currentPieceX;
    int currentPieceY;
    int currentPieceType;  // 0-6 corresponding to piece types
    int nextPieceType;
    bool gameOver;
    bool pauseGame;
} GameState;

// All Tetris pieces definitions
static const uint8_t PIECES[7][PIECE_SIZE][PIECE_SIZE] = {
    // I-piece
    {
        {0, 0, 0, 0},
        {1, 1, 1, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    // O-piece
    {
        {0, 0, 0, 0},
        {0, 2, 2, 0},
        {0, 2, 2, 0},
        {0, 0, 0, 0}
    },
    // T-piece
    {
        {0, 0, 0, 0},
        {0, 3, 0, 0},
        {3, 3, 3, 0},
        {0, 0, 0, 0}
    },
    // S-piece
    {
        {0, 0, 0, 0},
        {0, 4, 4, 0},
        {4, 4, 0, 0},
        {0, 0, 0, 0}
    },
    // Z-piece
    {
        {0, 0, 0, 0},
        {5, 5, 0, 0},
        {0, 5, 5, 0},
        {0, 0, 0, 0}
    },
    // J-piece
    {
        {0, 0, 0, 0},
        {6, 0, 0, 0},
        {6, 6, 6, 0},
        {0, 0, 0, 0}
    },
    // L-piece
    {
        {0, 0, 0, 0},
        {0, 0, 7, 0},
        {7, 7, 7, 0},
        {0, 0, 0, 0}
    }
};

/***********************************Structures**************************************/

/*******************************Background Threads**********************************/

// Function declarations
void Idle_Thread(void);
void Read_Buttons(void);
void Threads_Init(void);
void Tetris_Game_Thread(void);
void Tetris_Button_Thread(void);
void Tetris_Joystick_Thread(void);

/*******************************Background Threads**********************************/

/********************************Periodic Threads***********************************/

void Read_Joystick(void);
void Tetris_Display_Thread(void);

/********************************Periodic Threads***********************************/

/*******************************Aperiodic Threads***********************************/

void Button_Handler(void);

/*******************************Aperiodic Threads***********************************/


#endif /* THREADS_H_ */

