class Drawer {
	LineObjectPtr line_object;

	/// Creates a new drawer, using the given line_object.
	// @param line_object the line_object to use when drawing.
	Drawer(LineObjectPtr line_object) : line_object(line_object) {}

	/// Draws a NavigationMesh
	// @param nav_mesh A NavigationMesh object
	// void navmesh(nav_mesh) {
	// 	if (_Debug.disabled) return;

	// 	NavigationMesh.visualize_last_search(nav_mesh, line_object);
	// }

	/// Draws an array of positions as lines
	// @param array An array of positions
	// @param from_index optional starting index in the array
	void line_array(Vector3 *array, unsigned count, Vector4 color = DEFAULT_COLOR, unsigned from_index = 0, bool draw_coordinates = false) {
		if (_Debug.disabled) return;

		for (unsigned i = from_index + 1; i < count; i++) {
			_LineObject.add_line(line_object, &color, array + i - 1, array + i);
		}

		if (draw_coordinates) {
			for (unsigned i = from_index; i < count; i++) {
				Vector3 &position = array[i];
				_Debug.world_text_color(position, color, "%.1f,%.1f", position.x, position.y);
			}
		}
	}

	/// Draws a line.
	// @param from A Vector3 position where the line will begin
	// @param to A Vector3 position where the line will end
	// @param color Color of the line, defaults to debug::DEFAULT_COLOR
	// text, text_color: optional arguments, drawn at line middle
	void line(Vector3 from, Vector3 to, Vector4 color = DEFAULT_COLOR, char *text = 0, Vector4 text_color = DEFAULT_TEXT_COLOR) {
		if (_Debug.disabled) return;

		_LineObject.add_line(line_object, &color, &from, &to);
		if (text);
			_Debug.world_text_color(0.5f*(from+to), text_color, text);
	};

	/// Draws an arrow.
	// @param from A Vector3 position where the arrow will begin
	// @param to A Vector3 position where the arrow will end
	// @param color Color of the arrow, defaults to debug::DEFAULT_COLOR
	void arrow(Vector3 from, Vector3 to, Vector4 color = DEFAULT_COLOR) {
		if (_Debug.disabled) return;

		_LineObject.add_line(line_object, &color, &from, &to);

		float tip_length = 0.4f;
		float tip_width = 0.3f;
		Vector3 &tip = to;
		Vector3 axis = normalize(to - from);

		Vector3 x, y;
		make_axes(axis, x, y);

		line(from, tip, color);

		line(tip, tip - axis*tip_length - x*tip_width, color);
		line(tip, tip - axis*tip_length + x*tip_width, color);
		line(tip, tip - axis*tip_length - y*tip_width, color);
		line(tip, tip - axis*tip_length + y*tip_width, color);
	}

	/// Draws an arrow from one meter above the given units root, pointing in the give direction.
	// @param unit A unit from which the arrow will be drawn.
	// @param direction The direction of the arrow.
	// @param color Color of the arrow, defaults to debug::DEFAULT_COLOR
	void unit_arrow(UnitRef unit, Vector3 direction, Vector4 color = DEFAULT_COLOR) {
		Vector3 p = *_Unit.world_position(unit, 1) + vector3(0,0,1);
		arrow(p, p + direction, color);
	}

	/// Draws an arc with lines.
	// @param origin A vector3 position where the arc begins
	// @param direction A vector3 direction for the arc
	// @param angle The half-angle of the arc, defined as degrees and not radians
	// @param length The length of the arc
	// @param normal A vector3 towards the viewer, defaults to negative z
	// @param color Color of the line, defaults to debug::DEFAULT_COLOR
	void arc(Vector3 origin, Vector3 direction, float angle, float length = 1.0f, Vector3 normal = vector3(0,0,1), Vector4 color = DEFAULT_COLOR) {
		if (_Debug.disabled) return;

		Quaternion q_left = quaternion(normal, math::rad(angle * 0.5f));
		Vector3 left = origin + rotate(q_left, direction) * length;

		Quaternion q_right = quaternion(normal, math::rad(-angle * 0.5f));
		Vector3 right = origin + rotate(q_right, direction) * length;

		_LineObject.add_line(line_object, &color, &origin, &left);
		_LineObject.add_line(line_object, &color, &origin, &right);
		_LineObject.add_line(line_object, &color, &left, &right);
	}


	// Visualizes a quaternion
	void quat(Vector3 origin, Quaternion quat, float scale = 2.0f, Vector4 color = DEFAULT_COLOR) {
		if (_Debug.disabled) return;

		Vector3 axis;
		float theta;
		decompose(quat, &axis, &theta);
		Vector3 top = origin + axis*scale;
		line(origin, top, color);
	}

	/// Draws a circle.
	// @param center A Vector3 position of the circle
	// @param radius Radius of the circle
	// @param normal Normal direction of the circle
	// @param color Color of the line, defaults to debug::DEFAULT_COLOR
	void circle(Vector3 center, float radius, Vector3 normal = vector3(0,0,1), Vector4 color = DEFAULT_COLOR, unsigned segments = 20) {
		if (_Debug.disabled) return;
		_LineObject.add_circle(line_object, &color, &center, radius, &normal, segments);
	}

	/// Draws a sphere.
	// @param center A Vector3 position of the sphere
	// @param radius Radius of the sphere
	// @param color Color of the line, defaults to debug::DEFAULT_COLOR
	// @param segments Defaults to 20
	// @param parts Defaults to 2
	void sphere(Vector3 center, float radius, Vector4 color = DEFAULT_COLOR, unsigned segments = 20, unsigned parts = 2) {
		if (_Debug.disabled) return;
		_LineObject.add_sphere(line_object, &color, &center, radius, segments, parts);
	}

	/// Draws a representation of a pose.
	// @param center A Vector3 position of the sphere
	// @param radius Radius of the sphere, default is 0.2.
	// @param color Color of the line, defaults to debug::DEFAULT_COLOR
	// @param segments Defaults to 20
	// @param parts Defaults to 2
	void pose(Matrix4x4 pose, float radius = 0.2f, Vector4 color = DEFAULT_COLOR, float scale = 1.0f) {
		if (_Debug.disabled) return;

		Vector3 position = translation(pose);
		Quaternion rotation = quaternion(pose);
		sphere(position, radius, color);

		Vector3 right = rotate(rotation, vector3(1,0,0)) * scale;
		Vector3 forward = rotate(rotation, vector3(0,1,0)) * scale;
		Vector3 up = rotate(rotation, vector3(0,0,1)) * scale;

		line(position, position + right, vector4(255, 255, 0, 0));
		line(position, position + forward, vector4(255, 0, 255, 0));
		line(position, position + up, vector4(255, 0, 0, 255));
	}

	/// Draws a hemisphere.
	// @param center A Vector3 position of the hemisphere
	// @param radius Radius of the hemisphere
	// @param normal Direction of the hemisphere
	// @param color Color of the line, defaults to debug::DEFAULT_COLOR
	// @param segments Defaults to 20
	// @param parts Defaults to 2
	void hemisphere(Vector3 center, float radius, Vector3 normal, Vector4 color = DEFAULT_COLOR, unsigned segments = 20, unsigned parts = 2) {
		if (_Debug.disabled) return;
		_LineObject.add_half_sphere(line_object, &color, &center, radius, &normal, segments, parts);
	}

	/// Draws a box.
	// @param pose A Matrix4x4 specifying translation and orientation of the box
	// @param extents A Vector3 specifying the extents of the box
	// @param color Color of the box, defaults to debug::DEFAULT_COLOR
	void box(Matrix4x4 pose, Vector3 extents, Vector4 color = DEFAULT_COLOR) {
		if (_Debug.disabled) return;
		_LineObject.add_box(line_object, &color, &pose, &extents);
	}

	/// Draws an axis aligned box.
	// @param position A Vector3 specifying the translation the box
	// @param half_extents A Vector3 specifying the half extents of the box
	// @param color Color of the box, defaults to debug::DEFAULT_COLOR
	void aabb(Vector3 position, Vector3 half_extents, Vector4 color = DEFAULT_COLOR) {
		if (_Debug.disabled) return;

		Matrix4x4 pose = matrix4x4_identity();
		set_translation(pose, position);
		_LineObject.add_box(line_object, &color, &pose, &half_extents);
	}

	/// Draws a representation of a linear obb sweep.
	// @param from position where the sweep begins
	// @param to position where the sweep ends
	// @param extents the half extents of the obb
	// @param rotation the rotation of the obb
	// @param from_color the start start color used when lerping towards the } of the sweep
	// @param to_color the } color used when lerping towards the } of the sweep
	// @param segments the number of boxes to draw
	void linear_obb_sweep(Vector3 from, Vector3 to, Vector3 extents, Quaternion rotation, Vector4 from_color = DEFAULT_COLOR, Vector4 to_color = DEFAULT_COLOR, unsigned segments = 10) {
		float t = 0.0f;
		for (unsigned i=0; i < segments; i++) {
			t = (float)i / (float)segments;
			Vector3 forward = to - from;
			Matrix4x4 pose = matrix4x4(rotation);
			set_translation(pose, from + forward * t);

			Vector4 color = vector4(
				math::lerp(from_color.a, to_color.a, t),
				math::lerp(from_color.r, to_color.r, t),
				math::lerp(from_color.g, to_color.g, t),
				math::lerp(from_color.b, to_color.b, t));

			box(pose, extents, color);
		}
	}

	/// Draws a representation of a linear sphere sweep.
	// @param from position where the sweep begins
	// @param to position where the sweep ends
	// @param radius the radius of the sphere
	// @param from_color the start start color used when lerping towards the } of the sweep
	// @param to_color the } color used when lerping towards the } of the sweep
	// @param segments the number of spheres to draw
	void linear_sphere_sweep(Vector3 from, Vector3 to, float radius, Vector4 from_color = DEFAULT_COLOR, Vector4 to_color = DEFAULT_COLOR, unsigned segments = 20) {
		float t = 0.0f;
		for (unsigned i=0; i < segments; i++) {
			t = (float)i / (float)segments;
			Vector3 forward = to - from;
			Vector3 position = from + forward * t;

			Vector4 color = vector4(
				math::lerp(from_color.a, to_color.a, t),
				math::lerp(from_color.r, to_color.r, t),
				math::lerp(from_color.g, to_color.g, t),
				math::lerp(from_color.b, to_color.b, t));

			sphere(position, radius, color);
		}
	}

	/// Draws a capsule.
	// @from A Vector3 position where the capsule will begin
	// @to A Vector3 position where the capsule will end
	// @radius Radius of the capsule
	// @color Color of the capsule, defaults to debug::DEFAULT_COLOR
	// @segments Defaults to 20
	// @circles Defaults to 4
	// @bars Defaults to 10
	void capsule(Vector3 from, Vector3 to, float radius, Vector4 color = DEFAULT_COLOR, unsigned segments = 20, unsigned circles = 4, unsigned bars = 10) {
		if (_Debug.disabled) return;
		_LineObject.add_capsule(line_object, &color, &from, &to, radius, segments, circles, bars);
	}

	/// Draws a cone with the tip at 'from' and the base at 'to'.
	// @from A Vector3 position where the cone will begin
	// @to A Vector3 position where the cone will end
	// @radius Radius of the base
	// @color Color of the capsule, defaults to debug::DEFAULT_COLOR
	// @segments Defaults to 20
	// @bars Defaults to 10
	void cone(Vector3 from, Vector3 to, float radius, Vector4 color = DEFAULT_COLOR, unsigned segments = 20, unsigned bars = 10) {
		if (_Debug.disabled) return;
		_LineObject.add_cone(line_object, &color, &from, &to, radius, segments, bars);
	}

	/// Draws a circumferencing sphere on the unit
	// @color Color of the capsule, defaults to debug::DEFAULT_COLOR
	void unit_sphere(UnitRef unit, Vector4 color = DEFAULT_COLOR) {
		if (_Debug.disabled) return;
		OOBB oobb = _Unit.box(unit);

		Vector3 &m = oobb.max;
		float radius = (m.x > m.y) ? (m.x > m.z ? m.x : m.z) : (m.y > m.z ? m.y : m.z);

		Vector3 position = translation(oobb.tm);
		Vector3 forward = forward_axis(oobb.tm);
		arrow(position, position + forward * radius * 0.75, color);

		sphere(position, radius, color);
	}

	/// Draws a box around the unit.
	// @color Color of the capsule, defaults to debug::DEFAULT_COLOR
	void unit_box(UnitRef unit, Vector4 color = DEFAULT_COLOR) {
		if (_Debug.disabled) return;
		OOBB oobb = _Unit.box(unit);
		_LineObject.add_box(line_object, &color, &oobb.tm, &oobb.max);
	}

	/// Draws a units with a box.
	// @color Color of the capsule, defaults to debug::DEFAULT_COLOR
	void unit(UnitRef unit, Vector4 color = DEFAULT_COLOR) {
		unit_box(unit, color);
	}

	/// Draws a units skeleton
	void unit_skeleton(UnitRef unit, Vector4 color = DEFAULT_COLOR) {
		if (_Debug.disabled) return;

		BoneNamesWrapper bones = _Unit.bones(unit);
		for (unsigned i = 0; i < bones.num_bones; ++i) {
			IdString32 id32(bones.bone_names_list[i]);
			unsigned index = _Unit.node(unit, id32);
			unsigned parent = _Unit.find_scene_graph_parent(unit, index);
			if (parent != (unsigned)-1) {
				Vector3 from = *_Unit.world_position(unit, parent);
				Vector3 to = *_Unit.world_position(unit, index);
				float r = distance(from, to) * 0.1f;

				if (r > 0.1) { r = 0.1; }

				_LineObject.add_cone(line_object, &color, &to, &from, r, 20, 5);
			}
		}
	}

	/// Draws a units skeleton
	void unit_scene_graph(UnitRef unit, Vector4 color = DEFAULT_COLOR) {
		if (_Debug.disabled) return;

		unsigned num = _Unit.num_scene_graph_items(unit);
		for (unsigned i=1; i < num; i++) {
			unsigned parent = _Unit.find_scene_graph_parent(unit, i);
			if (parent != (unsigned)-1) {
				Vector3 from = *_Unit.world_position(unit, parent);
				Vector3 to = *_Unit.world_position(unit, i);
				float r = distance(from, to) * 0.1f;

				if (r > 0.1) { r = 0.1; }

				_LineObject.add_cone(line_object, &color, &to, &from, r, 20, 5);
				pose(*_Unit.world_pose(unit, i), 0.2f, color, 0.05f);
			}
		}
	}

	/// Draws an actor
	// @actor The actor to be drawn
	// @color Optional color, defaults to debug::DEFAULT_COLOR
	// @camera_pose Optional camera pose, it's used when drawing huge physics meshes and heightfields. Only the triangles close to the camera are drawn.
	void actor(ActorPtr actor, Vector4 color = DEFAULT_COLOR, Matrix4x4 *camera_pose = 0) {
		if (_Debug.disabled) return;
		_Actor.debug_draw(actor, line_object, &color, camera_pose);
	}

	/// Resets the objects drawn lines.
	void reset() {
		if (_Debug.disabled) return;
		_LineObject.reset(line_object);
	}
};
