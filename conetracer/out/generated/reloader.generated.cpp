#include "typeinfo.generated.cpp"

#include "reload/reloader.cpp"

// We need this to be a struct with static functions so that reloadable structs can declare this a friend.
// Otherwise, we can't access the private members of such a class.
struct Reloader {
static void generate_layout() {
	#if PS4
		#define TYPEINFO_FILEPATH ("/host/" __FILE__)
	#else
		#define TYPEINFO_FILEPATH (__FILE__)
	#endif
	#define RELOADER_PATHLENGTH (sizeof(TYPEINFO_FILEPATH)-1)
	char typeinfo_filepath[RELOADER_PATHLENGTH+1];
	char ending[] = "typeinfo.generated.cpp";
	unsigned start_of_filename = RELOADER_PATHLENGTH - (sizeof(ending)-1);
	for (unsigned i = 0; i < start_of_filename; ++i) {
		if (TYPEINFO_FILEPATH[i] == '\\')
			typeinfo_filepath[i] = '/';
		else
			typeinfo_filepath[i] = TYPEINFO_FILEPATH[i];
	}
	for (unsigned i = 0; i < (sizeof(ending)-1); ++i)
		typeinfo_filepath[i + start_of_filename] = ending[i];
	typeinfo_filepath[RELOADER_PATHLENGTH] = '\0';

	FILE *output = fopen(typeinfo_filepath, "w");

	fprintf(output, "enum ReloadType {\n");
	fprintf(output, "	ReloadType_void,\n");
	fprintf(output, "	ReloadType_generic,\n");
	fprintf(output, "	ReloadType_int,\n");
	fprintf(output, "	ReloadType_bool,\n");
	fprintf(output, "	ReloadType_unsigned,\n");
	fprintf(output, "	ReloadType_char,\n");
	fprintf(output, "	ReloadType_float,\n");
	fprintf(output, "	ReloadType_double,\n");
	fprintf(output, "	ReloadType_long,\n");
	fprintf(output, "	ReloadType_BaseType,\n");
	fprintf(output, "	ReloadType_Reloader,\n");
	fprintf(output, "	ReloadType_EntityData,\n");
	fprintf(output, "	ReloadType_Level,\n");
	fprintf(output, "	ReloadType_Game,\n");
	fprintf(output, "	ReloadType_ComponentGroup,\n");
	fprintf(output, "	ReloadType_Context,\n");
	fprintf(output, "	ReloadType_Entity,\n");
	fprintf(output, "	ReloadType_cl_shaders__Entity,\n");
	fprintf(output, "	ReloadType_FBO,\n");
	fprintf(output, "	ReloadType_RenderPipe,\n");
	fprintf(output, "}; \n");
	fprintf(output, "#define __RELOAD_SIZE__int 4\n");
	fprintf(output, "#define __RELOAD_SIZE__bool 1\n");
	fprintf(output, "#define __RELOAD_SIZE__unsigned 4\n");
	fprintf(output, "#define __RELOAD_SIZE__char 1\n");
	fprintf(output, "#define __RELOAD_SIZE__float 4\n");
	fprintf(output, "#define __RELOAD_SIZE__double 8\n");
	fprintf(output, "#define __RELOAD_SIZE__long 8\n");
	fprintf(output, "#define __RELOAD_SIZE__Reloader %zu\n", sizeof(Reloader));
	fprintf(output, "#define __RELOAD_SIZE__EntityData %zu\n", sizeof(EntityData));
	fprintf(output, "#define __RELOAD_OFFSET__EntityData_type %zu\n", offsetof(EntityData, type));
	fprintf(output, "#define __RELOAD_OFFSET__EntityData_offset %zu\n", offsetof(EntityData, offset));
	fprintf(output, "#define __RELOAD_OFFSET__EntityData_size %zu\n", offsetof(EntityData, size));
	fprintf(output, "#define __RELOAD_OFFSET__EntityData_rotation %zu\n", offsetof(EntityData, rotation));
	fprintf(output, "#define __RELOAD_OFFSET__EntityData_context %zu\n", offsetof(EntityData, context));
	fprintf(output, "#define __RELOAD_SIZE__Level %zu\n", sizeof(Level));
	fprintf(output, "#define __RELOAD_OFFSET__Level_count %zu\n", offsetof(Level, count));
	fprintf(output, "#define __RELOAD_OFFSET__Level_entity_data %zu\n", offsetof(Level, entity_data));
	fprintf(output, "#define __RELOAD_SIZE__Game %zu\n", sizeof(Game));
	fprintf(output, "#define __RELOAD_OFFSET__Game_persistent_arena %zu\n", offsetof(Game, persistent_arena));
	fprintf(output, "#define __RELOAD_OFFSET__Game_transient_arena %zu\n", offsetof(Game, transient_arena));
	fprintf(output, "#define __RELOAD_OFFSET__Game_components %zu\n", offsetof(Game, components));
	fprintf(output, "#define __RELOAD_OFFSET__Game_audio_manager %zu\n", offsetof(Game, audio_manager));
	fprintf(output, "#define __RELOAD_OFFSET__Game_render_pipe %zu\n", offsetof(Game, render_pipe));
	fprintf(output, "#define __RELOAD_OFFSET__Game_camera %zu\n", offsetof(Game, camera));
	fprintf(output, "#define __RELOAD_OFFSET__Game_random %zu\n", offsetof(Game, random));
	fprintf(output, "#define __RELOAD_OFFSET__Game_engine %zu\n", offsetof(Game, engine));
	fprintf(output, "#define __RELOAD_OFFSET__Game_cl_info %zu\n", offsetof(Game, cl_info));
	fprintf(output, "#define __RELOAD_OFFSET__Game_input_mem %zu\n", offsetof(Game, input_mem));
	fprintf(output, "#define __RELOAD_OFFSET__Game_output_mem %zu\n", offsetof(Game, output_mem));
	fprintf(output, "#define __RELOAD_OFFSET__Game_kernel %zu\n", offsetof(Game, kernel));
	fprintf(output, "#define __RELOAD_OFFSET__Game_initialized %zu\n", offsetof(Game, initialized));
	fprintf(output, "#define __RELOAD_OFFSET__Game_entity_count %zu\n", offsetof(Game, entity_count));
	fprintf(output, "#define __RELOAD_OFFSET__Game_entities %zu\n", offsetof(Game, entities));
	fprintf(output, "#define __RELOAD_SIZE__ComponentGroup %zu\n", sizeof(ComponentGroup));
	fprintf(output, "#define __RELOAD_OFFSET__ComponentGroup_input %zu\n", offsetof(ComponentGroup, input));
	fprintf(output, "#define __RELOAD_OFFSET__ComponentGroup_animation %zu\n", offsetof(ComponentGroup, animation));
	fprintf(output, "#define __RELOAD_OFFSET__ComponentGroup_mover %zu\n", offsetof(ComponentGroup, mover));
	fprintf(output, "#define __RELOAD_OFFSET__ComponentGroup_model %zu\n", offsetof(ComponentGroup, model));
	fprintf(output, "#define __RELOAD_OFFSET__ComponentGroup_actor %zu\n", offsetof(ComponentGroup, actor));
	fprintf(output, "#define __RELOAD_OFFSET__ComponentGroup_material %zu\n", offsetof(ComponentGroup, material));
	fprintf(output, "#define __RELOAD_OFFSET__ComponentGroup_renderer %zu\n", offsetof(ComponentGroup, renderer));
	fprintf(output, "#define __RELOAD_SIZE__Context %zu\n", sizeof(Context));
	fprintf(output, "#define __RELOAD_OFFSET__Context_material %zu\n", offsetof(Context, material));
	fprintf(output, "#define __RELOAD_SIZE__Entity %zu\n", sizeof(Entity));
	fprintf(output, "#define __RELOAD_OFFSET__Entity_type %zu\n", offsetof(Entity, type));
	fprintf(output, "#define __RELOAD_OFFSET__Entity_input_id %zu\n", offsetof(Entity, input_id));
	fprintf(output, "#define __RELOAD_OFFSET__Entity_animation_id %zu\n", offsetof(Entity, animation_id));
	fprintf(output, "#define __RELOAD_OFFSET__Entity_mover_id %zu\n", offsetof(Entity, mover_id));
	fprintf(output, "#define __RELOAD_OFFSET__Entity_model_id %zu\n", offsetof(Entity, model_id));
	fprintf(output, "#define __RELOAD_OFFSET__Entity_actor_id %zu\n", offsetof(Entity, actor_id));
	fprintf(output, "#define __RELOAD_OFFSET__Entity_material_id %zu\n", offsetof(Entity, material_id));
	fprintf(output, "#define __RELOAD_SIZE__cl_shaders__Entity %zu\n", sizeof(cl_shaders::Entity));
	fprintf(output, "#define __RELOAD_OFFSET__cl_shaders__Entity_type %zu\n", offsetof(cl_shaders::Entity, type));
	fprintf(output, "#define __RELOAD_OFFSET__cl_shaders__Entity_position %zu\n", offsetof(cl_shaders::Entity, position));
	fprintf(output, "#define __RELOAD_OFFSET__cl_shaders__Entity_data %zu\n", offsetof(cl_shaders::Entity, data));
	fprintf(output, "#define __RELOAD_OFFSET__cl_shaders__Entity_emittance_color %zu\n", offsetof(cl_shaders::Entity, emittance_color));
	fprintf(output, "#define __RELOAD_OFFSET__cl_shaders__Entity_reflection_color %zu\n", offsetof(cl_shaders::Entity, reflection_color));
	fprintf(output, "#define __RELOAD_OFFSET__cl_shaders__Entity_roughness %zu\n", offsetof(cl_shaders::Entity, roughness));
	fprintf(output, "#define __RELOAD_SIZE__FBO %zu\n", sizeof(FBO));
	fprintf(output, "#define __RELOAD_OFFSET__FBO_framebuffer_object %zu\n", offsetof(FBO, framebuffer_object));
	fprintf(output, "#define __RELOAD_OFFSET__FBO_count %zu\n", offsetof(FBO, count));
	fprintf(output, "#define __RELOAD_OFFSET__FBO_render_texture %zu\n", offsetof(FBO, render_texture));
	fprintf(output, "#define __RELOAD_SIZE__RenderPipe %zu\n", sizeof(RenderPipe));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_screen_width %zu\n", offsetof(RenderPipe, screen_width));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_screen_height %zu\n", offsetof(RenderPipe, screen_height));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_fullscreen_quad %zu\n", offsetof(RenderPipe, fullscreen_quad));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_passthrough_program %zu\n", offsetof(RenderPipe, passthrough_program));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_passthrough_render_texture_location %zu\n", offsetof(RenderPipe, passthrough_render_texture_location));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_ray_texture %zu\n", offsetof(RenderPipe, ray_texture));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_buffer %zu\n", offsetof(RenderPipe, buffer));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_image_data %zu\n", offsetof(RenderPipe, image_data));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_programs %zu\n", offsetof(RenderPipe, programs));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_program_count %zu\n", offsetof(RenderPipe, program_count));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe___padding %zu\n", offsetof(RenderPipe, __padding));

	fclose(output);
}
static void reload(void *old_mspace, size_t old_size, void *new_mspace, char *temporary_memory) {
	malloc_state *malloc_state_old = (malloc_state *)(old_mspace);
	malloc_state *malloc_state_new = (malloc_state *)(new_mspace);

	malloc_chunk *current_malloc_chunk_old = (malloc_chunk*)mem2chunk(old_mspace);
	malloc_chunk *current_malloc_chunk_new = (malloc_chunk*)mem2chunk(new_mspace);

	intptr_t old_memory = (intptr_t)current_malloc_chunk_old;
	intptr_t new_memory = (intptr_t)current_malloc_chunk_new;

	PointerContext context = setup_pointer_context(temporary_memory, old_memory, old_size);

	{ // Gather pointers
		// TODO(kalle): Clone the malloc_state? Should we keep the information about the freed objects?

		current_malloc_chunk_old = next_chunk(current_malloc_chunk_old);
		current_malloc_chunk_new = next_chunk(current_malloc_chunk_new);

		intptr_t base_old = (intptr_t)chunk2mem(current_malloc_chunk_old);
		intptr_t base_new = (intptr_t)chunk2mem(current_malloc_chunk_new);

		// We require to know about the "entry point", i.e. the layout of where the input memory is pointing.
		Pointer cursor = make_entry_pointer(base_old, ReloadType_game__ReloaderEntryPoint);

		bool done = false;
		while (!done) {
			// We have reached the end of a new:ed struct. We need to look at the next newed portion, i.e. the next malloc chunk to find out where to go next.

			malloc_chunk *previous = current_malloc_chunk_old;
			// Go to the next chunk
			current_malloc_chunk_old = next_chunk(current_malloc_chunk_old);
			current_malloc_chunk_new = next_chunk(current_malloc_chunk_new);

			base_old = (intptr_t)chunk2mem(current_malloc_chunk_old);
			base_new = (intptr_t)chunk2mem(current_malloc_chunk_new);

			size_t size_from_chunk = estimate_unpadded_request(chunksize(current_malloc_chunk_old));
			if (base_old + size_from_chunk >= old_memory + old_size) {
				intptr_t used_space_new = base_new - new_memory;
				intptr_t used_space_old = base_old - old_memory;
				intptr_t size_diff = used_space_new - used_space_old;
				// If we have used more space in the new state, we have less size left to init the top with
				current_malloc_chunk_new->head = current_malloc_chunk_old->head - size_diff;
				init_top(malloc_state_new, current_malloc_chunk_new, chunksize(current_malloc_chunk_new));
				done = true;
			} else {
				bool found_valid_cursor = false;
				while (!found_valid_cursor) {
					bool success = search_for_next_reloadable_pointer(context, &cursor);
					ASSERT(success, "Could not find another pointer to follow!");

					// Check to see if this pointed in to already mapped memory. If so it's useless to us, and we'll continue to the next one.
					intptr_t target_addr_old = *(intptr_t*)cursor.addr_old;
					found_valid_cursor = target_addr_old >= base_old; // A valid cursor is memory we _haven't_ seen, i.e. context we want to follow.
					ASSERT(found_valid_cursor, "We should only have valid pointers left! Since it shouldn't be possible to point back");
					*(intptr_t*)(cursor.addr_new) = base_new;
				}
			}
		}
	}
	// Reset state for the next iteration
	current_malloc_chunk_old = (malloc_chunk*)old_memory;
	current_malloc_chunk_new = (malloc_chunk*)new_memory;

	{ // Copy the data
		current_malloc_chunk_old = next_chunk(current_malloc_chunk_old);
		current_malloc_chunk_new = next_chunk(current_malloc_chunk_new);

		intptr_t base_old = (intptr_t)chunk2mem(current_malloc_chunk_old);
		intptr_t base_new = (intptr_t)chunk2mem(current_malloc_chunk_new);

		sort_pointers(context, base_old, old_size);

		// We require to know about the "entry point", i.e. the layout of where the input memory is pointing.
		Pointer cursor = make_entry_pointer(base_old, ReloadType_game__ReloaderEntryPoint);

		bool done = false;
		while (!done) {
			// We have reached the end of a new:ed struct. We need to look at the next newed portion, i.e. the next malloc chunk to find out where to go next.

			malloc_chunk *previous = current_malloc_chunk_old;
			// Go to the next chunk
			current_malloc_chunk_old = next_chunk(current_malloc_chunk_old);
			current_malloc_chunk_new = next_chunk(current_malloc_chunk_new);

			base_old = (intptr_t)chunk2mem(current_malloc_chunk_old);
			base_new = (intptr_t)chunk2mem(current_malloc_chunk_new);

			size_t size_from_chunk = estimate_unpadded_request(chunksize(current_malloc_chunk_old));
			if (base_old + size_from_chunk >= old_memory + old_size) {
				done = true;
			} else {
				get_next_reloadable_pointer(context, &cursor);
			}
		}
	}
}
};
