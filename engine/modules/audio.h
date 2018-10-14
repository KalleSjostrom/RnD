#pragma once

void audio_init();
void audio_load(const char *filename, u8 **buffer, u32 *length);
void audio_queue(u8 *buffer, u32 length);
u32 audio_queued_size();
void audio_free(i16 *buffer);
void audio_set_playing(bool playing);
