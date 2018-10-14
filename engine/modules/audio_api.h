#pragma once

struct AudioApi {
	void (*load)(const char *filename, u8 **buffer, u32 *length);
	void (*queue)(u8 *buffer, u32 length);
	u32 (*queued_size)();
	void (*free)(i16 *buffer);
	void (*set_playing)(bool playing);
};
