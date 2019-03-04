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
		"out vec3 position;\n"
		"void main() {\n"
		"	position = object_to_light * Position;\n"
		"	gl_Position = Position;\n"
        "   vec3 viewDir = normalize(viewPos-position);\n"
		"}\n"
		,
		"#version 330\n"
        "uniform sampler2D tex;\n"
        "uniform int primitives[10];\n"
        "uniform int positionsX[10];\n"
        "uniform int positionsY[10];\n"
        "uniform int positionsZ[10];\n"
		"in vec3 position;\n"
        "layout(location=0) out vec4 color_out;\n"

        "float sdSphere( vec3 p, float s ){\n"
        "   return length(p)-s; \n"
        "} \n"

        "float sdBox( vec3 p, vec3 b ){ \n"
        "   vec3 d = abs(p) - b; \n"
        "   return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0)); \n"
        "}"

        "float sdTorus( vec3 p, vec2 t ){ \n"
        "   return length( vec2(length(p.xz)-t.x,p.y) )-t.y; \n"
        "} \n"

        // vertical
        "float sdCylinder( vec3 p, vec2 h ){ \n"
        "   vec2 d = abs(vec2(length(p.xz),p.y)) - h; \n"
        "   return min(max(d.x,d.y),0.0) + length(max(d,0.0)); \n"
        "} \n"

        // arbitrary orientation
        "float sdCylinder(vec3 p, vec3 a, vec3 b, float r){ \n"
        "   vec3 pa = p - a; \n"
        "   vec3 ba = b - a; \n"
        "   float baba = dot(ba,ba); \n"
        "   float paba = dot(pa,ba); \n"
        "   float x = length(pa*baba-ba*paba) - r*baba; \n"
        "   float y = abs(paba-baba*0.5)-baba*0.5; \n"
        "   float x2 = x*x; \n"
        "   float y2 = y*y*baba; \n"
        "   float d = (max(x,y)<0.0)?-min(x2,y2):(((x>0.0)?x2:0.0)+((y>0.0)?y2:0.0)); \n"
        "   return sign(d)*sqrt(abs(d))/baba; \n"
        "} \n"

        "float sdCone( in vec3 p, in vec3 c ){ \n"
        "   vec2 q = vec2( length(p.xz), p.y ); \n"
        "   float d1 = -q.y-c.z; \n"
        "   float d2 = max( dot(q,c.xy), q.y); \n"
        "   return length(max(vec2(d1,d2),0.0)) + min(max(d1,d2), 0.); \n"
        "} \n"
//------------------------------------------------------------------

        "float opS( float d1, float d2 ){ \n"
        "   return max(-d2,d1); \n"
        "} \n"

        "vec2 opU( vec2 d1, vec2 d2 ){ \n"
	    "   return (d1.x<d2.x) ? d1 : d2; \n"
        "} \n"

        "#define ZERO 0 \n"

//------------------------------------------------------------------

        "vec2 map( in vec3 pos ){ \n"
        "   vec2 res = vec2( 1e10, 0.0 ); \n"
	   /* "   res = opU(res,vec2(sdSphere(pos-vec3(0.0,0.25,0.0),0.25),46.9));\n"
        "   res = opU(res,vec2(sdTorus(pos-vec3(0.0,0.25,1.0),vec2(0.20,0.05)),25.0)); \n"
	    "   res = opU(res,vec2(sdCone(pos-vec3(0.0,0.50,-1.0),vec3(0.8,0.6,0.3) ),55.0)); \n"
        "   res = opU(res,vec2(sdBox(pos-vec3(0.0,0.25,2.0),vec3(0.25)),3.0));\n"
   	    "   res = opU(res,vec2(sdCylinder(pos-vec3(0.0,0.30,-2.0),vec2(0.1,0.2) ),8.0));\n"*/
        "   for(int i = 0; i<10; i++){ \n"
        "       if(primitives[i]>0){ \n"
        "           if(primitives[i]==1) \n"
	    "               res = opU(res,vec2(sdSphere(pos-vec3(0.0,0.25,0.0),0.25),46.9));\n"
        "           else if(primitives[i]==2) \n"
        "               res = opU(res,vec2(sdBox(pos-vec3(0.0,0.25,2.0),vec3(0.25)),3.0));\n"
        "           else if(primitives[i]==3) \n"
	    "               res = opU(res,vec2(sdCone(pos-vec3(0.0,0.50,-1.0),vec3(0.8,0.6,0.3) ),55.0)); \n"
        "           else if(primitives[i]==4) \n"
   	    "               res = opU(res,vec2(sdCylinder(pos-vec3(0.0,0.30,-2.0),vec2(0.1,0.2) ),8.0));\n"
        "       } \n"
        "   } \n"
        "   return res; \n"
        "} \n"

        // http://iquilezles.org/www/articles/boxfunctions/boxfunctions.htm
        "vec2 iBox( in vec3 ro, in vec3 rd, in vec3 rad ){ \n"
        "   vec3 m = 1.0/rd; \n"
        "   vec3 n = m*ro; \n"
        "   vec3 k = abs(m)*rad; \n"
        "   vec3 t1 = -n - k; \n"
        "   vec3 t2 = -n + k; \n"
	    "   return vec2( max( max( t1.x, t1.y ), t1.z ),"
	    "            min( min( t2.x, t2.y ), t2.z ) ); \n"
        "} \n"
        "const float maxHei = 0.8; \n"

        "vec2 castRay( in vec3 ro, in vec3 rd ){ \n"
        "   vec2 res = vec2(-1.0,-1.0); \n"
        "   float tmin = 1.0; \n"
        "   float tmax = 20.0; \n"
        // raymarch primitives
        "   vec2 tb = iBox( ro-vec3(0.0,0.4,0.0), rd, vec3(2.5,0.41,2.5) ); \n"
        "   if( tb.x<tb.y && tb.y>0.0 && tb.x<tmax){ \n"
        "       tmin = max(tb.x,tmin); \n"
        "       tmax = min(tb.y,tmax); \n"

        "       float t = tmin; \n"
        "       for( int i=0; i<70 && t<tmax; i++ ){ \n"
        "           vec2 h = map( ro+rd*t ); \n"
        "           if( abs(h.x)<(0.0001*t) ){ \n"
        "               res = vec2(t,h.y); \n"
        "               break; \n"
        "           }\n"
        "           t += h.x; \n"
        "       } \n"
        "   } \n"

        "   return res; \n"
        "} \n"


        // http://iquilezles.org/www/articles/rmshadows/rmshadows.htm
        "float calcSoftshadow(in vec3 ro,in vec3 rd,in float mint,in float tmax ){ \n"
    // bounding volume
        "   float tp = (maxHei-ro.y)/rd.y; if(tp>0.0) tmax = min(tmax,tp);\n"
        "   float res = 1.0; \n"
        "   float t = mint; \n"
        "   for( int i=ZERO; i<16; i++ ){ \n"
		"       float h = map( ro + rd*t ).x; \n"
        "       res = min( res, 8.0*h/t ); \n"
        "       t += clamp( h, 0.02, 0.10 ); \n"
        "       if( res<0.005 || t>tmax ) break; \n"
        "   } \n"
        "   return clamp( res, 0.0, 1.0 ); \n"
        "} \n"

        // http://iquilezles.org/www/articles/normalsSDF/normalsSDF.htm
        "vec3 calcNormal( in vec3 pos ){ \n"
        "#if 1 \n"
        "   vec2 e = vec2(1.0,-1.0)*0.5773*0.0005; \n"
        "   return normalize( e.xyy*map( pos + e.xyy ).x +  \n"
		"			  e.yyx*map( pos + e.yyx ).x +  \n"
		"			  e.yxy*map( pos + e.yxy ).x +  \n"
		"			  e.xxx*map( pos + e.xxx ).x ); \n"
        "#else \n"
        // inspired by klems - a way to prevent the compiler from inlining map() 4 times
        "   vec3 n = vec3(0.0); \n"
        "   for( int i=ZERO; i<4; i++ ){ \n"
        "       vec3 e = 0.5773*(2.0*vec3((((i+3)>>1)&1),((i>>1)&1),(i&1))-1.0); \n"
        "       n += e*map(pos+0.0005*e).x; \n"
        "   } \n"
        "   return normalize(n); \n"
        "#endif     \n"
        "} \n"

        "float calcAO( in vec3 pos, in vec3 nor ){ \n"
        "	float occ = 0.0; \n"
        "   float sca = 1.0; \n"
        "   for( int i=ZERO; i<5; i++ ){ \n"
        "       float hr = 0.01 + 0.12*float(i)/4.0; \n"
        "       vec3 aopos =  nor * hr + pos; \n"
        "       float dd = map( aopos ).x; \n"
        "       occ += -(dd-hr)*sca; \n"
        "       sca *= 0.95; \n"
        "   } \n"
        "   return clamp( 1.0 - 3.0*occ, 0.0, 1.0 ) * (0.5+0.5*nor.y); \n"
        "} \n"

        "vec3 render( in vec3 ro, in vec3 rd ){ \n"
        "   vec3 col = vec3(0.7, 0.9, 1.0) +rd.y*0.8; \n"
        "   vec2 res = castRay(ro,rd); \n"
        "   float t = res.x; \n"
	    "   float m = res.y; \n"
        "   if( m>-0.5 ){ \n"
        "       vec3 pos = ro + t*rd; \n"
        "       vec3 nor = (m<1.5) ? vec3(0.0,1.0,0.0) : calcNormal( pos ); \n"
        "       vec3 ref = reflect( rd, nor ); \n"
                // material
		"       col = 0.45 + 0.35*sin( vec3(0.05,0.08,0.10)*(m-1.0) ); \n"
        "       if( m<1.5 ) col = 0.3 + vec3(0.1); \n"
                // lighting
        "       float occ = calcAO( pos, nor ); \n"
		"       vec3  lig = normalize( vec3(-0.4, 0.7, -0.6) ); \n"
        "       vec3  hal = normalize( lig-rd ); \n"
		"       float amb = clamp( 0.5+0.5*nor.y, 0.0, 1.0 ); \n"
        "       float dif = clamp( dot( nor, lig ), 0.0, 1.0 ); \n"
        "       float bac = clamp( dot( nor, normalize(vec3(-lig.x,0.0,-lig.z))), 0.0, 1.0 )*clamp( 1.0-pos.y,0.0,1.0); \n"
        "       float dom = smoothstep( -0.2, 0.2, ref.y ); \n"
        "       float fre = pow( clamp(1.0+dot(nor,rd),0.0,1.0), 2.0 ); \n"
        "       dif *= calcSoftshadow( pos, lig, 0.02, 2.5 ); \n"
        "       dom *= calcSoftshadow( pos, ref, 0.02, 2.5 ); \n"

		"       float spe = pow( clamp( dot( nor, hal ), 0.0, 1.0 ),16.0)* \n"
                    "dif*(0.04+0.96*pow(clamp(1.0+dot(hal,rd),0.0,1.0),5.0));\n"

        "       vec3 lin = vec3(0.0); \n"
        "       lin += 1.40*dif*vec3(1.00,0.80,0.55); \n"
        "       lin += 0.20*amb*vec3(0.40,0.60,1.00)*occ; \n"
        "       lin += 0.40*dom*vec3(0.40,0.60,1.00)*occ; \n"
        "       lin += 0.50*bac*vec3(0.25,0.25,0.25)*occ; \n"
        "       lin += 0.25*fre*vec3(1.00,1.00,1.00)*occ; \n"
		"       col = col*lin; \n"
		"       col += 9.00*spe*vec3(1.00,0.90,0.70); \n"

    	"       col = mix(col,vec3(0.8,0.9,1.0),1.0-exp(-0.0002*t*t*t));\n"
        "   } \n"
	    "return vec3( clamp(col,0.0,1.0) ); \n"
        "} \n"

        "mat3 setCamera( in vec3 ro, in vec3 ta, float cr ){ \n"
	    "   vec3 cw = normalize(ta-ro); \n"
    	"   vec3 cp = vec3(sin(cr), cos(cr),0.0); \n"
	    "   vec3 cu = normalize( cross(cw,cp) ); \n"
    	"   vec3 cv =          ( cross(cu,cw) ); \n"
        "   return mat3( cu, cv, cw ); \n"
        "} \n"

		"void main() {\n"
        "   vec3 ro = vec3( 4.6, 1.0, 0.0 ); \n"
        "   vec3 ta = vec3( 0.0, 1.0, 2.0 ); \n"
        // camera-to-world transformation
        "   mat3 ca = setCamera( ro, ta, 0.0 ); \n"

        "   vec3 tot = vec3(0.0);\n"
        "   vec2 resolution = textureSize(tex, 0); \n"
        "   vec2 p = (-resolution.xy + 2.0*gl_FragCoord.xy)/resolution.y; \n"
         // ray direction
        "   vec3 rd = ca * normalize( vec3(p.xy,2.0) ); \n"
        // render
        "   vec3 col = render( ro, rd ); \n"
		// gamma
        "   col = pow( col, vec3(0.4545) );\n"
        "   tot += col; \n"
        "   color_out = vec4( tot, 1.0 ); \n"
		"}\n"
	);
    object_to_clip_mat4 = glGetUniformLocation(program, "object_to_clip");
	object_to_light_mat4x3 = glGetUniformLocation(program, "object_to_light");
	normal_to_light_mat3 = glGetUniformLocation(program, "normal_to_light");

	time = glGetUniformLocation(program, "time");
	viewPos = glGetUniformLocation(program, "viewPos");

	primitives = glGetUniformLocation(program, "primitives");
	positionsX = glGetUniformLocation(program, "positionsX");
	positionsY = glGetUniformLocation(program, "positionsY");
	positionsZ = glGetUniformLocation(program, "positionsZ");

	glUseProgram(program);

	GLuint tex_sampler2D = glGetUniformLocation(program, "tex");
	glUniform1i(tex_sampler2D, 0);

	glUseProgram(0);

	GL_ERRORS();
}

Load< SceneProgram > scene_program(LoadTagInit, [](){
	return new SceneProgram();
});
