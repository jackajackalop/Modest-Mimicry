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
        "uniform int wins1; \n"
        "uniform int wins2; \n"
		"uniform sampler2D game_tex;\n"
		"uniform sampler2D spotlight_tex;\n"
		"uniform sampler2D award_tex;\n"
        "layout(location=0) out vec4 color_out;\n"
		"void main() {\n"
        "   vec2 texCoord1 = (vec2(2.0, 1.0)*gl_FragCoord.xy); \n"
        "   vec2 texCoord2 = (vec2(2.0, 1.0)*gl_FragCoord.xy)-vec2(1.1*width, 0.0*height); \n"
        "   texCoord1 = texCoord1/vec2(width, height); \n"
        "   texCoord2 = texCoord2/vec2(width, height); \n"
        "   vec4 spotlight_color = texture(spotlight_tex, texCoord1); \n"
        "   color_out = texelFetch(game_tex, ivec2(gl_FragCoord.xy), 0); \n"
        "   color_out*=0.7;\n"
        "   float factor = mix(0.0, 0.7, score1/100.0);"
        "   color_out*=1.0+spotlight_color*factor;\n"
        "   spotlight_color = texture(spotlight_tex, texCoord2); \n"
        "   color_out*=1.0+spotlight_color*(score2/200.0);\n"

        "   for(int i = 0; i<wins1; i++){\n"
        "       texCoord1 = (vec2(20.0, 8.0)*gl_FragCoord.xy-vec2(width*i, 7.0*height)); \n"
        "       texCoord1 = texCoord1/vec2(width, height); \n"
        "       vec4 award_color = texture(award_tex, texCoord1); \n"
        "       color_out = (award_color.r>0.5?award_color:color_out); \n"
        "   }\n"
        "   for(int j = 5; j>5-wins2; j--){\n"
        "       texCoord1 = (vec2(20.0, 8.0)*gl_FragCoord.xy-vec2(width*(14.0+j), 7.0*height)); \n"
        "       texCoord1 = texCoord1/vec2(width, height); \n"
        "       vec4 award_color = texture(award_tex, texCoord1); \n"
        "       color_out = (award_color.r>0.5?award_color:color_out); \n"
        "   }\n"

		"}\n"
	);
	glUseProgram(program);

    score1 = glGetUniformLocation(program, "score1");
    score2 = glGetUniformLocation(program, "score2");
    width = glGetUniformLocation(program, "width");
    height = glGetUniformLocation(program, "height");
    wins1 = glGetUniformLocation(program, "wins1");
    wins2 = glGetUniformLocation(program, "wins2");

    glUniform1i(glGetUniformLocation(program, "game_tex"), 0);
    glUniform1i(glGetUniformLocation(program, "spotlight_tex"), 1);
    glUniform1i(glGetUniformLocation(program, "award_tex"), 2);

	glUseProgram(0);

	GL_ERRORS();
}

Load< EffectProgram > effect_program(LoadTagInit, [](){
	return new EffectProgram();
});
