#include "GL.hpp"
#include "Load.hpp"

//SceneProgram draws a surface lit by two lights (a distant directional and a hemispherical light) where the surface color is drawn from texture unit 0:
struct SceneProgram {
	//opengl program object:
	GLuint program = 0;

	//uniform locations:
	GLuint object_to_clip_mat4 = -1U;
	GLuint object_to_light_mat4x3 = -1U;
	GLuint normal_to_light_mat3 = -1U;
    GLuint time = -1U;
    GLuint clip_units_per_pixel = -1U;
    GLuint viewPos = -1U;

	//textures:
	//texture0 - texture for the surface
	//texture1 - texture for spot light shadow map

	SceneProgram();
};

extern Load< SceneProgram > scene_program;
