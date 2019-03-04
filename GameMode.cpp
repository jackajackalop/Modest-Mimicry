#include "GameMode.hpp"

#include "MenuMode.hpp"
#include "Load.hpp"
#include "MeshBuffer.hpp"
#include "Scene.hpp"
#include "gl_errors.hpp" //helper for dumpping OpenGL error messages
#include "check_fb.hpp" //helper for checking currently bound OpenGL framebuffer
#include "read_chunk.hpp" //helper for reading a vector of structures from a file
#include "data_path.hpp" //helper to get paths relative to executable
#include "compile_program.hpp" //helper to compile opengl shader programs
#include "draw_text.hpp" //helper to... um.. draw text
#include "load_save_png.hpp"
#include "scene_program.hpp"
#include "depth_program.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <map>
#include <cstddef>
#include <random>

Load< MeshBuffer > meshes(LoadTagDefault, [](){
	return new MeshBuffer(data_path("test.pgct"));
});

Load< GLuint > meshes_for_scene_program(LoadTagDefault, [](){
	return new GLuint(meshes->make_vao_for_program(scene_program->program));
});

Load< GLuint > meshes_for_depth_program(LoadTagDefault, [](){
	return new GLuint(meshes->make_vao_for_program(depth_program->program));
});

//used for fullscreen passes:
Load< GLuint > empty_vao(LoadTagDefault, [](){
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindVertexArray(0);
	return new GLuint(vao);
});

Load< GLuint > copy_program(LoadTagDefault, [](){
	GLuint program = compile_program(
		//this draws a triangle that covers the entire screen:
		"#version 330\n"
		"void main() {\n"
		"	gl_Position = vec4(4 * (gl_VertexID & 1) - 1,  2 * (gl_VertexID & 2) - 1, 0.0, 1.0);\n"
		"}\n"
		,
		//NOTE on reading screen texture:
		//texelFetch() gives direct pixel access with integer coordinates, but accessing out-of-bounds pixel is undefined:
		//	vec4 color = texelFetch(tex, ivec2(gl_FragCoord.xy), 0);
		//texture() requires using [0,1] coordinates, but handles out-of-bounds more gracefully (using wrap settings of underlying texture):
		//	vec4 color = texture(tex, gl_FragCoord.xy / textureSize(tex,0));

		"#version 330\n"
		"uniform sampler2D tex;\n"
		"out vec4 fragColor;\n"
		"void main() {\n"
		"	fragColor = texelFetch(tex, ivec2(gl_FragCoord.xy), 0);\n"
		"}\n"
	);

	glUseProgram(program);

	glUniform1i(glGetUniformLocation(program, "tex"), 0);

	glUseProgram(0);

	return new GLuint(program);
});


GLuint load_texture(std::string const &filename) {
	glm::uvec2 size;
	std::vector< glm::u8vec4 > data;
	load_png(filename, &size, &data, LowerLeftOrigin);

	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	GL_ERRORS();

	return tex;
}

Load< GLuint > wood_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/grid.png")));
});

Load< GLuint > marble_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/marble.png")));
});

Load< GLuint > paper_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/marble.png")));
});

Load< GLuint > normal_map_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/wood.png")));
});

Load< GLuint > white_tex(LoadTagDefault, [](){
	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glm::u8vec4 white(0xff, 0xff, 0xff, 0xff);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, glm::value_ptr(white));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	return new GLuint(tex);
});

Scene::Transform *camera_parent_transform = nullptr;
Scene::Camera *camera = nullptr;
//Scene::Transform *spot_parent_transform = nullptr;
//Scene::Lamp *spot = nullptr;

float elapsed_time = 0.0f;
int edit_mode = 0; //0 for translation, 1 for rotation, 2 for scaling

Load< Scene > scene(LoadTagDefault, [](){
	Scene *ret = new Scene;

	//pre-build some program info (material) blocks to assign to each object:
	Scene::Object::ProgramInfo scene_program_info;
	scene_program_info.program = scene_program->program;
	scene_program_info.vao = *meshes_for_scene_program;
	scene_program_info.mvp_mat4  = scene_program->object_to_clip_mat4;
	scene_program_info.mv_mat4x3 = scene_program->object_to_light_mat4x3;
	scene_program_info.itmv_mat3 = scene_program->normal_to_light_mat3;

	Scene::Object::ProgramInfo depth_program_info;
	depth_program_info.program = depth_program->program;
	depth_program_info.vao = *meshes_for_depth_program;
	depth_program_info.mvp_mat4  = depth_program->object_to_clip_mat4;

	//load transform hierarchy:
	ret->load(data_path("test.scene"), [&](Scene &s, Scene::Transform *t, std::string const &m){
		Scene::Object *obj = s.new_object(t);

		obj->programs[Scene::Object::ProgramTypeDefault] = scene_program_info;
		if (t->name == "Platform") {
			obj->programs[Scene::Object::ProgramTypeDefault].textures[0] = *wood_tex;
		} else if (t->name == "Pedestal") {
			obj->programs[Scene::Object::ProgramTypeDefault].textures[0] = *marble_tex;
		} else {
			obj->programs[Scene::Object::ProgramTypeDefault].textures[0] = *white_tex;
		}

		obj->programs[Scene::Object::ProgramTypeShadow] = depth_program_info;

		MeshBuffer::Mesh const &mesh = meshes->lookup(m);
		obj->programs[Scene::Object::ProgramTypeDefault].start = mesh.start;
		obj->programs[Scene::Object::ProgramTypeDefault].count = mesh.count;

		obj->programs[Scene::Object::ProgramTypeShadow].start = mesh.start;
		obj->programs[Scene::Object::ProgramTypeShadow].count = mesh.count;
	});

	//look up camera parent transform:
	for (Scene::Transform *t = ret->first_transform; t != nullptr; t = t->alloc_next) {
		if (t->name == "CameraParent") {
			if (camera_parent_transform) throw std::runtime_error("Multiple 'CameraParent' transforms in scene.");
			camera_parent_transform = t;
		}
/*		if (t->name == "SpotParent") {
			if (spot_parent_transform) throw std::runtime_error("Multiple 'SpotParent' transforms in scene.");
			spot_parent_transform = t;
		}
*/
	}
	if (!camera_parent_transform) throw std::runtime_error("No 'CameraParent' transform in scene.");
//	if (!spot_parent_transform) throw std::runtime_error("No 'SpotParent' transform in scene.");

	//look up the camera:
	for (Scene::Camera *c = ret->first_camera; c != nullptr; c = c->alloc_next) {
		if (c->transform->name == "Camera") {
			if (camera) throw std::runtime_error("Multiple 'Camera' objects in scene.");
			camera = c;
		}
	}
	if (!camera) throw std::runtime_error("No 'Camera' camera in scene.");
/*
	//look up the spotlight:
	for (Scene::Lamp *l = ret->first_lamp; l != nullptr; l = l->alloc_next) {
		if (l->transform->name == "Spot") {
			if (spot) throw std::runtime_error("Multiple 'Spot' objects in scene.");
			if (l->type != Scene::Lamp::Spot) throw std::runtime_error("Lamp 'Spot' is not a spotlight.");
			spot = l;
		}
	}
	if (!spot) throw std::runtime_error("No 'Spot' spotlight in scene.");
*/
	return ret;
});

GameMode::GameMode() {
}

GameMode::~GameMode() {
}

void GameMode::add_primitive(int primitive_type){
   Primitive new_prim;
   new_prim.shape = primitive_type;
   primitives.emplace_back(new_prim);
   std::cout<<"added"<<std::endl;
}

bool GameMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	//ignore any keys that are the result of automatic key repeat:
	if (evt.type == SDL_KEYDOWN && evt.key.repeat) {
		return false;
	}

    if (evt.type == SDL_KEYDOWN) {
        if (evt.key.keysym.scancode == SDL_SCANCODE_T){
            edit_mode = 0; //translation
        }else if(evt.key.keysym.scancode == SDL_SCANCODE_R){
            edit_mode = 1;
        }else if(evt.key.keysym.scancode == SDL_SCANCODE_Y){
            edit_mode = 2;
        }

        if (evt.key.keysym.scancode == SDL_SCANCODE_1) {
            add_primitive(1); //sphere
        }else if (evt.key.keysym.scancode == SDL_SCANCODE_2) {
            add_primitive(2); //sphere
        }else if (evt.key.keysym.scancode == SDL_SCANCODE_3) {
            add_primitive(3); //sphere
        }else if (evt.key.keysym.scancode == SDL_SCANCODE_4) {
            add_primitive(4); //sphere
        }

        if(evt.key.keysym.scancode == SDL_SCANCODE_A){
            if(edit_mode==0) primitives[0].position.z+=0.1;
            else if(edit_mode==2) primitives[0].scale -= 0.1;
        }else if(evt.key.keysym.scancode == SDL_SCANCODE_D){
            if(edit_mode==0) primitives[0].position.z-=0.1;
            else if(edit_mode==2) primitives[0].scale += 0.1;
        }else if(evt.key.keysym.scancode == SDL_SCANCODE_W){
            if(edit_mode==0) primitives[0].position.y+=0.1;
            else if(edit_mode==2) primitives[0].scale += 0.1;
        }else if(evt.key.keysym.scancode == SDL_SCANCODE_S){
            if(edit_mode==0) primitives[0].position.y-=0.1;
            else if(edit_mode==2) primitives[0].scale -= 0.1;
        }else if(evt.key.keysym.scancode == SDL_SCANCODE_Q){
            if(edit_mode==0) primitives[0].position.x-=0.1;
            else if(edit_mode==2) primitives[0].scale -= 0.1;
        }else if(evt.key.keysym.scancode == SDL_SCANCODE_E){
            if(edit_mode==0) primitives[0].position.x+=0.1;
            else if(edit_mode==2) primitives[0].scale += 0.1;
        }

    }

    if (evt.type == SDL_MOUSEBUTTONDOWN) {

	}
	return false;
}

void GameMode::update(float elapsed) {
	camera_parent_transform->rotation = glm::angleAxis(camera_spin, glm::vec3(0.0f, 0.0f, 1.0f));
//	spot_parent_transform->rotation = glm::angleAxis(spot_spin, glm::vec3(0.0f, 0.0f, 1.0f));
    elapsed_time+=elapsed;
}

//GameMode will render to some offscreen framebuffer(s).
//This code allocates and resizes them as needed:
struct Textures {
	glm::uvec2 size = glm::uvec2(0,0); //remember the size of the framebuffer

	GLuint color_tex = 0;
	GLuint depth_tex = 0;
	void allocate(glm::uvec2 const &new_size) {
    //allocate full-screen framebuffer:

		if (size != new_size) {
			size = new_size;

            auto alloc_tex = [this](GLuint *tex, GLint internalformat, GLint format){
                if (*tex == 0) glGenTextures(1, tex);
	    		glBindTexture(GL_TEXTURE_2D, *tex);
		    	glTexImage2D(GL_TEXTURE_2D, 0, internalformat, size.x,
                        size.y, 0, format, GL_UNSIGNED_BYTE, NULL);
			    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	    		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			    glBindTexture(GL_TEXTURE_2D, 0);
            };

            alloc_tex(&color_tex, GL_RGBA8, GL_RGBA);
            alloc_tex(&depth_tex, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT);
			GL_ERRORS();
		}

	}
} textures;

void GameMode::draw_scene(GLuint* color_tex_, GLuint* depth_tex_){
    assert(color_tex_);
    assert(depth_tex_);
    auto &color_tex = *color_tex_;
    auto &depth_tex = *depth_tex_;

    static GLuint fb = 0;
    if(fb==0) glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                            color_tex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                            depth_tex, 0);
    GLenum bufs[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, bufs);
    check_fb();


	//Draw scene to off-screen framebuffer:
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glViewport(0,0, textures.size.x, textures.size.y);

	camera->aspect = textures.size.x / float(textures.size.y);

    GLfloat black[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    glClearBufferfv(GL_COLOR, 0, black);
    glClearBufferfv(GL_COLOR, 1, black);
	glClear(GL_DEPTH_BUFFER_BIT);

	//set up basic OpenGL state:
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(scene_program->program);

    glUniform1f(scene_program->time, elapsed_time);
    glUniform3fv(scene_program->viewPos, 1,
            glm::value_ptr(camera->transform->make_local_to_world()));

    set_prim_uniforms();
    scene->draw(camera);

}

void GameMode::set_prim_uniforms(){
    int n = primitives.size();
    int prim10[10];
    float posX10[10];
    float posY10[10];
    float posZ10[10];
    float scale10[10];
    for(int i = 0; i<10; i++){
        if(i<n){
            prim10[i] = primitives[i].shape;
            posX10[i] = primitives[i].position.x;
            posY10[i] = primitives[i].position.y;
            posZ10[i] = primitives[i].position.z;
            scale10[i] = primitives[i].scale;
        }else{
            prim10[i] = 0;
            posX10[i] = 0;
            posY10[i] = 0;
            posZ10[i] = 0;
            scale10[i] = 1;
        }
    }
    glUniform1iv(scene_program->primitives, 10, prim10);
    glUniform1fv(scene_program->positionsX, 10, posX10);
    glUniform1fv(scene_program->positionsY, 10, posY10);
    glUniform1fv(scene_program->positionsZ, 10, posZ10);
    glUniform1fv(scene_program->scales, 10, scale10);
}

void GameMode::draw(glm::uvec2 const &drawable_size) {
	textures.allocate(drawable_size);

    draw_scene(&textures.color_tex, &textures.depth_tex);
    glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GL_ERRORS();

	//Copy scene from color buffer to screen, performing post-processing effects:
	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures.color_tex);

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
	glUseProgram(*copy_program);
	glBindVertexArray(*empty_vao);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glUseProgram(0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
