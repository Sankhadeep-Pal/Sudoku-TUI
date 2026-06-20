#include <stdio.h>

/*
 * Cross-Platform Compilation Flags
 * Standard console operations line-buffer input by default.
 * To read hardware keys instantly, we pull platform-specific kernel APIs.
*/
#if defined(_WIN32) || defined(_WIN64)
#include <conio.h>  // Windows-specific console I/O
#define PLATFORM_WINDOWS 1
#else
#include <unistd.h>
#include <termios.h>
#define PLATFORM_WINDOWS 0
#endif

#if !PLATFORM_WINDOWS
// Preserves initial terminal attributes so we can restore them on exit
struct termios orig_termios;

// Restores canonical (line-buffered) processing modes to the standard stream.
void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

/*
 * Disables input echoing (ECHO) and canonical mode line-buffering (ICANON).
 * This forces the Linux kernel to pass keystrokes to our game instantaneously.
*/
void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;

    // Clear the ICANON and ECHO bitwise flags
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
#endif

// This function abstracts away the OS differences
char getNextKey() {
#if PLATFORM_WINDOWS
    // _getch() reads a keypress instantly without echoing or waiting for Enter in Windows
    return _getch();
#else
    char ch;
    if (read(STDIN_FILENO, &ch, 1) > 0) {
        return ch;
    }
    return 0;
#endif
}

/*
 * Exposes a clean interface to toggle raw modes regardless of target OS platform.
 * i == 0 -> Active raw reading state
 * i == 1 -> Standard normal fallback terminal state
*/
void rawMode(int i) {
#if !PLATFORM_WINDOWS
    if (i == 0)
        enableRawMode();
    else
        disableRawMode();
#endif
}

char input() {
    int isEscape = 0;
    while (1) {
        char ch = getNextKey();

#if PLATFORM_WINDOWS
        /*
         * Windows Extended Keys Handling
         * Function and arrow keys throw a 2-byte sequence where the first byte
         * is either 0 or 0xE0, followed by a hardware scan code byte.
        */
        if (ch == 0 || (unsigned char)ch == 0xE0) {
            char nextCh = getNextKey(); // Get the second byte
            if (nextCh == 72) return 'A'; // Up
            if (nextCh == 80) return 'B'; // Down
            if (nextCh == 77) return 'C'; // Right
            if (nextCh == 75) return 'D'; // Left
            continue;
        }
#else
        /*
         * Linux ANSI Escape Code Handling
         * Linux terminal arrow keys send a 3-character ANSI escape sequence:
         * ESC ('\x1b') -> '[' -> Virtual Direction Identifier ('A' through 'D')
        */
        if (ch == 0 || ch == '\x1b') continue;
        else if (ch == '[') {
            isEscape = 1;
            continue;
        }
        if (isEscape == 1) {
            isEscape = 0;
            if (ch == 'A' || ch == 'B' || ch == 'C' || ch == 'D') return ch;
            else continue;
        }
#endif
        if (isEscape == 0) {
            // Standard letters and WASD layouts into matching navigation tokens
            if (ch >= '0' && ch <= '9') return ch;
            else if (ch == 'w' || ch == 'W') return 'A'; // Up
            else if (ch == 'a' || ch == 'A') return 'D'; // Left
            else if (ch == 's' || ch == 'S') return 'B'; // Down
            else if (ch == 'd' || ch == 'D') return 'C'; // Right
            else if (ch == 'q' || ch == 'Q') return 'Q';
            else if (ch == 'y' || ch == 'Y') return 'Y';
            else if (ch == 'n' || ch == 'N') return 'N';
            else if (ch == '\n' || ch == '\r') return 'E'; // Enter Key
        }
    }
}