//// Raycast
Entity &ray_entity = ...

i32 count = 2;
v3 ray_vertices[] = {
	{ 0.0f, 0.0f, 0.0f },
	{ 500.0f * cosf(time), 500.0f * sinf(time), 0.0f },
	{ 0.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f },

	{ 0.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f },
};

GLindex ray_indices[] = { 0, 1, 2, 3, 4, 5, 6, 7 };

Ray ray = make_ray(ray_vertices[0], ray_vertices[1]);

bool ray_hit = true;
while (ray_hit && count < (i32)ARRAY_COUNT(ray_vertices)) {
	RaycastResults rr = game.components.actor.raycast(ray);
	ray_hit = rr.id > 0;
	if (ray_hit) {
		ray_vertices[count-1] = rr.position;
		v3 projected = dot(ray.delta, rr.normal) * rr.normal;
		v3 reflection = ray.delta - projected * 2;
		ray_vertices[count] = rr.position + normalize(reflection) * 500;

		ray = make_ray(ray_vertices[count-1], ray_vertices[count]);

		count++;
	}
}

CALL(ray_entity, model, update_vertices, ray_indices, count, ray_vertices, count);


//// Roatate
CALL(game.entities[game.entity_count-2], model, rotate, 0.01f);
CALL(game.entities[game.entity_count-1], model, rotate, 0.02f);

//// Input
Entity &avatar = game.entities[0];
v2 direction = V2_f32(0, 0);
if (is_held(input, InputKey_D)) {
	direction.x = 1;
}
if (is_held(input, InputKey_A)) {
	direction.x = -1;
}
if (is_held(input, InputKey_W)) {
	direction.y = 1;
}
if (is_held(input, InputKey_S)) {
	direction.y = -1;
}
direction = normalize_or_zero(direction);
CALL(avatar, mover, add_acceleration, V3(direction.x * 2000, direction.y * 2000, 0));
