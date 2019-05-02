#include "GL.hpp"
#include "Load.hpp"

//EffectProgram draws to effect_tex so that the r component is the heightmap,
//the g component is the xcomponent of the normal map, the b component is the
//y component of the normal map, and the a component is the tint of the paper
//assuming a light source at (1,1,1)
struct EffectProgram {
	//opengl program object:
	GLuint program = 0;
    GLuint score1 = -1U;
    GLuint score2 = -1U;
    GLuint width = -1U;
    GLuint height = -1U;
    GLuint wins1 = -1U;
    GLuint wins2 = -1U;

	//uniform locations:
	EffectProgram();
};

extern Load< EffectProgram > effect_program;
