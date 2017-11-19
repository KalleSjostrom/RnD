namespace voxel_visualization {
static const char *vertex = GLSL(
	layout(location = 0) in vec3 position;
	out vec2 textureCoordinateFrag;

	// Scales and bias a given vector (i.e. from [-1, 1] to [0, 1]).
	vec2 scaleAndBias(vec2 p) { return 0.5f * p + vec2(0.5f); }

	void main(){
		textureCoordinateFrag = scaleAndBias(position.xy);
		gl_Position = vec4(position, 1);
	}
);

static const char *fragment = GLSL(
	#define INV_STEP_LENGTH (1.0f/STEP_LENGTH)
	#define STEP_LENGTH 0.005f

	uniform sampler2D textureBack; // Unit cube back FBO.
	uniform sampler2D textureFront; // Unit cube front FBO.
	uniform sampler3D texture3D; // Texture in which voxelization is stored.
	uniform vec3 cameraPosition; // World camera position.
	uniform int state = 0; // Decides mipmap sample level.

	in vec2 textureCoordinateFrag;
	out vec4 color;

	// Scales and bias a given vector (i.e. from [-1, 1] to [0, 1]).
	vec3 scaleAndBias(vec3 p) { return 0.5f * p + vec3(0.5f); }

	// Returns true if p is inside the unity cube (+ e) centered on (0, 0, 0).
	bool isInsideCube(vec3 p, float e) { return abs(p.x) < 1 + e && abs(p.y) < 1 + e && abs(p.z) < 1 + e; }

	void main() {
		const float mipmapLevel = state;

		// Initialize ray.
		const vec3 origin = isInsideCube(cameraPosition, 0.2f) ?
			cameraPosition : texture(textureFront, textureCoordinateFrag).xyz;
		vec3 direction = texture(textureBack, textureCoordinateFrag).xyz - origin;
		const uint numberOfSteps = uint(INV_STEP_LENGTH * length(direction));
		direction = normalize(direction);

		// Trace.
		color = vec4(0.0f);
		for(uint step = 0; step < numberOfSteps && color.a < 0.99f; ++step) {
			const vec3 currentPoint = origin + STEP_LENGTH * step * direction;
			vec3 coordinate = scaleAndBias(currentPoint);
			vec4 currentSample = textureLod(texture3D, scaleAndBias(currentPoint), mipmapLevel);
			color += (1.0f - color.a) * currentSample;
		}
		color.rgb = pow(color.rgb, vec3(1.0 / 2.2));
	}
);
}

namespace voxel_visualization {
static const char *vertex = GLSL(
	layout(location = 0) in vec3 position;

	uniform mat4 M;
	uniform mat4 V;
	uniform mat4 P;

	out vec3 worldPosition;

	void main(){
		worldPosition = vec3(M * vec4(position, 1));
		gl_Position = P * V * vec4(worldPosition, 1);
	}
);

static const char *fragment = GLSL(
	in vec3 worldPosition;

	out vec4 color;

	void main(){ color.rgb = worldPosition; }
);