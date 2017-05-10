static void convert(i32 angle, i32 distance, v2_f64 &position, f64 &rotation) {
	if (angle == 0 && distance == 0) { // No movement
		return;
	}

	/* The angle that Roomba has turned through since the angle was
	last requested. The angle is expressed as the difference in
	the distance traveled by Roomba’s two wheels in millimeters,
	specifically the right wheel distance minus the left wheel
	distance, divided by two. This makes counter-clockwise angles
	positive and clockwise angles negative. This can be used to
	directly calculate the angle that Roomba has turned through
	since the last request. Since the distance between Roomba’s
	wheels is 258mm, the equations for calculating the angles in
	familiar units are:
	Angle in radians = (2 * difference) / 258
	Angle in degrees = (360 * difference) / (258 * Pi).
	If the value is not polled frequently enough, it will be capped at
	its minimum or maximum.*/

#if 0
	d = (r + l) / 2
	a = (r - l) / 2 => 2*a + l = r
	// substitute r in the equation for distance
	d = (2*a + l + l) / 2 => d = (a + l)
	// Move all known to left side
	d - a = l
	// Left is distance - angle
	// Plug this in the eqution for angle
	d = (r - (d - a)) / 2 => d = r/2 - d/2 + a/2 => 3d - a = r
	a = (r - (d - a)) / 2 => a = r/2 - d/2 + a/2 => a + d = r
	// right is angle + distance
#endif

	if (angle == 0) { // Haven't turned
		v2_f64 direction = V2_f64(cos(rotation), sin(rotation));
		position += direction * distance / 1000.0;
	} else if (distance == 0) { // Turned on the spot
		f64 angle_in_radians = (2.0 * angle) / 258.0;
		rotation += angle_in_radians;
	} else { // Moved while turning
		f64 angle_in_radians = (2.0 * angle) / 258.0; // The angle that the roomba has turned through
		f64 outer_arc_length;
		if (angle > 0) { // counter clockwise, or left, right wheel is outer
			outer_arc_length = angle + distance; // right_wheel_distance;
		} else {
			// Need to negate the length in order to use the same calculations for the right wheel.
			outer_arc_length = -(distance - angle); // left_wheel_distance;
		}

		// pido => pi * 2 * r * angle_in_radians/(2*pi) = outer_arc_length
		// pido => r * angle_in_radians = outer_arc_length
		f64 total_radius = outer_arc_length / angle_in_radians;
		f64 triangle_side_length = total_radius - 258.0/2.0;

		// Law of cosines - b^2 = a^2 + c^2 - 2ac * cos(beta);
		// Since a and c are equal in length (radius of the circle) this becomes
		// b^2 = 2*a^2 - 2*a^2 * cos(beta) or:
		// b^2 = 2*a^2(1 - cos(beta))
		f64 distance_sq = 2 * triangle_side_length * triangle_side_length * (1.0 - cos(angle_in_radians));
		f64 straight_distance = sqrt(distance_sq);

		f64 angle_of_direction = (M_PI - angle_in_radians) / 2;
		angle_of_direction = (M_PI / 2) - angle_of_direction;
		f64 rotation_for_direction = rotation + angle_of_direction;
		v2_f64 direction = V2_f64(cos(rotation_for_direction), sin(rotation_for_direction));

		position += direction * straight_distance / 1000.0;
		rotation += angle_in_radians;
	}
}
