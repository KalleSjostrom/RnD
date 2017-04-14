
namespace component_glue {
	void update(Entity *entites, int count, float dt) {
		for (int i = 0; i < count; ++i) {
			Entity &entity = entites[i];
			switch (entity.type) {
				case EntityType_Avatar: {
					v3 *vertices = CALL(entity, animation, get_skeleton_vertices);
					CALL(entity, model, update_vertices, vertices);
				} break;
				case EntityType_BlockAvatar: {
					v3 position = CALL(entity, mover, get_position);
					CALL(entity, model, set_position, position);
					// CALL(entity, actor, set_position, position);
					m4 &pose = GET(entity, model, pose);
					CALL(entity, actor, set_pose, pose);
				} break;
				case EntityType_Block: {
					m4 &pose = GET(entity, model, pose);
					CALL(entity, actor, set_pose, pose);
				} break;
				default: {};
			}
		}
	}
}