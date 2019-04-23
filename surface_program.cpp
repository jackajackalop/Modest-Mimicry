#include "surface_program.hpp"

#include "compile_program.hpp"
#include "gl_errors.hpp"

SurfaceProgram::SurfaceProgram() {
	program = compile_program(
		"#version 330\n"
		"void main() {\n"
        "   gl_Position = vec4(4*(gl_VertexID & 1) -1, 2 * (gl_VertexID &2) -1, 0.0, 1.0);"
		"}\n"
		,
		"#version 330\n"
        "uniform int width; \n"
        "uniform int height; \n"
		"uniform sampler2D bg_tex;\n"
		"uniform sampler2D model_tex;\n"
        "layout(location=0) out vec4 bg_out;\n"
        "layout(location=1) out vec4 model_out;\n"
		"void main() {\n"
        "   vec2 texCoord = gl_FragCoord.xy/vec2(width, height); \n"
		"	bg_out = texture(bg_tex, texCoord);\n"
		"	model_out = texture(model_tex, texCoord);\n"
		"}\n"
	);
	glUseProgram(program);

    width = glGetUniformLocation(program, "width");
    height = glGetUniformLocation(program, "height");

    glUniform1i(glGetUniformLocation(program, "bg_tex"), 0);
    glUniform1i(glGetUniformLocation(program, "model_tex"), 1);

	glUseProgram(0);

	GL_ERRORS();
}

Load< SurfaceProgram > surface_program(LoadTagInit, [](){
	return new SurfaceProgram();
});
