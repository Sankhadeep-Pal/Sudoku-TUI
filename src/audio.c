#define MINIAUDIO_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include "../headers/miniaudio.h"

/*
 * Dynamic Audio Engine Initializer
 * Allocates heap memory for the miniaudio engine state structure and boots up
 * the underlying OS audio subsystem (ALSA on Linux / WASAPI on Windows).
*/
ma_engine* createEngine() {
    ma_result result;
    ma_engine* engine = malloc(sizeof(ma_engine));
    if (engine == NULL) {
        return NULL;
    }

    // Passing NULL as the first argument uses the operating system's default playback device
    result = ma_engine_init(NULL, engine);
    if (result != MA_SUCCESS) {
        free(engine);
        return NULL;
    } else {
        return engine;
    }
}

/*
 * Dynamic Audio Source Binder
 * Loads a target media path file directly and establishes a tracked audio buffer object.
 * Configures the loaded sound to continuously loop over its timeline.
*/
ma_sound* createSound(ma_engine* engine, const char s[]) {
    ma_result result;
    ma_sound* sound = malloc(sizeof(ma_sound));
    if (sound == NULL) {
        return NULL;
    }
    result = ma_sound_init_from_file(engine, s, 0, NULL, NULL, sound);
    if (result != MA_SUCCESS) {
        free(sound);
        return NULL;
    } else {
        ma_sound_set_looping(sound, MA_TRUE);
        return sound;
    }
}

void audio(ma_engine* engine, ma_sound* sound, int i) {
    if (engine == NULL || sound == NULL) return;
    if (i == 0) {
        ma_sound_start(sound);
    } else {
        // Disconnect mixer nodes and stop active sound card pipelines safely
        ma_sound_uninit(sound);
        ma_engine_uninit(engine);
        free(engine);
        free(sound);
    }
}