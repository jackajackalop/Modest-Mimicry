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
        "uniform int primitives[10];\n"
		"uniform sampler2D bg_tex;\n"
		"uniform sampler2D model_tex;\n"
		"uniform sampler2D menu0_tex;\n"
		"uniform sampler2D menu1_tex;\n"
		"uniform sampler2D menu2_tex;\n"
		"uniform sampler2D menu3_tex;\n"
		"uniform sampler2D menu4_tex;\n"
		"uniform sampler2D expand_tex;\n"
		"uniform sampler2D expand2_tex;\n"
		"uniform sampler2D sphere_tex;\n"
		"uniform sampler2D cube_tex;\n"
		"uniform sampler2D cone_tex;\n"
		"uniform sampler2D cylinder_tex;\n"
        "layout(location=0) out vec4 bg_out;\n"
        "layout(location=1) out vec4 model_out;\n"
        "layout(location=2) out vec4 ui_out;\n"
		"void main() {\n"
        "   vec2 texCoord = gl_FragCoord.xy/vec2(width, height); \n"
		"	bg_out = texture(bg_tex, texCoord);\n"
		"	model_out = texture(model_tex, texCoord);\n"
        "   texCoord = (vec2(2.0, 2.0)*gl_FragCoord.xy)-vec2(0.45*width, -0.1*height); \n"
        "   vec2 texCoordPlus = texCoord-vec2(0.0, 0.1*height); \n"
        "   texCoord = texCoord/vec2(width, height); \n"
        "   texCoordPlus = texCoordPlus/vec2(width, height); \n"
		"	vec4 menu0 = texture(menu0_tex, texCoord);\n"
		"	vec4 menu1 = texture(menu1_tex, texCoord);\n"
		"	vec4 menu2 = texture(menu2_tex, texCoord);\n"
		"   vec4 menu3 = texture(menu3_tex, texCoord);\n"
		"   vec4 menu4 = texture(menu4_tex, texCoord);\n"
		"   vec4 expand = texture(expand_tex, texCoordPlus);\n"
		"   vec4 expand2 = texture(expand2_tex, texCoordPlus);\n"

        "   ui_out = vec4(0.0, 0.0, 0.0, 0.0); \n"
        "   if(edit_mode == 0) \n"
		"       menu0 = texture(menu0_tex, texCoordPlus);\n"
        "   else if(edit_mode == 1) \n"
		"       menu1 = texture(menu1_tex, texCoordPlus);\n"
        "   if(edit_mode == 2) \n"
		"       menu2 = texture(menu2_tex, texCoordPlus);\n"
        "   if(edit_mode == 3){ \n"
        "       menu3 = texture(menu3_tex, texCoordPlus);\n"
        "       ui_out += expand; \n"
        "   } \n"
        "   if(edit_mode == 4){ \n"
        "       menu4 = texture(menu4_tex, texCoordPlus);\n"
        "       for(int i = 0; i<10; i++){ \n"
        "           texCoord = (vec2(18.0, 13.0)*gl_FragCoord.xy)-vec2((4.5+0.8*i)*width, 3.4*height); \n"
        "           texCoord = texCoord/vec2(width, height); \n"
        "           if(primitives[i] == 1) \n"
        "               ui_out+=texture(sphere_tex, texCoord); \n"
        "           else if(primitives[i] == 2) \n"
        "               ui_out+=texture(cube_tex, texCoord); \n"
        "           else if(primitives[i] == 3) \n"
        "               ui_out+=texture(cone_tex, texCoord); \n"
        "           else if(primitives[i] == 4) \n"
        "               ui_out+=texture(cylinder_tex, texCoord); \n"
        "       }\n"
        "       ui_out = (ui_out.r<0.3?expand2:ui_out); \n"
        "   } \n"

        "   ui_out += menu0+menu1+menu2+menu3+menu4; \n"
		"}\n"
	);
	glUseProgram(program);

    width = glGetUniformLocation(program, "width");
    height = glGetUniformLocation(program, "height");
    edit_mode = glGetUniformLocation(program, "edit_mode");
    primitives = glGetUniformLocation(program, "primitives");


    glUniform1i(glGetUniformLocation(program, "bg_tex"), 0);
    glUniform1i(glGetUniformLocation(program, "model_tex"), 1);
    glUniform1i(glGetUniformLocation(program, "menu0_tex"), 2);
    glUniform1i(glGetUniformLocation(program, "menu1_tex"), 3);
    glUniform1i(glGetUniformLocation(program, "menu2_tex"), 4);
    glUniform1i(glGetUniformLocation(program, "menu3_tex"), 5);
    glUniform1i(glGetUniformLocation(program, "menu4_tex"), 6);
    glUniform1i(glGetUniformLocation(program, "expand_tex"), 7);
    glUniform1i(glGetUniformLocation(program, "expand2_tex"), 8);
    glUniform1i(glGetUniformLocation(program, "sphere_tex"), 9);
    glUniform1i(glGetUniformLocation(program, "cube_tex"), 10);
    glUniform1i(glGetUniformLocation(program, "cone_tex"), 11);
    glUniform1i(glGetUniformLocation(program, "cylinder_tex"), 12);

	glUseProgram(0);

	GL_ERRORS();
}

Load< SurfaceProgram > surface_program(LoadTagInit, [](){
	return new SurfaceProgram();
});
