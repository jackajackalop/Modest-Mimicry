#include "GL.hpp"
#include "Load.hpp"

//MainProgram draws a surface lit by two lights (a distant directional and a hemispherical light) where the surface color is drawn from texture unit 0:
struct MainProgram {
	//opengl program object:
	GLuint program = 0;

	//uniform locations:
	GLuint object_to_clip_mat4 = -1U;
	GLuint object_to_light_mat4x3 = -1U;
	GLuint normal_to_light_mat3 = -1U;
    GLuint clip_units_per_pixel = -1U;
    GLuint viewPos = -1U;

    GLuint primitives = -1U;
    GLuint positionsX = -1U;
    GLuint positionsY = -1U;
    GLuint positionsZ = -1U;
    GLuint rotationsX = -1U;
    GLuint rotationsY = -1U;
    GLuint rotationsZ = -1U;
    GLuint scales = -1U;
    GLuint selected = -1U;
    GLuint primitivesb = -1U;
    GLuint positionsXb = -1U;
    GLuint positionsYb = -1U;
    GLuint positionsZb = -1U;
    GLuint rotationsXb = -1U;
    GLuint rotationsYb = -1U;
    GLuint rotationsZb = -1U;
    GLuint scalesb = -1U;
    GLuint width = -1U;
    GLuint height = -1U;
	//textures:
	//texture0 - texture for the surface
	//texture1 - texture for spot light shadow map

	MainProgram();
};

extern Load< MainProgram > main_program;
