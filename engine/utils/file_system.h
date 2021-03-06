enum CacheState {
	CacheState_Unmodified = 1 << 0,
	CacheState_Added      = 1 << 1,
	CacheState_Removed    = 1 << 2,
	CacheState_Modified   = 1 << 3,

	CacheState_All        = 0xffffffff,
};

struct CacheEntry {
	FILETIME file_time;
	u32 filetype;
	u64 key;
};
struct Cache {
	CacheEntry *entries;
	bool *touched;
	u32 max_count;

	bool editor_mode;
	bool valid;
};

struct FileInfo {
	String filepath;
	CacheState state;
	u32 filetype;
	u64 key;
};

// From settings
struct Ending {
	String string;

	u32 id;
	// u64 mask;

	Ending *next;
};

struct FileSystem {
	Ending ending_storage[64];
	Ending ending_lookup[128];

	Allocator allocator;

	String root;
	DynamicString source_folder;
	DynamicString data_folder;
	DynamicString output_folder;
	u64 output_folder_id;

	Cache cache;
	FileInfo *file_infos;

	i32 ending_storage_count;
};
