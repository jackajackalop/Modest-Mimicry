//Based on Jaume Sanchez Elias's implementation of "Real-Time Hatching" paper
//https://github.com/spite/cross-hatching
//http://hhoppe.com/hatching.pdf

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
        "uniform sampler2D hatch0_tex;\n"
        "uniform sampler2D hatch1_tex;\n"
        "uniform sampler2D hatch2_tex;\n"
        "uniform sampler2D hatch3_tex;\n"
        "uniform sampler2D hatch4_tex;\n"
        "uniform sampler2D hatch5_tex;\n"
        "layout(location=0) out vec4 color_out;\n"

        "   vec2 og = gl_FragCoord.xy; \n"
        "   vec2 tile = textureSize(hatch0_tex, 0);"
        "   vec2 vUv = vec2(mod(og.x, tile.x), mod(og.y, tile.y)); \n"
        "   vec4 hatch0 = texelFetch(hatch0_tex, ivec2(vUv), 0);\n"
        "   vec4 hatch1 = texelFetch(hatch1_tex, ivec2(vUv), 0);\n"
        "   vec4 hatch2 = texelFetch(hatch2_tex, ivec2(vUv), 0);\n"
        "   vec4 hatch3 = texelFetch(hatch3_tex, ivec2(vUv), 0);\n"
        "   vec4 hatch4 = texelFetch(hatch4_tex, ivec2(vUv), 0);\n"
        "   vec4 hatch5 = texelFetch(hatch5_tex, ivec2(vUv), 0);\n"

        "vec4 shade(){ \n"
        "   float ambientWeight = 0.08; \n"
        "   float diffuseWeight = 1.0; \n"
        "   float rimWeight = 0.46; \n"
        "   float specularWeight = 1.0; \n"
        "   float shininess = 49.0; \n"
        "   vec4 og = texelFetch(tex, ivec2(gl_FragCoord.xy), 0);\n"
        "   float shading = (og.r+og.g+og.b)/3.0; \n"
        "   vec4 c = vec4( 0.0, 0.0, 0.0, 1.0); \n"
        "   float step = 1.0/6.0; \n"
        "   if( shading <= step && shading > 0.0) \n"
        "       c = mix(hatch5, hatch4, 6.0*shading); \n"
        "   if( shading>step && shading<=2.0*step) \n"
        "       c = mix(hatch4, hatch3, 6.0*(shading-step)); \n"
        "   if( shading>2.0*step && shading<=3.0*step) \n"
        "       c = mix(hatch3, hatch2, 6.0*(shading-2.0*step)); \n"
        "   if( shading>3.0*step && shading<=4.0*step) \n"
        "       c = mix(hatch2, hatch1, 6.0*(shading-3.0*step)); \n"
        "   if( shading>4.0*step && shading<=5.0*step) \n"
        "       c = mix(hatch1, hatch0, 6.0*(shading-4.0*step)); \n"
        "   if( shading>5.0*step ) \n"
        "       c = mix( hatch0, vec4(1.0), 6.0*(shading-5.0*step)); \n"
        "   return c; \n"
        "} \n"

		"void main() {\n"
        "   vec4 hatched = shade(); \n"
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
    glUniform1i(glGetUniformLocation(program, "hatch0_tex"), 2);
    glUniform1i(glGetUniformLocation(program, "hatch1_tex"), 3);
    glUniform1i(glGetUniformLocation(program, "hatch2_tex"), 4);
    glUniform1i(glGetUniformLocation(program, "hatch3_tex"), 5);
    glUniform1i(glGetUniformLocation(program, "hatch4_tex"), 6);
    glUniform1i(glGetUniformLocation(program, "hatch5_tex"), 7);

	glUseProgram(0);

	GL_ERRORS();
}

Load< HatchingProgram > hatching_program(LoadTagInit, [](){
	return new HatchingProgram();
});
