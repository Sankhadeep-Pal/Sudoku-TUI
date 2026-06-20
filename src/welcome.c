#include <stdio.h>
#include "../headers/input.h"
#include "../headers/audio.h"

/*
 * Modifies the menu item selector index based on directional input.
 * Implements circular wrapping when moving past bounds (0-2).
*/
void moveTracker(char c, int* tracker) {
    switch (c)
    {
        case 'A': // Up
            (*tracker)--;
            if (*tracker < 0) *tracker = 2;
            break;
        case 'B': // Down
            (*tracker)++;
            if (*tracker > 2) *tracker = 0;
            break;
    }
}

/*
 * Clears terminal layout using ANSI escapes, draws the large ASCII art header,
 * and iterates through option arrays to render a focused cursor indicator (">").
*/
void printScreen(char str[3][9], int tracker) {
    printf("\e[1;1H\e[2J");
    printf("\n\n\n");
    printf("    ███████╗██╗   ██╗██████╗  ██████╗ ██╗  ██╗██╗   ██╗\n    ██╔════╝██║   ██║██╔══██╗██╔═══██╗██║ ██╔╝██║   ██║\n    ███████╗██║   ██║██║  ██║██║   ██║█████╔╝ ██║   ██║\n    ╚════██║██║   ██║██║  ██║██║   ██║██╔═██╗ ██║   ██║\n    ███████║╚██████╔╝██████╔╝╚██████╔╝██║  ██╗╚██████╔╝\n    ╚══════╝ ╚═════╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═╝ ╚═════╝\n\n                ████████╗██╗   ██╗██╗\n                ╚══██╔══╝██║   ██║██║\n                   ██║   ██║   ██║██║\n                   ██║   ██║   ██║██║\n                   ██║   ╚██████╔╝██║\n                   ╚═╝    ╚═════╝ ╚═╝\n\n\n");
    for (int i = 0; i < 3; i++) {
        printf("                     ");
        if (i == tracker) printf("> ");
        else printf("  ");
        puts(str[i]);
        printf("\n\n");
    }

}

/*
 * Primary Welcome Menu Lifecycle Controller
 * Handles option navigation, sub-menus for difficulty clue values, and audio engine setup.
*/
int welcome(int* clue) {
    char options[3][9] = { "New Game\0", "Continue\0", "Exit\0" };
    char mode[3][9] = { "Easy\0", "Normal\0", "Hard\0" };
    int tracker = 0;
    int modeTracker = 0;
    rawMode(0);
    ma_engine* engine = createEngine();
    ma_sound* sound = createSound(engine, "./assets_data/welcome.mp3");
    audio(engine, sound, 0);
    while (1) {
        printScreen(options, tracker);
        char c = input();
        moveTracker(c, &tracker);
        if (c == 'E') { // 'E' represents the selection action (Enter key proxy)
            if (tracker == 0) { // New Game chosen: Redirect into difficulty sub-loop
                while (1) {
                    printScreen(mode, modeTracker);
                    char c = input();
                    moveTracker(c, &modeTracker);
                    if (c == 'E') {
                        // Assign the exact starting pre-filled numbers based on chosen difficulty
                        if (modeTracker == 0) // Easy
                            *clue = 40;
                        else if (modeTracker == 1) // Normal
                            *clue = 30;
                        else if (modeTracker == 2) // Hard
                            *clue = 20;
                        break;

                    }
                }
            }
            // Teardown steps before returning tracking integers back to the main loop context
            audio(engine, sound, 1);
            rawMode(1);
            return tracker;
        }
    }
}