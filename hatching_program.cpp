#include "hatching_program.hpp"

#include "compile_program.hpp"
#include "gl_errors.hpp"

HatchingProgram::HatchingProgram() {
	program = compile_program(
		"#version 330\n"
        "uniform float time;\n"
        "uniform vec3 viewPos;\n"
		"layout(location=0) in vec4 Position;\n"
        //note: layout keyword used to make sure that the location-0 attribute is always bound to something
		"out vec3 position;\n"
        "out float Time; \n"
		"void main() {\n"
		"	gl_Position = Position;\n"
        "   vec3 viewDir = normalize(viewPos-position);\n"
        "   Time = time; \n"
		"}\n"
		,
		"#version 330\n"
        "uniform sampler2D tex;\n"
        "uniform sampler2D bg_tex;\n"
        "layout(location=0) out vec4 color_out;\n"

		"void main() {\n"
        "   vec4 hatched = texelFetch(tex, ivec2(gl_FragCoord.xy), 0);\n"
        "   vec4 bg_color = texelFetch(bg_tex, ivec2(vec2(1.7, 2)*gl_FragCoord.xy), 0)\n;"
        "   color_out = (hatched!=vec4(0,0,0,1) ? hatched : bg_color); \n"
		"}\n"
	);
	time = glGetUniformLocation(program, "time");
	viewPos = glGetUniformLocation(program, "viewPos");

	glUseProgram(program);

	GLuint tex_sampler2D = glGetUniformLocation(program, "tex");
    glUniform1i(glGetUniformLocation(program, "bg_tex"), 1);
	glUniform1i(tex_sampler2D, 0);

	glUseProgram(0);

	GL_ERRORS();
}

Load< HatchingProgram > hatching_program(LoadTagInit, [](){
	return new HatchingProgram();
});
