struct Sound {
	u32 at;
	u32 length;
	u8 *buffer;
	f32 volume[2];
	b32 playing;
	i32 __padding;
};

struct Fade {
	f32 volume_start[2];
	f32 volume_stop[2];
	f32 duration;
	u32 start;
};

struct ActiveSound {
	i32 sound_id;
	i32 end_count;

	f32 volume_target[2];
};

#define SHRT_MIN -32767
#define SHRT_MAX 32767

struct AudioManager {
	i32 sound_count;
	i32 __padding;
	Sound sounds[64];
	Fade fades[64];

	i32 play(EngineApi *engine, const char *filename, f32 volume = 1.0f) {
		ASSERT_IN_BOUNDS(sounds, (u32)sound_count);

		i32 id = sound_count++;
		Sound &sound = sounds[id];
		sound.at = 0;

		set_volume(id, volume);

		sound.playing = true;

		engine->audio_load(filename, &sound.buffer, &sound.length);

		return id;
	}

	void set_volume(i32 id, f32 volume_ch1, f32 volume_ch2) {
		fades[id].start = 0xFFFFFFFF;

		Sound &sound = sounds[id];

		sound.volume[0] = volume_ch1;
		sound.volume[1] = volume_ch2;
	}
	inline void set_volume(i32 id, f32 volume_ch1) {
		set_volume(id, volume_ch1, volume_ch1);
	}

	void set_fade(i32 id, f32 duration, f32 target_volume_ch1, f32 target_volume_ch2 = 0.0f) {
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
	inline void set_fade(i32 id, f32 duration, f32 target_volume_ch1) {
		set_fade(id, duration, target_volume_ch1, target_volume_ch1);
	}

	void set_playing(i32 id, b32 playing) {
		sounds[id].playing = playing;
	}

	b32 is_playing(i32 id) {
		return sounds[id].playing;
	}

	b32 is_alive(i32 id) {
		return sounds[id].buffer != 0;
	}

	void update(MemoryArena &arena, EngineApi *engine, f32 dt) {
		(void)dt;

		static i32 frequency = 44100;
		static i32 sample_count = 4096; // 4096/44100 = 0.09287981859 seconds
		static i32 channel_count = 2;
		static i32 byte_count = 2;

		i32 size = (i32) engine->audio_queued_size();
		i32 queued_samples = size / (channel_count * byte_count);

		// printf("%d %u %f\n", queued_samples, size, dt);
		if (queued_samples > sample_count)
			return;

		i32 count = sample_count * channel_count * byte_count;
		if (queued_samples == 0) { // We are completely depleted!
			count *= 4; // Add some extra data
		}

		MemoryBlockHandle memory_block = begin_block(arena);
		u8 *output = (u8*) allocate_memory(arena, (u32)count);
		i32 channel = 0;

		i32 active_sound_count = 0;

		// No-one may scratch allocate until this function returns!
		ActiveSound *active_sounds = (ActiveSound *)(arena.memory + arena.offset);
		for (i32 i = 0; i < (i32)ARRAY_COUNT(sounds); i++) {
			Sound &sound = sounds[i];
			if (sound.playing) {
				ActiveSound &active_sound = active_sounds[active_sound_count++];

				active_sound.sound_id = i;
				active_sound.end_count = (i32)sound.length - (i32)sound.at;
			}
		}

		for (i32 i = 0; i < count; i+=2) {
			f32 mixed = 0;

			for (i32 j = 0; j < active_sound_count; j++) {
				ActiveSound &active_sound = active_sounds[j];
				Sound &sound = sounds[active_sound.sound_id];
				Fade &fade = fades[active_sound.sound_id];

				if (fade.start != 0xFFFFFFFF) {
					i32 delta = (i32)sound.at - (i32)fade.start;
					f32 t = clamp(delta / (frequency*fade.duration), 0, 1);
					sound.volume[channel] = lerp(fade.volume_start[channel], fade.volume_stop[channel], t);

					if (float_equal(t, 1.0f) && channel == 1) { // Only remove the fade if we are on the last channel
						fade.start = 0xFFFFFFFF;
					}
				}

				i16 sample = *(i16*)(sound.buffer + (sound.at + (u32)i));
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

		for (i32 j = 0; j < active_sound_count; j++) {
			ActiveSound &active_sound = active_sounds[j];
			Sound &sound = sounds[active_sound.sound_id];
			sound.at += (u32)count;
		}

		engine->audio_queue(output, (u32)count);

		end_block(arena, memory_block);
	}
};
