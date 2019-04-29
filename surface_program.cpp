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
        "uniform int edit_mode; \n"
		"uniform sampler2D bg_tex;\n"
		"uniform sampler2D model_tex;\n"
		"uniform sampler2D menu0_tex;\n"
		"uniform sampler2D menu1_tex;\n"
		"uniform sampler2D menu2_tex;\n"
		"uniform sampler2D menu3_tex;\n"
		"uniform sampler2D expand_tex;\n"
        "layout(location=0) out vec4 bg_out;\n"
        "layout(location=1) out vec4 model_out;\n"
        "layout(location=2) out vec4 ui_out;\n"
		"void main() {\n"
        "   vec2 texCoord = gl_FragCoord.xy/vec2(width, height); \n"
		"	bg_out = texture(bg_tex, texCoord);\n"
		"	model_out = texture(model_tex, texCoord);\n"
        "   texCoord = (vec2(2.0, 2.0)*gl_FragCoord.xy)-vec2(0.6*width, -0.1*height); \n"
        "   vec2 texCoordPlus = texCoord-vec2(0.0, 0.1*height); \n"
        "   texCoord = texCoord/vec2(width, height); \n"
        "   texCoordPlus = texCoordPlus/vec2(width, height); \n"
		"	vec4 menu0_color = texture(menu0_tex, texCoord);\n"
		"	vec4 menu1_color = texture(menu1_tex, texCoord);\n"
		"	vec4 menu2_color = texture(menu2_tex, texCoord);\n"
		"   vec4 menu3_color = texture(menu3_tex, texCoord);\n"
		"   vec4 expand_color = texture(expand_tex, texCoord);\n"

        "   ui_out = vec4(0.0, 0.0, 0.0, 0.0); \n"
        "   if(edit_mode == 0) \n"
		"       menu0_color = texture(menu0_tex, texCoordPlus);\n"
        "   else if(edit_mode == 1) \n"
		"       menu1_color = texture(menu1_tex, texCoordPlus);\n"
        "   if(edit_mode == 2) \n"
		"       menu2_color = texture(menu2_tex, texCoordPlus);\n"
        "   if(edit_mode == 3){ \n"
        "       menu3_color = texture(menu3_tex, texCoordPlus);\n"
        "       ui_out += expand_color; \n"
        "   } \n"

        "   ui_out += menu0_color+menu1_color+menu2_color+menu3_color; \n"
		"}\n"
	);
	glUseProgram(program);

    width = glGetUniformLocation(program, "width");
    height = glGetUniformLocation(program, "height");
    edit_mode = glGetUniformLocation(program, "edit_mode");

    glUniform1i(glGetUniformLocation(program, "bg_tex"), 0);
    glUniform1i(glGetUniformLocation(program, "model_tex"), 1);
    glUniform1i(glGetUniformLocation(program, "menu0_tex"), 2);
    glUniform1i(glGetUniformLocation(program, "menu1_tex"), 3);
    glUniform1i(glGetUniformLocation(program, "menu2_tex"), 4);
    glUniform1i(glGetUniformLocation(program, "menu3_tex"), 5);
    glUniform1i(glGetUniformLocation(program, "expand_tex"), 6);

	glUseProgram(0);

	GL_ERRORS();
}

Load< SurfaceProgram > surface_program(LoadTagInit, [](){
	return new SurfaceProgram();
});
