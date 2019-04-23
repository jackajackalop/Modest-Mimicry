#include "scene_program.hpp"

#include "compile_program.hpp"
#include "gl_errors.hpp"

SceneProgram::SceneProgram() {
	program = compile_program(
		"#version 330\n"
		"uniform mat4 object_to_clip;\n"
		"uniform mat4x3 object_to_light;\n"
		"uniform mat3 normal_to_light;\n"
        "uniform vec3 viewPos;\n"
		"layout(location=0) in vec4 Position;\n"
        //note: layout keyword used to make sure that the location-0 attribute is always bound to something
		"out vec3 position;\n"
		"void main() {\n"
		"	position = object_to_light * Position;\n"
		"	gl_Position = Position;\n"
        "   vec3 viewDir = normalize(viewPos-position);\n"
		"}\n"
		,
		"#version 330\n"
        "uniform sampler2D tex;\n"
        "layout(location=0) out vec4 color_out;\n"

		"void main() {\n"
        "   color_out = vec4(0, 0, 0, 0); \n"
		"}\n"
	);
    object_to_clip_mat4 = glGetUniformLocation(program, "object_to_clip");
	object_to_light_mat4x3 = glGetUniformLocation(program, "object_to_light");
	normal_to_light_mat3 = glGetUniformLocation(program, "normal_to_light");

	viewPos = glGetUniformLocation(program, "viewPos");

	glUseProgram(program);

	GLuint tex_sampler2D = glGetUniformLocation(program, "tex");

	glUniform1i(tex_sampler2D, 0);

	glUseProgram(0);

	GL_ERRORS();
}

Load< SceneProgram > scene_program(LoadTagInit, [](){
	return new SceneProgram();
});
