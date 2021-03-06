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
	fprintf(output, "	ReloadType_ComponentGroup,\n");
	fprintf(output, "	ReloadType_Entity,\n");
	fprintf(output, "	ReloadType_GASettings,\n");
	fprintf(output, "	ReloadType_Game,\n");
	fprintf(output, "	ReloadType_EntityData,\n");
	fprintf(output, "	ReloadType_Level,\n");
	fprintf(output, "	ReloadType_Neuron,\n");
	fprintf(output, "	ReloadType_NeuralNet,\n");
	fprintf(output, "	ReloadType_NeuronInputOutput,\n");
	fprintf(output, "	ReloadType_FloatArray,\n");
	fprintf(output, "	ReloadType_NeuronLayer,\n");
	fprintf(output, "	ReloadType_FBO,\n");
	fprintf(output, "	ReloadType_RenderPipe,\n");
	fprintf(output, "	ReloadType_Reloader,\n");
	fprintf(output, "	ReloadType_Buffer,\n");
	fprintf(output, "}; \n");
	fprintf(output, "#define __RELOAD_SIZE__int 4\n");
	fprintf(output, "#define __RELOAD_SIZE__bool 1\n");
	fprintf(output, "#define __RELOAD_SIZE__unsigned 4\n");
	fprintf(output, "#define __RELOAD_SIZE__char 1\n");
	fprintf(output, "#define __RELOAD_SIZE__float 4\n");
	fprintf(output, "#define __RELOAD_SIZE__double 8\n");
	fprintf(output, "#define __RELOAD_SIZE__long 8\n");
	fprintf(output, "#define __RELOAD_SIZE__ComponentGroup %zu\n", sizeof(ComponentGroup));
	fprintf(output, "#define __RELOAD_OFFSET__ComponentGroup_input %zu\n", offsetof(ComponentGroup, input));
	fprintf(output, "#define __RELOAD_OFFSET__ComponentGroup_animation %zu\n", offsetof(ComponentGroup, animation));
	fprintf(output, "#define __RELOAD_OFFSET__ComponentGroup_mover %zu\n", offsetof(ComponentGroup, mover));
	fprintf(output, "#define __RELOAD_OFFSET__ComponentGroup_model %zu\n", offsetof(ComponentGroup, model));
	fprintf(output, "#define __RELOAD_OFFSET__ComponentGroup_actor %zu\n", offsetof(ComponentGroup, actor));
	fprintf(output, "#define __RELOAD_OFFSET__ComponentGroup_renderer %zu\n", offsetof(ComponentGroup, renderer));
	fprintf(output, "#define __RELOAD_OFFSET__ComponentGroup___padding %zu\n", offsetof(ComponentGroup, __padding));
	fprintf(output, "#define __RELOAD_OFFSET__ComponentGroup____padding %zu\n", offsetof(ComponentGroup, ___padding));
	fprintf(output, "#define __RELOAD_SIZE__Entity %zu\n", sizeof(Entity));
	fprintf(output, "#define __RELOAD_OFFSET__Entity_type %zu\n", offsetof(Entity, type));
	fprintf(output, "#define __RELOAD_OFFSET__Entity_input_id %zu\n", offsetof(Entity, input_id));
	fprintf(output, "#define __RELOAD_OFFSET__Entity_animation_id %zu\n", offsetof(Entity, animation_id));
	fprintf(output, "#define __RELOAD_OFFSET__Entity_mover_id %zu\n", offsetof(Entity, mover_id));
	fprintf(output, "#define __RELOAD_OFFSET__Entity_model_id %zu\n", offsetof(Entity, model_id));
	fprintf(output, "#define __RELOAD_OFFSET__Entity_actor_id %zu\n", offsetof(Entity, actor_id));
	fprintf(output, "#define __RELOAD_SIZE__GASettings %zu\n", sizeof(GASettings));
	fprintf(output, "#define __RELOAD_OFFSET__GASettings_max_fitness %zu\n", offsetof(GASettings, max_fitness));
	fprintf(output, "#define __RELOAD_OFFSET__GASettings_min_fitness %zu\n", offsetof(GASettings, min_fitness));
	fprintf(output, "#define __RELOAD_OFFSET__GASettings_max_iterations %zu\n", offsetof(GASettings, max_iterations));
	fprintf(output, "#define __RELOAD_OFFSET__GASettings_population_size %zu\n", offsetof(GASettings, population_size));
	fprintf(output, "#define __RELOAD_OFFSET__GASettings_mutation_probability %zu\n", offsetof(GASettings, mutation_probability));
	fprintf(output, "#define __RELOAD_OFFSET__GASettings_selection_type %zu\n", offsetof(GASettings, selection_type));
	fprintf(output, "#define __RELOAD_OFFSET__GASettings_mating_type %zu\n", offsetof(GASettings, mating_type));
	fprintf(output, "#define __RELOAD_OFFSET__GASettings_mutation_type %zu\n", offsetof(GASettings, mutation_type));
	fprintf(output, "#define __RELOAD_SIZE__Game %zu\n", sizeof(Game));
	fprintf(output, "#define __RELOAD_OFFSET__Game_persistent_arena %zu\n", offsetof(Game, persistent_arena));
	fprintf(output, "#define __RELOAD_OFFSET__Game_transient_arena %zu\n", offsetof(Game, transient_arena));
	fprintf(output, "#define __RELOAD_OFFSET__Game_components %zu\n", offsetof(Game, components));
	fprintf(output, "#define __RELOAD_OFFSET__Game_audio_manager %zu\n", offsetof(Game, audio_manager));
	fprintf(output, "#define __RELOAD_OFFSET__Game_render_pipe %zu\n", offsetof(Game, render_pipe));
	fprintf(output, "#define __RELOAD_OFFSET__Game_camera %zu\n", offsetof(Game, camera));
	fprintf(output, "#define __RELOAD_OFFSET__Game_engine %zu\n", offsetof(Game, engine));
	fprintf(output, "#define __RELOAD_OFFSET__Game_initialized %zu\n", offsetof(Game, initialized));
	fprintf(output, "#define __RELOAD_OFFSET__Game_entity_count %zu\n", offsetof(Game, entity_count));
	fprintf(output, "#define __RELOAD_OFFSET__Game_entities %zu\n", offsetof(Game, entities));
	fprintf(output, "#define __RELOAD_SIZE__EntityData %zu\n", sizeof(EntityData));
	fprintf(output, "#define __RELOAD_OFFSET__EntityData_type %zu\n", offsetof(EntityData, type));
	fprintf(output, "#define __RELOAD_OFFSET__EntityData_x %zu\n", offsetof(EntityData, x));
	fprintf(output, "#define __RELOAD_OFFSET__EntityData_y %zu\n", offsetof(EntityData, y));
	fprintf(output, "#define __RELOAD_OFFSET__EntityData_w %zu\n", offsetof(EntityData, w));
	fprintf(output, "#define __RELOAD_OFFSET__EntityData_h %zu\n", offsetof(EntityData, h));
	fprintf(output, "#define __RELOAD_OFFSET__EntityData_rotation %zu\n", offsetof(EntityData, rotation));
	fprintf(output, "#define __RELOAD_SIZE__Level %zu\n", sizeof(Level));
	fprintf(output, "#define __RELOAD_OFFSET__Level_count %zu\n", offsetof(Level, count));
	fprintf(output, "#define __RELOAD_OFFSET__Level_entity_data %zu\n", offsetof(Level, entity_data));
	fprintf(output, "#define __RELOAD_SIZE__Neuron %zu\n", sizeof(Neuron));
	fprintf(output, "#define __RELOAD_OFFSET__Neuron_weights %zu\n", offsetof(Neuron, weights));
	fprintf(output, "#define __RELOAD_OFFSET__Neuron_count %zu\n", offsetof(Neuron, count));
	fprintf(output, "#define __RELOAD_SIZE__NeuralNet %zu\n", sizeof(NeuralNet));
	fprintf(output, "#define __RELOAD_OFFSET__NeuralNet_layers %zu\n", offsetof(NeuralNet, layers));
	fprintf(output, "#define __RELOAD_OFFSET__NeuralNet_layer_count %zu\n", offsetof(NeuralNet, layer_count));
	fprintf(output, "#define __RELOAD_SIZE__NeuronInputOutput %zu\n", sizeof(NeuronInputOutput));
	fprintf(output, "#define __RELOAD_OFFSET__NeuronInputOutput_input %zu\n", offsetof(NeuronInputOutput, input));
	fprintf(output, "#define __RELOAD_OFFSET__NeuronInputOutput_count %zu\n", offsetof(NeuronInputOutput, count));
	fprintf(output, "#define __RELOAD_SIZE__FloatArray %zu\n", sizeof(FloatArray));
	fprintf(output, "#define __RELOAD_OFFSET__FloatArray_values %zu\n", offsetof(FloatArray, values));
	fprintf(output, "#define __RELOAD_OFFSET__FloatArray_count %zu\n", offsetof(FloatArray, count));
	fprintf(output, "#define __RELOAD_SIZE__NeuronLayer %zu\n", sizeof(NeuronLayer));
	fprintf(output, "#define __RELOAD_OFFSET__NeuronLayer_neurons %zu\n", offsetof(NeuronLayer, neurons));
	fprintf(output, "#define __RELOAD_OFFSET__NeuronLayer_outputs %zu\n", offsetof(NeuronLayer, outputs));
	fprintf(output, "#define __RELOAD_OFFSET__NeuronLayer_neuron_count %zu\n", offsetof(NeuronLayer, neuron_count));
	fprintf(output, "#define __RELOAD_SIZE__FBO %zu\n", sizeof(FBO));
	fprintf(output, "#define __RELOAD_OFFSET__FBO_framebuffer_object %zu\n", offsetof(FBO, framebuffer_object));
	fprintf(output, "#define __RELOAD_OFFSET__FBO_count %zu\n", offsetof(FBO, count));
	fprintf(output, "#define __RELOAD_OFFSET__FBO_render_texture %zu\n", offsetof(FBO, render_texture));
	fprintf(output, "#define __RELOAD_SIZE__RenderPipe %zu\n", sizeof(RenderPipe));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_screen_width %zu\n", offsetof(RenderPipe, screen_width));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_screen_height %zu\n", offsetof(RenderPipe, screen_height));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_fullscreen_quad %zu\n", offsetof(RenderPipe, fullscreen_quad));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_scene %zu\n", offsetof(RenderPipe, scene));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_shadowmap %zu\n", offsetof(RenderPipe, shadowmap));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_lightmap %zu\n", offsetof(RenderPipe, lightmap));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_bloom %zu\n", offsetof(RenderPipe, bloom));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_light_colors %zu\n", offsetof(RenderPipe, light_colors));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_light_positions %zu\n", offsetof(RenderPipe, light_positions));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_light_radii %zu\n", offsetof(RenderPipe, light_radii));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_passthrough_program %zu\n", offsetof(RenderPipe, passthrough_program));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_passthrough_render_texture_location %zu\n", offsetof(RenderPipe, passthrough_render_texture_location));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_shadowmap_program %zu\n", offsetof(RenderPipe, shadowmap_program));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_shadowmap_scene_location %zu\n", offsetof(RenderPipe, shadowmap_scene_location));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_shadowmap_light_positions_location %zu\n", offsetof(RenderPipe, shadowmap_light_positions_location));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_shadowmap_light_radii_location %zu\n", offsetof(RenderPipe, shadowmap_light_radii_location));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_lightmap_program %zu\n", offsetof(RenderPipe, lightmap_program));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_lightmap_shadowmap_location %zu\n", offsetof(RenderPipe, lightmap_shadowmap_location));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_lightmap_light_positions_location %zu\n", offsetof(RenderPipe, lightmap_light_positions_location));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_lightmap_light_radii_location %zu\n", offsetof(RenderPipe, lightmap_light_radii_location));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_lightmap_light_colors_location %zu\n", offsetof(RenderPipe, lightmap_light_colors_location));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_bloom_program %zu\n", offsetof(RenderPipe, bloom_program));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_bloom_render_texture_location %zu\n", offsetof(RenderPipe, bloom_render_texture_location));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_bloom_direction_location %zu\n", offsetof(RenderPipe, bloom_direction_location));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_blend_program %zu\n", offsetof(RenderPipe, blend_program));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_blend_shadow_location %zu\n", offsetof(RenderPipe, blend_shadow_location));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_blend_bloom_location %zu\n", offsetof(RenderPipe, blend_bloom_location));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_blend_lightmap_info_location %zu\n", offsetof(RenderPipe, blend_lightmap_info_location));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_blend_lightmap_color_location %zu\n", offsetof(RenderPipe, blend_lightmap_color_location));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_blend_scene_location %zu\n", offsetof(RenderPipe, blend_scene_location));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_shadow_texture %zu\n", offsetof(RenderPipe, shadow_texture));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_fsaa_tex %zu\n", offsetof(RenderPipe, fsaa_tex));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_fsaa_fbo %zu\n", offsetof(RenderPipe, fsaa_fbo));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_programs %zu\n", offsetof(RenderPipe, programs));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe_program_count %zu\n", offsetof(RenderPipe, program_count));
	fprintf(output, "#define __RELOAD_OFFSET__RenderPipe___padding %zu\n", offsetof(RenderPipe, __padding));
	fprintf(output, "#define __RELOAD_SIZE__Reloader %zu\n", sizeof(Reloader));
	fprintf(output, "#define __RELOAD_SIZE__Buffer %zu\n", sizeof(Buffer));
	fprintf(output, "#define __RELOAD_OFFSET__Buffer_fill_method %zu\n", offsetof(Buffer, fill_method));
	fprintf(output, "#define __RELOAD_OFFSET__Buffer_buffer_name %zu\n", offsetof(Buffer, buffer_name));

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
