#!/bin/bash
cd "$(dirname "$0")"
# 4. Hand over control entirely to your application
# gnome-terminal --maximize --zoom=0.8 -- ./sudoku
xterm -geometry 10000x10000 -fa "Monospace" -fs 10 -e "gcc ./src/main.c ./src/welcome.c ./src/sudoku.c ./src/input.c ./src/audio.c -lpthread -lm -o sudoku && ./sudoku"