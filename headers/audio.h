#ifndef AUDIO_H
#define AUDIO_H
#include "miniaudio.h"
ma_engine* createEngine();
ma_sound* createSound(ma_engine* engine, const char s[]);
void audio(ma_engine* engine, ma_sound* sound, int i);
#endif