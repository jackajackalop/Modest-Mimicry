#include "GL.hpp"
#include "Load.hpp"

struct HatchingProgram {
	//opengl program object:
	GLuint program = 0;

	//uniform locations:
/*	GLuint object_to_clip_mat4 = -1U;
	GLuint object_to_light_mat4x3 = -1U;
	GLuint normal_to_light_mat3 = -1U;*/
    GLuint time = -1U;
    GLuint clip_units_per_pixel = -1U;
    GLuint viewPos = -1U;
	//textures:
	//texture0 - texture for the surface
	//texture1 - texture for spot light shadow map

	HatchingProgram();
};

extern Load< HatchingProgram > hatching_program;
