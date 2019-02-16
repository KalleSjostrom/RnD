// Common & core
#include "common.h"
#include "logging.h"
#include "error.h"

// Utils
#include "core/utils/string_id.h"
#include "core/memory/allocator.h"
#include "core/memory/arena_allocator.h"
#include "core/containers/array.h"
#include "core/containers/hashmap.h"
#include "core/utils/quick_sort.h"

// Includes from microsoft
#define _VC_VER_INC
#include "engine/reloader/include/cvinfo.h"

// Parsing pdb
#include "engine/reloader/pdb/stream.h"
#include "engine/reloader/pdb/type_stream.h"
#include "engine/reloader/pdb/type_stream.cpp"
#include "engine/reloader/pdb/pdb_reader.cpp"

// Reloading and patching up memory
#include "engine/reloader/allocator_reloader_backend.h"
#include "engine/reloader/type_lookup.h"
#include "engine/reloader/type_pairs.cpp"
#include "engine/reloader/reloader.cpp"

// Reloader module
#include "reloader.h"

struct ReloaderContextInternal {
	uint64_t entry_name_id;
	TypeContext type_context;
};

void *reloader_setup(Allocator *a, const char *pdb_name, const char *entry_name) {
	TypeContext type_context = {};
	int success = parse_pdb(*a, pdb_name, &type_context);
	if (success) {
		ReloaderContextInternal *context = (ReloaderContextInternal*) allocate(a, sizeof(ReloaderContextInternal));
		context->type_context = type_context;
		context->entry_name_id = to_id64((unsigned)strlen(entry_name), entry_name);

		TypeRecord *entry_record = get_type_record(&context->type_context, context->entry_name_id);
		gather_type_information(&context->type_context, entry_record->type_index);

		return context;
	}
	return 0;
}

int reloader_reload(Allocator *a, const char *pdb_name, ReloadHeader *reload_header, void *handle) {
	int success = 0;
	if (handle) {
		ReloaderContextInternal *context = (ReloaderContextInternal*)handle;
		TypeContext type_context = {};
		success = parse_pdb(*a, pdb_name, &type_context);
		if (success) {
			TypeRecord *entry_record = get_type_record(&type_context, context->entry_name_id);
			gather_type_information(&type_context, entry_record->type_index);

			patch_memory(*a, pdb_name, context->entry_name_id, context->type_context, type_context, reload_header->new_allocator, reload_header->old_allocator, reload_header->memory_size);

			// TODO(kalle): We should swap in the context here to avoid having to re-read it on the next reload!
			// context->type_context = new_type_context;
		}
	}

	return success;
}
