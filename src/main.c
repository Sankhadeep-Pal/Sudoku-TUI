#include <stdio.h>
#include <stdlib.h>

/*
 * Operating System Specific Conditional Compilation
 * Windows terminals do not enable ANSI escape sequences by default.
 * We must manually invoke the kernel via windows.h to unlock Virtual Terminal processing.
*/
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
void enableWindowsAnsi() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &dwMode)) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }
}
#endif
#include "../headers/welcome.h"
#include "../headers/sudoku.h"
#include "../headers/input.h"

/*
 * Main Application Lifecycle Engine
 * Acts as a state machine controlling state initialization, block binary I/O,
 * win/loss evaluation headers, and game restoration streams.
*/
int main() {
#if defined(_WIN32) || defined(_WIN64)
    enableWindowsAnsi();
#endif
    while (1) {
        int response;
        int clue = 0;
        response = welcome(&clue);
        int gameClone[9][9] = { 0 };
        if (response == 2) break;
        else if (response == 0) {
            FILE* fptr;
            fptr = fopen("./assets_data/sudoku.bin", "wb");
            if (fptr == NULL) exit(0);
            fwrite(gameClone, 9 * 9 * sizeof(int), 1, fptr);
            fclose(fptr);
            response = sudoku(gameClone, &clue);
        } else if (response == 1) {
            FILE* fp;
            fp = fopen("./assets_data/sudoku.bin", "rb");
            if (fp == NULL) {
                fp = fopen("./assets_data/sudoku.bin", "wb");
                if (fp == NULL) exit(0);
                fwrite(gameClone, 9 * 9 * sizeof(int), 1, fp);
            }
            fread(gameClone, 9 * 9 * sizeof(int), 1, fp);
            response = sudoku(gameClone, &clue);
            fclose(fp);
        }

        /*
         * Game Feedback Evaluation Phase
         * response == -1 : Validation Errors Found -> Show "YOU LOSE" ASCII array banner
         * response ==  1 : Validation Complete     -> Show "YOU WIN" ASCII array banner
         * response ==  0 : End the game
        */
        if (response == -1) {
            printf("\e[1;1H\e[2J");
            printf("    ▗▖  ▗▖▗▄▖ ▗▖ ▗▖    ▗▖    ▗▄▖  ▗▄▄▖▗▄▄▄▖ ▗▖\n     ▝▚▞▘▐▌ ▐▌▐▌ ▐▌    ▐▌   ▐▌ ▐▌▐▌   ▐▌    ▐▌\n      ▐▌ ▐▌ ▐▌▐▌ ▐▌    ▐▌   ▐▌ ▐▌ ▝▀▚▖▐▛▀▀▘ ▐▌\n      ▐▌ ▝▚▄▞▘▝▚▄▞▘    ▐▙▄▄▖▝▚▄▞▘▗▄▄▞▘▐▙▄▄▖ ▗▖\n");
        } else if (response == 1) {
            printf("\e[1;1H\e[2J");
            printf("    ▗▖  ▗▖▗▄▖ ▗▖ ▗▖    ▗▖ ▗▖▗▄▄▄▖▗▖  ▗▖ ▗▖\n     ▝▚▞▘▐▌ ▐▌▐▌ ▐▌    ▐▌ ▐▌  █  ▐▛▚▖▐▌ ▐▌\n      ▐▌ ▐▌ ▐▌▐▌ ▐▌    ▐▌ ▐▌  █  ▐▌ ▝▜▌ ▐▌\n      ▐▌ ▝▚▄▞▘▝▚▄▞▘    ▐▙█▟▌▗▄█▄▖▐▌  ▐▌ ▗▖\n");
        } else if (response == 0) break;
        rawMode(0);
        printf("\n\nDo you want to play again? (Y)es/(N)o\n");
        fflush(stdout);
        int c = input();
        rawMode(1);
        if (c == 'Y') continue;
        else if (c == 'N') {
            rawMode(1);
            break;
        }
    }
    return 0;
}