#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../headers/audio.h"
#include "../headers/input.h"

/*
 * Global Game State Arrays
 * To optimize lookups, rows, columns, and 3x3 boxes are mapped directly.
 * tracker[i][j]: 1 = Permanent Clue, 0 = Empty, -1 = User Placed Digit.
*/
int game[9][9] = { 0 };
int rowMaintainer[9][9] = { 0 };
int columnMaintainer[9][9] = { 0 };
int boxMaintainer[9][9] = { 0 };
int tracker[9][9] = { 0 };
int cursorBoxIndex = 0;
int cursorNumIndex = 0;

/*
 * Draws the top ('u') or bottom ('l') boundary of a grid cell.
 * Highlights the cell border if it matches the current cursor position.
 */
void rowPad(char c, int boxIndex, int rowIndex) {
    printf("    ");
    for (int i = boxIndex; i < boxIndex + 3; i++) {
        for (int j = rowIndex; j < rowIndex + 3; j++) {
            if (i == cursorBoxIndex && j == cursorNumIndex && c == 'u') {
                printf("┌───┐");
            } else if (i == cursorBoxIndex && j == cursorNumIndex && c == 'l') {
                printf("└───┘");
            } else {
                printf("     ");
            }
            if (j != rowIndex + 2)
                printf("║");
        }
        if (i != boxIndex + 2)
            printf("██");
    }
    printf("\n");
}

/*
 * Renders the center segment of a row, printing either the digit
 * or an empty gap, surrounded by cursor brackets if focused.
*/
void rowPrint(int boxIndex, int rowIndex) {
    for (int i = boxIndex; i < boxIndex + 3; i++) {
        for (int j = rowIndex; j < rowIndex + 3; j++) {
            int num = game[i][j];
            if (num != 0) {
                if (i == cursorBoxIndex && j == cursorNumIndex)
                    printf("│ %d │", num);
                else
                    printf("  %d  ", num);
            } else {
                if (i == cursorBoxIndex && j == cursorNumIndex)
                    printf("│   │");
                else
                    printf("     ");
            }
            if (j != rowIndex + 2)
                printf("║");
        }
        if (i != boxIndex + 2)
            printf("██");
    }
    printf("\n");
}

/*
 * Clears the terminal using ANSI escape codes and re-renders
 * the entire Sudoku board from the macro block perspective.
*/
void printBoard() {
    printf("\e[1;1H\e[2J");
    printf("\n\n\n");
    printf("    Press w, a, s, d to move.\n    Press 1-9 to enter the number.\n    Press 0 to erase the number");
    printf("\n\n\n");
    for (int i = 0; i < 9; i++) {
        int boxIndex = i / 3 * 3;
        int rowIndex = (i % 3) * 3;
        rowPad('u', boxIndex, rowIndex);
        printf("    ");
        rowPrint(boxIndex, rowIndex);
        rowPad('l', boxIndex, rowIndex);
        if (i == 2 || i == 5)
            printf("    ███████████████████████████████████████████████████████\n");
        else if (i != 8)
            printf("    ═════╬═════╬═════██═════╬═════╬═════██═════╬═════╬═════\n");
    }
}
int randGen(int min, int max) {
    return ((rand() % (max - min + 1)) + min);
}

/*
 * Generates a completely randomized independent 3x3 block config.
 * Used to seed main diagonal grids safely without breaking constraint rules.
*/
void fillSquare(int squareIndex) {
    int square[9] = { 0,0,0,0,0,0,0,0,0 };
    for (int i = 1; i <= 9;) {
        int pos = randGen(0, 8);
        if (square[pos] == 0) {
            square[pos] = i;
            i++;
        } else {
            continue;
        }
    }
    for (int i = 0; i <= 8; i++) {
        game[squareIndex][i] = square[i];
    }
}

/*
 * Maps the game 9x9 block space back to horizontal/vertical rows/columns
 * to populate lookup tables for constrint checking.
*/
void fillMaintainers() {
    int squareIndex, numberIndex;
    for (int i = 0; i <= 8; i++) {
        for (int j = 0; j <= 8; j++) {
            squareIndex = ((i / 3) * 3) + (j / 3);
            numberIndex = ((i % 3) * 3) + (j % 3);
            if (game[squareIndex][numberIndex] != 0) {
                rowMaintainer[i][game[squareIndex][numberIndex] - 1] = 1;
                columnMaintainer[j][game[squareIndex][numberIndex] - 1] = 1;
                boxMaintainer[squareIndex][game[squareIndex][numberIndex] - 1] = 1;
                tracker[squareIndex][numberIndex] = 1;
            }
        }
    }
}

/*
 * Backtracking Solver
 * Recursively explores rows and columns to find valid grid completions.
*/
int filler(int sequence[9], int row, int col) {
    int response = 0;
    if (col == 9) {
        response = filler(sequence, row + 1, 0);
        return response;
    } else if (row == 9) {
        return 1; // Base case: Entire board filled successfully
    }
    int squareIndex = ((row / 3) * 3) + (col / 3);
    int numberIndex = ((row % 3) * 3) + (col % 3);
    if (game[squareIndex][numberIndex] != 0) {
        response = filler(sequence, row, col + 1);
        return response;
    } else {
        for (int i = 0; i <= 8; i++) {
            int number = sequence[i] - 1;
            if (rowMaintainer[row][number] != 1 && columnMaintainer[col][number] != 1 && boxMaintainer[squareIndex][number] != 1) {
                game[squareIndex][numberIndex] = number + 1;
                rowMaintainer[row][number] = 1;
                columnMaintainer[col][number] = 1;
                boxMaintainer[squareIndex][number] = 1;
                response = filler(sequence, row, col + 1);
                // Backtrack step if combination locks up down the line
                if (response == 0) {
                    rowMaintainer[row][number] = 0;
                    columnMaintainer[col][number] = 0;
                    boxMaintainer[squareIndex][number] = 0;
                    game[squareIndex][numberIndex] = 0;
                } else {
                    return 1;
                }
            }
        }
        return 0;
    }
}

/*
 * Seeds independent diagonal blocks, initializes lookups,
 * and calls the backtracking solver to output a fully solved board.
*/
void sudokuCreate() {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            tracker[i][j] = 1;
        }
    }
    srand(time(NULL));
    // Fills blocks (0, 4, 8) because diagonal blocks are independent of each other
    for (int i = 0; i <= 8; i += 4) {
        fillSquare(i);
    }
    fillMaintainers();
    int sequence[9] = { 0,0,0,0,0,0,0,0,0 };
    for (int i = 1; i <= 9;) {
        int pos = randGen(0, 8);
        if (sequence[pos] == 0) {
            sequence[pos] = i;
            i++;
        } else {
            continue;
        }
    }
    filler(sequence, 0, 0);
}

/*
 * Checks for a unique solution by calculating completion pathways.
 * Halts exploration immediately if counter grows beyond 1 (multiple solutions).
*/
void solver(int row, int col, int* counter) {
    if (*counter > 1) {
        return;
    }
    if (col == 9) {
        solver(row + 1, 0, counter);
        return;
    } else if (row == 9) {
        (*counter)++;
        return;
    }
    int squareIndex = ((row / 3) * 3) + (col / 3);
    int numberIndex = ((row % 3) * 3) + (col % 3);
    if (game[squareIndex][numberIndex] != 0) {
        solver(row, col + 1, counter);
        return;
    } else {
        for (int num = 0; num <= 8; num++) {
            if (rowMaintainer[row][num] != 1 && columnMaintainer[col][num] != 1 && boxMaintainer[squareIndex][num] != 1) {
                rowMaintainer[row][num] = 1;
                columnMaintainer[col][num] = 1;
                boxMaintainer[squareIndex][num] = 1;
                game[squareIndex][numberIndex] = num + 1;
                solver(row, col + 1, counter);

                // Reset state parameters for alternative paths evaluation
                rowMaintainer[row][num] = 0;
                columnMaintainer[col][num] = 0;
                boxMaintainer[squareIndex][num] = 0;
                game[squareIndex][numberIndex] = 0;
            }
        }
    }
}

/*
 * Picks out random cell in the completed puzzle array based on requested difficulty levels.
 * Validates uniqueness via solver() step before making a cell empty permanently.
*/
void boardReady(int* clue) {
    int boxIndex, numberIndex;
    int attempt = 0, maxAttempt = 1000;
    for (int i = 1; i <= (81 - (*clue));) {
        boxIndex = randGen(0, 8);
        numberIndex = randGen(0, 8);
        if (attempt > maxAttempt) {
            break;
        }
        int row = ((boxIndex / 3) * 3) + (numberIndex / 3);
        int col = ((boxIndex % 3) * 3) + (numberIndex % 3);
        if (game[boxIndex][numberIndex] != 0 && tracker[boxIndex][numberIndex] == 1) {
            int num = game[boxIndex][numberIndex] - 1;
            game[boxIndex][numberIndex] = 0;
            rowMaintainer[row][num] = 0;
            columnMaintainer[col][num] = 0;
            boxMaintainer[boxIndex][num] = 0;
            int counter = 0;
            solver(0, 0, &counter);
            if (counter == 1) { // Safe removal: Puzzle still yields exactly one true solution
                tracker[boxIndex][numberIndex] = 0;
                i++;
            } else { // Undo removal: Multiple solution detected
                game[boxIndex][numberIndex] = num + 1;
                rowMaintainer[row][num] = 1;
                columnMaintainer[col][num] = 1;
                boxMaintainer[boxIndex][num] = 1;
                tracker[boxIndex][numberIndex] = -1;
                attempt++;
            }
        } else {
            attempt++;
        }
    }
}

/*
 * Manages cursor coordinates and update according to the keystroke.
 * Intercepts numeric inputs ('0'-'9') and translates key values into internal arrays.
*/
void cursorMove(char c) {
    int boxIndex = cursorBoxIndex;
    int numIndex = cursorNumIndex;
    switch (c)
    {
        case 'A': // Up
            numIndex -= 3;
            if (numIndex < 0) {
                boxIndex -= 3;
                numIndex = cursorNumIndex + 6;
            }
            if (boxIndex < 0) {
                boxIndex = cursorBoxIndex;
                numIndex = cursorNumIndex;
            }
            break;
        case 'B': // Down
            numIndex += 3;
            if (numIndex > 8) {
                boxIndex += 3;
                numIndex = cursorNumIndex - 6;
            }
            if (boxIndex > 8) {
                boxIndex = cursorBoxIndex;
                numIndex = cursorNumIndex;
            }
            break;
        case 'C': // Right
            numIndex++;
            if (numIndex % 3 == 0) {
                boxIndex++;
                numIndex -= 3;
                if (boxIndex % 3 == 0) {
                    boxIndex = cursorBoxIndex;
                    numIndex = cursorNumIndex;
                }
            }
            break;
        case 'D': // Left
            if (numIndex % 3 == 0) {
                if (boxIndex % 3 == 0) {
                    boxIndex = cursorBoxIndex;
                    numIndex = cursorNumIndex;
                } else {
                    boxIndex--;
                    numIndex += 2;
                }
            } else numIndex--;
            break;
        case '0': // Erase
            if (tracker[boxIndex][numIndex] == -1) { // Restrict modifications to user filled cells only
                int row = ((boxIndex / 3) * 3) + (numIndex / 3);
                int col = ((boxIndex % 3) * 3) + (numIndex % 3);
                int preNum = game[boxIndex][numIndex] - 1;
                game[boxIndex][numIndex] = 0;
                tracker[boxIndex][numIndex] = 0;
                rowMaintainer[row][preNum] = 0;
                columnMaintainer[col][preNum] = 0;
                boxMaintainer[boxIndex][preNum] = 0;
            }
            break;
        default:
            if (c >= '1' && c <= '9') { // 1-9 keys
                int num = c - '1';
                if (tracker[boxIndex][numIndex] == 0 || tracker[boxIndex][numIndex] == -1) {
                    int row = ((boxIndex / 3) * 3) + (numIndex / 3);
                    int col = ((boxIndex % 3) * 3) + (numIndex % 3);
                    if (game[boxIndex][numIndex] != 0) { // Clear previous coordinate tracks
                        int preNum = game[boxIndex][numIndex] - 1;
                        rowMaintainer[row][preNum] = 0;
                        columnMaintainer[col][preNum] = 0;
                        boxMaintainer[boxIndex][preNum] = 0;
                    }
                    rowMaintainer[row][num] = 1;
                    columnMaintainer[col][num] = 1;
                    boxMaintainer[boxIndex][num] = 1;
                    game[boxIndex][numIndex] = num + 1;
                    tracker[boxIndex][numIndex] = -1;
                }
            }
            break;
    }
    cursorBoxIndex = boxIndex;
    cursorNumIndex = numIndex;
}

/*
 * Evaluates completion. Returns 0 if empty cells remain,
 * -1 if logic errors are found, or 1 for a completely valid grid layout.
*/
int checkBoard() {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (tracker[i][j] == 0) {
                return 0;
            }
        }
    }
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (rowMaintainer[i][j] == 0) return -1;
            if (columnMaintainer[i][j] == 0) return -1;
            if (boxMaintainer[i][j] == 0) return -1;
        }
    }
    return 1;
}
void resetSudokuState() {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            game[i][j] = 0;
            rowMaintainer[i][j] = 0;
            columnMaintainer[i][j] = 0;
            boxMaintainer[i][j] = 0;
            tracker[i][j] = 0;
        }
    }
    cursorBoxIndex = 0;
    cursorNumIndex = 0;
}

/*
 * Core Runtime Controller loop
 * Manages audio instances via miniaudio handlers, inputs, and persistence saves.
*/
int sudoku(int sudoku[9][9], int* clue) {
    resetSudokuState();
    int isZero = 1;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (game[i][j] != sudoku[i][j]) isZero = 0;
            game[i][j] = sudoku[i][j];
        }
    }
    rawMode(0);
    ma_engine* engine = createEngine();
    ma_sound* sound = createSound(engine, "./assets_data/back.mp3");
    audio(engine, sound, 0);
    if (isZero == 1) {
        sudokuCreate();
        boardReady(clue);
        printBoard();
    } else {
        fillMaintainers();
        printBoard();
    }
    while (1) {
        char c = input();
        if (c == 'Q') {
            printf("\n\nDo you want to save the game? (Y)es/ (N)o\n");
            fflush(stdout);
            c = input();

            if (c == 'Y') {
                FILE* fp = fopen("./assets_data/sudoku.bin", "wb");
                if (fp == NULL) {
                    break;
                }
                size_t written = fwrite(game, 9 * 9 * sizeof(int), 1, fp);
                fflush(fp);

                fclose(fp);
                break;
            } else if (c == 'N') break;
        }
        cursorMove(c);
        printBoard();
        int i = checkBoard();
        switch (i) {
            case 0:
                break;
            case -1:
            case 1:
                audio(engine, sound, 1); // Kill background thread execution
                rawMode(1); // Restore standard canonical shell settings
                return i;
        }
    }
    audio(engine, sound, 1);
    rawMode(1);
    return 0;
}