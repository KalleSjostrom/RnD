struct Sound {
	unsigned at;
	unsigned length;
	uint8_t *buffer;
	float volume[2];
	bool playing;
	int __padding;
};

struct Fade {
	float volume_start[2];
	float volume_stop[2];
	float duration;
	unsigned start;
};

struct ActiveSound {
	int sound_id;
	int end_count;

	float volume_target[2];
};

// #define SHRT_MIN -32767
// #define SHRT_MAX 32767

struct AudioManager {
	int sound_count;
	int __padding;
	Sound sounds[64];
	Fade fades[64];

	int play(EngineApi *engine, const char *filename, float volume = 1.0f) {
		// ASSERT_IN_BOUNDS(sounds, (unsigned)sound_count);

		int id = sound_count++;
		Sound &sound = sounds[id];
		sound.at = 0;

		set_volume(id, volume);

		sound.playing = true;

		engine->audio.load(filename, &sound.buffer, &sound.length);

		return id;
	}

	void set_volume(int id, float volume_ch1, float volume_ch2) {
		fades[id].start = 0xFFFFFFFF;

		Sound &sound = sounds[id];

		sound.volume[0] = volume_ch1;
		sound.volume[1] = volume_ch2;
	}
	inline void set_volume(int id, float volume_ch1) {
		set_volume(id, volume_ch1, volume_ch1);
	}

	void set_fade(int id, float duration, float target_volume_ch1, float target_volume_ch2 = 0.0f) {
		ASSERT(duration > 0.0f, "Can't set fade with 0 duration!");
		Sound &sound = sounds[id];
		Fade &fade = fades[id];

		fade.volume_start[0] = sound.volume[0];
		fade.volume_start[1] = sound.volume[1];

		fade.volume_stop[0] = target_volume_ch1;
		fade.volume_stop[1] = target_volume_ch2 < 0.0f ? target_volume_ch1 : target_volume_ch2;

		fade.start = sound.at;
		fade.duration = duration;
	}
	inline void set_fade(int id, float duration, float target_volume_ch1) {
		set_fade(id, duration, target_volume_ch1, target_volume_ch1);
	}

	void set_playing(int id, bool playing) {
		sounds[id].playing = playing;
	}

	bool is_playing(int id) {
		return sounds[id].playing;
	}

	bool is_alive(int id) {
		return sounds[id].buffer != 0;
	}

	void update(ArenaAllocator &arena, EngineApi *engine, float dt) {
		(void)dt;

		static int frequency = 44100;
		static int sample_count = 4096; // 4096/44100 = 0.09287981859 seconds
		static int channel_count = 2;
		static int byte_count = 2;

		int size = (int) engine->audio.queued_size();
		int queued_samples = size / (channel_count * byte_count);

		// printf("%d %u %f\n", queued_samples, size, dt);
		if (queued_samples > sample_count)
			return;

		int count = sample_count * channel_count * byte_count;
		if (queued_samples == 0) { // We are completely depleted!
			count *= 4; // Add some extra data
		}

		TempAllocator ta(&arena);
		uint8_t *output = (uint8_t*) allocate(&arena, (unsigned)count);
		int channel = 0;

		int active_sound_count = 0;

		// No-one may scratch allocate until this function returns!
		ActiveSound *active_sounds = PUSH(&arena, (unsigned)ARRAY_COUNT(sounds), ActiveSound);
		for (int i = 0; i < (int)ARRAY_COUNT(sounds); i++) {
			Sound &sound = sounds[i];
			if (sound.playing) {
				ActiveSound &active_sound = active_sounds[active_sound_count++];

				active_sound.sound_id = i;
				active_sound.end_count = (int)sound.length - (int)sound.at;
			}
		}

		for (int i = 0; i < count; i+=2) {
			float mixed = 0;

			for (int j = 0; j < active_sound_count; j++) {
				ActiveSound &active_sound = active_sounds[j];
				Sound &sound = sounds[active_sound.sound_id];
				Fade &fade = fades[active_sound.sound_id];

				if (fade.start != 0xFFFFFFFF) {
					int delta = (int)sound.at - (int)fade.start;
					float t = clamp(delta / (frequency*fade.duration), 0, 1);
					sound.volume[channel] = lerp(fade.volume_start[channel], fade.volume_stop[channel], t);

					if (float_equal(t, 1.0f) && channel == 1) { // Only remove the fade if we are on the last channel
						fade.start = 0xFFFFFFFF;
					}
				}

				i16 sample = *(i16*)(sound.buffer + (sound.at + (unsigned)i));
				mixed += sample * sound.volume[channel];

				if (i > active_sound.end_count) {
					active_sounds[j--] = active_sounds[--active_sound_count];
					sound.playing = false;
					sound.buffer = 0;
				}
			}

			*(i16*)(output + i) = (i16) clamp(mixed, SHRT_MIN, SHRT_MAX);

			channel = !channel;
		}

		for (int j = 0; j < active_sound_count; j++) {
			ActiveSound &active_sound = active_sounds[j];
			Sound &sound = sounds[active_sound.sound_id];
			sound.at += (unsigned)count;
		}

		engine->audio.queue(output, (unsigned)count);
	}
};
