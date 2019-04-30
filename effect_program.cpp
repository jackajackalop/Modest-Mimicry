#include "effect_program.hpp"

#include "compile_program.hpp"
#include "gl_errors.hpp"

EffectProgram::EffectProgram() {
	program = compile_program(
		"#version 330\n"
		"void main() {\n"
        "   gl_Position = vec4(4*(gl_VertexID & 1) -1, 2 * (gl_VertexID &2) -1, 0.0, 1.0);"
		"}\n"
		,
		"#version 330\n"
        "uniform int score1; \n"
        "uniform int score2; \n"
        "uniform int width; \n"
        "uniform int height; \n"
		"uniform sampler2D game_tex;\n"
		"uniform sampler2D spotlight_tex;\n"
        "layout(location=0) out vec4 color_out;\n"
		"void main() {\n"
        "   vec2 texCoord1 = (vec2(2.0, 1.0)*gl_FragCoord.xy); \n"
        "   vec2 texCoord2 = (vec2(2.0, 1.0)*gl_FragCoord.xy)-vec2(1.1*width, 0.0*height); \n"
        "   texCoord1 = texCoord1/vec2(width, height); \n"
        "   texCoord2 = texCoord2/vec2(width, height); \n"
        "   vec4 spotlight_color = texture(spotlight_tex, texCoord1); \n"
        "   color_out = texelFetch(game_tex, ivec2(gl_FragCoord.xy), 0); \n"
        "   color_out*=0.7;\n"
        "   color_out+=spotlight_color*(score1/300.0);\n"
        "   spotlight_color = texture(spotlight_tex, texCoord2); \n"
        "   color_out+=spotlight_color*(score2/300.0);\n"
		"}\n"
	);
	glUseProgram(program);

    score1 = glGetUniformLocation(program, "score1");
    score2 = glGetUniformLocation(program, "score2");
    width = glGetUniformLocation(program, "width");
    height = glGetUniformLocation(program, "height");

    glUniform1i(glGetUniformLocation(program, "game_tex"), 0);
    glUniform1i(glGetUniformLocation(program, "spotlight_tex"), 1);

	glUseProgram(0);

	GL_ERRORS();
}

Load< EffectProgram > effect_program(LoadTagInit, [](){
	return new EffectProgram();
});
