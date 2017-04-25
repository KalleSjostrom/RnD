#include "../generated/type_info.generated.cpp"
#include "../reload/reloader.cpp"

#define __RELOAD_TYPE__generic 0
#define __RELOAD_SIZE__generic 1
#define __RELOAD_TYPE__int 1
#define __RELOAD_SIZE__int 4
#define __RELOAD_TYPE__bool 2
#define __RELOAD_SIZE__bool 1
#define __RELOAD_TYPE__unsigned 3
#define __RELOAD_SIZE__unsigned 4
#define __RELOAD_TYPE__char 4
#define __RELOAD_SIZE__char 1
#define __RELOAD_TYPE__float 5
#define __RELOAD_SIZE__float 4
#define __RELOAD_TYPE__double 6
#define __RELOAD_SIZE__double 8
#define __RELOAD_TYPE__long 7
#define __RELOAD_SIZE__long 8
#ifndef __RELOAD_TYPE__MemoryArena
	#define __RELOAD_TYPE__MemoryArena 1000000
	#define __RELOAD_SIZE__MemoryArena 0
#endif
#ifndef __RELOAD_OFFSET__MemoryArena_offset
	#define __RELOAD_OFFSET__MemoryArena_offset 0
#endif
#ifndef __RELOAD_OFFSET__MemoryArena_memory
	#define __RELOAD_OFFSET__MemoryArena_memory 0
#endif
#ifndef __RELOAD_OFFSET__MemoryArena__DEBUG_maxsize
	#define __RELOAD_OFFSET__MemoryArena__DEBUG_maxsize 0
#endif
#ifndef __RELOAD_TYPE__AppMemory
	#define __RELOAD_TYPE__AppMemory 1000001
	#define __RELOAD_SIZE__AppMemory 0
#endif
#ifndef __RELOAD_OFFSET__AppMemory_initialized
	#define __RELOAD_OFFSET__AppMemory_initialized 0
#endif
#ifndef __RELOAD_OFFSET__AppMemory_shader_program
	#define __RELOAD_OFFSET__AppMemory_shader_program 0
#endif
#ifndef __RELOAD_OFFSET__AppMemory_vao
	#define __RELOAD_OFFSET__AppMemory_vao 0
#endif
#ifndef __RELOAD_OFFSET__AppMemory_vbo
	#define __RELOAD_OFFSET__AppMemory_vbo 0
#endif
#ifndef __RELOAD_OFFSET__AppMemory_eab
	#define __RELOAD_OFFSET__AppMemory_eab 0
#endif
#ifndef __RELOAD_OFFSET__AppMemory_info
	#define __RELOAD_OFFSET__AppMemory_info 0
#endif
#ifndef __RELOAD_OFFSET__AppMemory_persistent_arena
	#define __RELOAD_OFFSET__AppMemory_persistent_arena 0
#endif
#ifndef __RELOAD_OFFSET__AppMemory_transient_arena
	#define __RELOAD_OFFSET__AppMemory_transient_arena 0
#endif
#ifndef __RELOAD_OFFSET__AppMemory_gui
	#define __RELOAD_OFFSET__AppMemory_gui 0
#endif
#ifndef __RELOAD_OFFSET__AppMemory_fps_string
	#define __RELOAD_OFFSET__AppMemory_fps_string 0
#endif
#ifndef __RELOAD_OFFSET__AppMemory_fps_job_handle
	#define __RELOAD_OFFSET__AppMemory_fps_job_handle 0
#endif
#ifndef __RELOAD_OFFSET__AppMemory_fps_update
	#define __RELOAD_OFFSET__AppMemory_fps_update 0
#endif
#ifndef __RELOAD_OFFSET__AppMemory_fps_frames
	#define __RELOAD_OFFSET__AppMemory_fps_frames 0
#endif
#ifndef __RELOAD_OFFSET__AppMemory_fps_timer
	#define __RELOAD_OFFSET__AppMemory_fps_timer 0
#endif

void generate_layout() {
	FILE *output = fopen("./generated/type_info.generated.cpp", "w");

	fprintf(output, "#define __RELOAD_TYPE__MemoryArena 1000\n");
	fprintf(output, "#define __RELOAD_SIZE__MemoryArena %lu\n", sizeof(MemoryArena));
	fprintf(output, "#define __RELOAD_OFFSET__MemoryArena_offset %lu\n", offsetof(MemoryArena, offset));
	fprintf(output, "#define __RELOAD_OFFSET__MemoryArena_memory %lu\n", offsetof(MemoryArena, memory));
	fprintf(output, "#define __RELOAD_OFFSET__MemoryArena__DEBUG_maxsize %lu\n", offsetof(MemoryArena, _DEBUG_maxsize));
	fprintf(output, "#define __RELOAD_TYPE__AppMemory 1001\n");
	fprintf(output, "#define __RELOAD_SIZE__AppMemory %lu\n", sizeof(AppMemory));
	fprintf(output, "#define __RELOAD_OFFSET__AppMemory_initialized %lu\n", offsetof(AppMemory, initialized));
	fprintf(output, "#define __RELOAD_OFFSET__AppMemory_shader_program %lu\n", offsetof(AppMemory, shader_program));
	fprintf(output, "#define __RELOAD_OFFSET__AppMemory_vao %lu\n", offsetof(AppMemory, vao));
	fprintf(output, "#define __RELOAD_OFFSET__AppMemory_vbo %lu\n", offsetof(AppMemory, vbo));
	fprintf(output, "#define __RELOAD_OFFSET__AppMemory_eab %lu\n", offsetof(AppMemory, eab));
	fprintf(output, "#define __RELOAD_OFFSET__AppMemory_info %lu\n", offsetof(AppMemory, info));
	fprintf(output, "#define __RELOAD_OFFSET__AppMemory_persistent_arena %lu\n", offsetof(AppMemory, persistent_arena));
	fprintf(output, "#define __RELOAD_OFFSET__AppMemory_transient_arena %lu\n", offsetof(AppMemory, transient_arena));
	fprintf(output, "#define __RELOAD_OFFSET__AppMemory_gui %lu\n", offsetof(AppMemory, gui));
	fprintf(output, "#define __RELOAD_OFFSET__AppMemory_fps_string %lu\n", offsetof(AppMemory, fps_string));
	fprintf(output, "#define __RELOAD_OFFSET__AppMemory_fps_job_handle %lu\n", offsetof(AppMemory, fps_job_handle));
	fprintf(output, "#define __RELOAD_OFFSET__AppMemory_fps_update %lu\n", offsetof(AppMemory, fps_update));
	fprintf(output, "#define __RELOAD_OFFSET__AppMemory_fps_frames %lu\n", offsetof(AppMemory, fps_frames));
	fprintf(output, "#define __RELOAD_OFFSET__AppMemory_fps_timer %lu\n", offsetof(AppMemory, fps_timer));

	fclose(output);
}
void set_MemoryArena(MemoryMapper *mapper, intptr_t base_old, intptr_t base_new) {
	unsigned size;
	set_generic(mapper, base_old + __RELOAD_OFFSET__MemoryArena_offset, base_new + offsetof(MemoryArena, offset), sizeof(size_t));
	size = *(unsigned*)((base_old + __RELOAD_OFFSET__MemoryArena_offset));
	set_pointer(mapper, base_old + __RELOAD_OFFSET__MemoryArena_memory, base_new + offsetof(MemoryArena, memory), __RELOAD_TYPE__generic, size);
	set_generic(mapper, base_old + __RELOAD_OFFSET__MemoryArena__DEBUG_maxsize, base_new + offsetof(MemoryArena, _DEBUG_maxsize), sizeof(size_t));
}

void set_AppMemory(MemoryMapper *mapper, intptr_t base_old, intptr_t base_new) {
	unsigned size;
	set_generic(mapper, base_old + __RELOAD_OFFSET__AppMemory_initialized, base_new + offsetof(AppMemory, initialized), sizeof(bool));
	set_generic(mapper, base_old + __RELOAD_OFFSET__AppMemory_shader_program, base_new + offsetof(AppMemory, shader_program), sizeof(GLuint));
	set_generic(mapper, base_old + __RELOAD_OFFSET__AppMemory_vao, base_new + offsetof(AppMemory, vao), sizeof(GLuint));
	set_generic(mapper, base_old + __RELOAD_OFFSET__AppMemory_vbo, base_new + offsetof(AppMemory, vbo), sizeof(GLuint));
	set_generic(mapper, base_old + __RELOAD_OFFSET__AppMemory_eab, base_new + offsetof(AppMemory, eab), sizeof(GLuint));
	set_generic(mapper, base_old + __RELOAD_OFFSET__AppMemory_info, base_new + offsetof(AppMemory, info), sizeof(cl_manager::ClInfo));
	set_MemoryArena(mapper, base_old + __RELOAD_OFFSET__AppMemory_persistent_arena, base_new + offsetof(AppMemory, persistent_arena));
	set_MemoryArena(mapper, base_old + __RELOAD_OFFSET__AppMemory_transient_arena, base_new + offsetof(AppMemory, transient_arena));
	size = sizeof(gui::GUI);
	set_pointer(mapper, base_old + __RELOAD_OFFSET__AppMemory_gui, base_new + offsetof(AppMemory, gui), __RELOAD_TYPE__generic, size);
	size = sizeof(char);
	set_pointer(mapper, base_old + __RELOAD_OFFSET__AppMemory_fps_string, base_new + offsetof(AppMemory, fps_string), __RELOAD_TYPE__generic, size);
	set_generic(mapper, base_old + __RELOAD_OFFSET__AppMemory_fps_job_handle, base_new + offsetof(AppMemory, fps_job_handle), sizeof(u16));
	set_generic(mapper, base_old + __RELOAD_OFFSET__AppMemory_fps_update, base_new + offsetof(AppMemory, fps_update), sizeof(bool));
	set_generic(mapper, base_old + __RELOAD_OFFSET__AppMemory_fps_frames, base_new + offsetof(AppMemory, fps_frames), sizeof(int));
	set_generic(mapper, base_old + __RELOAD_OFFSET__AppMemory_fps_timer, base_new + offsetof(AppMemory, fps_timer), sizeof(float));
}

void reload(char *old_memory, char *new_memory) {
	AddressPair address_lookup[ADDRESS_LOOKUP_SIZE] = {0};
	MemorySlot memory_slots[1024] = {0};

	MemoryMapper mapper = {0};
	mapper.address_lookup = address_lookup;
	mapper.address_lookup_size = ADDRESS_LOOKUP_SIZE;
	mapper.memory_slots = memory_slots;
	mapper.memory_slot_counter = 0;
	{
		intptr_t base_new = (intptr_t) new_memory;
		intptr_t base_old = (intptr_t) old_memory;

		MemorySlot entry_point = {0};
		entry_point.type = __RELOAD_TYPE__AppMemory;
		entry_point.target_addr_old = base_old;
		memory_slots[0] = entry_point;
		mapper.memory_slot_counter = 1;
		MemorySlot *top = memory_slots;

		while (top) {
			intptr_t distance = top->target_addr_old - base_old;
			base_new += distance;
			base_old += distance;

			unsigned size = top->size > 0 ? top->size : 1;

			switch (top->type) {
				case __RELOAD_TYPE__MemoryArena: {
					for (int i = 0; i < size; ++i) {
						set_MemoryArena(&mapper, base_old, base_new);
						base_new += sizeof(MemoryArena);
						base_old += __RELOAD_SIZE__MemoryArena;
					}
				} break;
				case __RELOAD_TYPE__AppMemory: {
					for (int i = 0; i < size; ++i) {
						set_AppMemory(&mapper, base_old, base_new);
						base_new += sizeof(AppMemory);
						base_old += __RELOAD_SIZE__AppMemory;
					}
				} break;
				case __RELOAD_TYPE__generic: {
					set_generic(&mapper, top->target_addr_old, base_new, top->size);
					base_new += top->size;
					base_old += top->size;
				} break;
				default: {
					ASSERT(false, "Unknown reload type!")
				}
			}

			top = 0;
			for (int i = mapper.memory_slot_counter-1; i >= 0; i--) {
				MemorySlot *entry = memory_slots + i;
				if (entry->target == 0) {
					memory_slots[i] = memory_slots[mapper.memory_slot_counter-1];
					mapper.memory_slot_counter--;
				} else if (entry->target->addr_old != 0) {
					*(intptr_t*)(entry->addr_new) = entry->target->addr_new;
					memory_slots[i] = memory_slots[mapper.memory_slot_counter-1];
					mapper.memory_slot_counter--;
				} else {
					if (top) {
						top = top->target_addr_old < entry->target_addr_old ? top : entry;
					} else {
						top = entry;
					}
				}
			}
		}
	}
}
