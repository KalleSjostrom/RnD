#include "common.h"

#include "include/SDL.h"

#include "engine/modules/logging.h"

#include "error.h"
#include "audio.h"

static SDL_AudioDeviceID device_id = 1;
static SDL_AudioSpec wav_spec = {};

void audio_init() {
	wav_spec.freq = 44100;
	wav_spec.format = 32784;
	wav_spec.channels = 2;
	wav_spec.silence = 0;
	wav_spec.samples = 4096;
	wav_spec.size = 0;

	if (SDL_OpenAudio(&wav_spec, &wav_spec) < 0) { // TODO(kalle): SDL_CloseAudio();
		log_error("Audio", "Couldn't open audio: %s\n", SDL_GetError());
	}

	log_info("Audio", "audio spec.freq: %u\n", wav_spec.freq);
	log_info("Audio", "audio spec.format: %u\n", wav_spec.format);
	log_info("Audio", "audio spec.format BITSIZE: %u\n", (int) SDL_AUDIO_BITSIZE(wav_spec.format));
	log_info("Audio", "audio spec.format ISFLOAT: %u\n", (int) SDL_AUDIO_ISFLOAT(wav_spec.format));
	log_info("Audio", "audio spec.format ISBIGENDIAN: %u\n", (int) SDL_AUDIO_ISBIGENDIAN(wav_spec.format));
	log_info("Audio", "audio spec.format ISSIGNED: %u\n", (int) SDL_AUDIO_ISSIGNED(wav_spec.format));
	log_info("Audio", "audio spec.format ISINT: %u\n", (int) SDL_AUDIO_ISINT(wav_spec.format));
	log_info("Audio", "audio spec.format ISLITTLEENDIAN: %u\n", (int) SDL_AUDIO_ISLITTLEENDIAN(wav_spec.format));
	log_info("Audio", "audio spec.format ISUNSIGNED: %u\n", (int) SDL_AUDIO_ISUNSIGNED(wav_spec.format));
	log_info("Audio", "audio spec.channels: %u\n", wav_spec.channels);
	log_info("Audio", "audio spec.silence: %u\n", wav_spec.silence);
	log_info("Audio", "audio spec.samples: %u\n", wav_spec.samples);
	log_info("Audio", "audio spec.size: %u\n", wav_spec.size);
}

void audio_load(const char *filename, u8 **buffer, u32 *length) {
	SDL_AudioSpec loaded_wav_spec = {};
	if (SDL_LoadWAV_RW(SDL_RWFromFile(filename, "rb"), 1, &loaded_wav_spec, (Uint8**)buffer, length) == 0) {
		log_error("Audio", "Couldn't load audio! (filename=%s, error=%s)\n", filename, SDL_GetError());
	}

#if 0
	ASSERT(wav_spec.freq == loaded_wav_spec.freq, "Mismatch audio! freq (%u %u)", wav_spec.freq, loaded_wav_spec.freq);
	ASSERT(wav_spec.format == loaded_wav_spec.format, "Mismatch audio! format");
	ASSERT(SDL_AUDIO_BITSIZE(wav_spec.format) == SDL_AUDIO_BITSIZE(loaded_wav_spec.format), "Mismatch audio! SDL_AUDIO_BITSIZE");
	ASSERT(SDL_AUDIO_ISFLOAT(wav_spec.format) == SDL_AUDIO_ISFLOAT(loaded_wav_spec.format), "Mismatch audio! SDL_AUDIO_ISFLOAT");
	ASSERT(SDL_AUDIO_ISBIGENDIAN(wav_spec.format) == SDL_AUDIO_ISBIGENDIAN(loaded_wav_spec.format), "Mismatch audio! SDL_AUDIO_ISBIGENDIAN");
	ASSERT(SDL_AUDIO_ISSIGNED(wav_spec.format) == SDL_AUDIO_ISSIGNED(loaded_wav_spec.format), "Mismatch audio! SDL_AUDIO_ISSIGNED");
	ASSERT(SDL_AUDIO_ISINT(wav_spec.format) == SDL_AUDIO_ISINT(loaded_wav_spec.format), "Mismatch audio! SDL_AUDIO_ISINT");
	ASSERT(SDL_AUDIO_ISLITTLEENDIAN(wav_spec.format) == SDL_AUDIO_ISLITTLEENDIAN(loaded_wav_spec.format), "Mismatch audio! SDL_AUDIO_ISLITTLEENDIAN");
	ASSERT(SDL_AUDIO_ISUNSIGNED(wav_spec.format) == SDL_AUDIO_ISUNSIGNED(loaded_wav_spec.format), "Mismatch audio! SDL_AUDIO_ISUNSIGNED");
#endif
	ASSERT(wav_spec.channels == loaded_wav_spec.channels, "Mismatch audio! channels");
	ASSERT(wav_spec.silence == loaded_wav_spec.silence, "Mismatch audio! silence");
	ASSERT(wav_spec.samples == loaded_wav_spec.samples, "Mismatch audio! samples");
}

void audio_queue(u8 *buffer, u32 length) {
	if (SDL_QueueAudio(device_id, buffer, length) < 0) {
		log_error("Audio", "Couldn't queue audio! (error=%s)\n", SDL_GetError());
	}
}

u32 audio_queued_size() {
	return SDL_GetQueuedAudioSize(device_id);
}

void audio_free(int16_t *buffer) {
	SDL_FreeWAV((Uint8*)buffer);
}

void audio_set_playing(bool playing) {
	SDL_PauseAudioDevice(device_id, !playing);
}
