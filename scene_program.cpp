#include "scene_program.hpp"

#include "compile_program.hpp"
#include "gl_errors.hpp"

SceneProgram::SceneProgram() {
	program = compile_program(
		"#version 330\n"
		"uniform mat4 object_to_clip;\n"
		"uniform mat4x3 object_to_light;\n"
		"uniform mat3 normal_to_light;\n"
        "uniform float time;\n"
        "uniform vec3 viewPos;\n"
		"layout(location=0) in vec4 Position;\n"
        //note: layout keyword used to make sure that the location-0 attribute is always bound to something
        "in vec3 GeoNormal;\n"
		"in vec3 Normal;\n"
		"in vec4 Color;\n"
        "in vec4 ControlColor;"
		"in vec2 TexCoord;\n"
		"out vec3 position;\n"
        "out vec3 geoNormal;\n"
		"out vec3 shadingNormal;\n"
		"out vec4 color;\n"
        "out vec4 controlColor;\n"
		"out vec2 texCoord;\n"
		"void main() {\n"
		"	gl_Position = object_to_clip * Position;\n"
		"	position = object_to_light * Position;\n"
		"	shadingNormal = normal_to_light * Normal;\n"
        "   geoNormal = GeoNormal;\n"
		"	color = Color;\n"
        "   controlColor = ControlColor;\n"
		"	texCoord = TexCoord;\n"
        "   vec3 viewDir = normalize(viewPos-position);\n"
		"}\n"
		,
		"#version 330\n"
		"uniform vec3 sun_direction;\n"
		"uniform vec3 sun_color;\n"
		"uniform vec3 sky_direction;\n"
		"uniform vec3 sky_color;\n"
        "uniform float dA;\n"
        "uniform sampler2D tex;\n"
		"in vec3 position;\n"
        "in vec3 geoNormal;\n"
		"in vec3 shadingNormal;\n"
		"in vec4 color;\n"
        "in vec4 controlColor;\n"
		"in vec2 texCoord;\n"
        "layout(location=0) out vec4 color_out;\n"
        "layout(location=1) out vec4 control_out;\n"
		"void main() {\n"
		"	color_out = texture(tex, texCoord) * vec4(color.rgb, 1.0);\n"
		"}\n"
	);
    object_to_clip_mat4 = glGetUniformLocation(program, "object_to_clip");
	object_to_light_mat4x3 = glGetUniformLocation(program, "object_to_light");
	normal_to_light_mat3 = glGetUniformLocation(program, "normal_to_light");

	time = glGetUniformLocation(program, "time");
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
