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
Load< GLuint > bg_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/bg.png")));
});
Load< GLuint > hatch0_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/hatch_0.png")));
});
Load< GLuint > hatch1_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/hatch_1.png")));
});
Load< GLuint > hatch2_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/hatch_2.png")));
});
Load< GLuint > hatch3_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/hatch_3.png")));
});
Load< GLuint > hatch4_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/hatch_4.png")));
});
Load< GLuint > hatch5_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/hatch_5.png")));
});
Load< GLuint > level0_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/level0.png")));
});
Load< GLuint > level1_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/level1.png")));
});
Load< GLuint > level2_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/level2.png")));
});
Load< GLuint > level3_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/level3.png")));
});
Load< GLuint > level4_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/level4.png")));
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

int width =0, height =0;
float time_left = 2.0;
int edit_mode = 0; //0 for translation, 1 for rotation, 2 for scaling
bool updated = false;
bool paused = false;
Game state1, state2;

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
			obj->programs[Scene::Object::ProgramTypeDefault].textures[0] = *marble_tex;
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

GameMode::GameMode(Client &client_) : client(client_) {
    client.connection.send_raw("h", 1); //send a 'hello' to the server
}

GameMode::~GameMode() {
}

void GameMode::add_primitive(int primitive_type){
    if(state1.prim_num>=10) return;
    Primitive new_prim;
    new_prim.shape = primitive_type;
    state1.primitives[state1.prim_num] = new_prim;
    state1.prim_num++;
    std::cout<<"added"<<std::endl;
}

bool GameMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	//ignore any keys that are the result of automatic key repeat:

    if (evt.type == SDL_KEYDOWN) {
        updated = true;

        if (evt.key.keysym.scancode == SDL_SCANCODE_T){
            edit_mode = 0; //translation
        }else if(evt.key.keysym.scancode == SDL_SCANCODE_R){
            edit_mode = 1;
        }else if(evt.key.keysym.scancode == SDL_SCANCODE_Y){
            edit_mode = 2;
        }

        if (evt.key.keysym.scancode == SDL_SCANCODE_LEFT){
            selected = selected-1;
            if(selected<0) selected = state1.prim_num-1;
        }else if(evt.key.keysym.scancode == SDL_SCANCODE_RIGHT){
            selected = selected+1;
            if(selected>=state1.prim_num) selected = 0;
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
            if(edit_mode==0) state1.primitives[selected].position.z+=0.1;
            else if(edit_mode==1)state1.primitives[selected].rotation.y -= 0.1;
            else if(edit_mode==2) state1.primitives[selected].scale -= 0.1;
        }else if(evt.key.keysym.scancode == SDL_SCANCODE_D){
            if(edit_mode==0) state1.primitives[selected].position.z-=0.1;
            else if(edit_mode==1) state1.primitives[selected].rotation.y += 0.1;
            else if(edit_mode==2) state1.primitives[selected].scale += 0.1;
        }else if(evt.key.keysym.scancode == SDL_SCANCODE_W){
            if(edit_mode==0) state1.primitives[selected].position.y+=0.1;
            else if(edit_mode==1)state1.primitives[selected].rotation.z += 0.1;
            else if(edit_mode==2) state1.primitives[selected].scale += 0.1;
        }else if(evt.key.keysym.scancode == SDL_SCANCODE_S){
            if(edit_mode==0) state1.primitives[selected].position.y-=0.1;
            else if(edit_mode==1)state1.primitives[selected].rotation.z -= 0.1;
            else if(edit_mode==2) state1.primitives[selected].scale -= 0.1;
        }else if(evt.key.keysym.scancode == SDL_SCANCODE_Q){
            if(edit_mode==0) state1.primitives[selected].position.x-=0.1;
            else if(edit_mode==1)state1.primitives[selected].rotation.x += 0.1;
            else if(edit_mode==2) state1.primitives[selected].scale -= 0.1;
        }else if(evt.key.keysym.scancode == SDL_SCANCODE_E){
            if(edit_mode==0) state1.primitives[selected].position.x+=0.1;
            else if(edit_mode==1)state1.primitives[selected].rotation.x -= 0.1;
            else if(edit_mode==2) state1.primitives[selected].scale += 0.1;
        }

    }

    if (evt.type == SDL_MOUSEBUTTONDOWN) {

	}
	return false;
}

void GameMode::update(float elapsed) {
    if(paused) return;
	camera_parent_transform->rotation = glm::angleAxis(camera_spin, glm::vec3(0.0f, 0.0f, 1.0f));
//	spot_parent_transform->rotation = glm::angleAxis(spot_spin, glm::vec3(0.0f, 0.0f, 1.0f));
    time_left-=elapsed;
    if(time_left<=0.0){
        if(score1>score2){
            if(playerNum==0) show_win();
            else show_lose();
        }else{
            if(playerNum==0) show_lose();
            else show_win();
        }
    }

    if (client.connection) {
        //send game state to server:
        client.connection.send_raw("a", 1);
        client.connection.send_raw(&state1.primitives, 10*sizeof(Primitive));
    }
    client.poll([&](Connection *c, Connection::Event event){
        if (event == Connection::OnOpen) {
        //probably won't get this.
        } else if (event == Connection::OnClose) {
            std::cerr << "Lost connection to server." << std::endl;
        } else { assert(event == Connection::OnRecv);}

        if(c->recv_buffer[0] == 'p'){
            if(c->recv_buffer.size() < 2){
                return;
            }else{
                memcpy(&playerNum, c->recv_buffer.data()+1, 1);
                c->recv_buffer.erase(c->recv_buffer.begin(),
                        c->recv_buffer.begin() + 2);
                std::cout<<"You're Player "<<playerNum<<std::endl;
            }
        }else if (c->recv_buffer[0] == 'a') {
            if (c->recv_buffer.size() < 1 + sizeof(Game)) {
                return; //wait for more data
            } else {
                    memcpy(&state2.primitives, c->recv_buffer.data()+1,
                            10*sizeof(Primitive));
                    c->recv_buffer.erase(c->recv_buffer.begin(),
                        c->recv_buffer.begin() + 1 + 10*sizeof(Primitive));
            }
        }
    });

}

//GameMode will render to some offscreen framebuffer(s).
//This code allocates and resizes them as needed:
struct Textures {
	glm::uvec2 size = glm::uvec2(0,0); //remember the size of the framebuffer

	GLuint color_tex = 0;
    GLuint hatched_tex = 0;
	GLuint depth_tex = 0;
    GLuint player_tex = 0;
    GLuint model_tex = 0;
    GLuint text_tex = 0;
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

            alloc_tex(&color_tex, GL_RGBA, GL_RGBA);
            alloc_tex(&hatched_tex, GL_RGBA, GL_RGBA);
            alloc_tex(&depth_tex, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT);
            alloc_tex(&player_tex, GL_RGBA, GL_RGBA);
            alloc_tex(&model_tex, GL_RGBA, GL_RGBA);
            alloc_tex(&text_tex, GL_RGBA, GL_RGBA);
			GL_ERRORS();
		}

	}
} textures;

void GameMode::draw_scene(GLuint text_tex, GLuint* color_tex_,
        GLuint* depth_tex_, GLuint* player_tex_, GLuint* model_tex_){
    assert(color_tex_);
    assert(depth_tex_);
    assert(player_tex_);
    assert(model_tex_);
    auto &color_tex = *color_tex_;
    auto &depth_tex = *depth_tex_;
    auto &player_tex = *player_tex_;
    auto &model_tex = *model_tex_;

    static GLuint fb = 0;
    if(fb==0) glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                            color_tex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                            player_tex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
                            model_tex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                            depth_tex, 0);
    GLenum bufs[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, bufs);
    check_fb();


	//Draw scene to off-screen framebuffer:
	glViewport(0,0, textures.size.x, textures.size.y);
    width = textures.size.x;
    height = textures.size.y;

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

    glUniform1f(scene_program->time, time_left);
    glUniform3fv(scene_program->viewPos, 1,
            glm::value_ptr(camera->transform->make_local_to_world()));

    set_prim_uniforms();
    GLuint level_tex = 0;
    if(level==0) level_tex= *level0_tex;
    else if(level==1) level_tex = *level1_tex;
    else if(level==2) level_tex = *level2_tex;
    else if(level==3) level_tex = *level3_tex;
    else if(level==4) level_tex = *level4_tex;
    scene->draw(camera, *bg_tex, *hatch0_tex, *hatch1_tex, *hatch2_tex,
            *hatch3_tex, *hatch4_tex, *hatch5_tex, level_tex, text_tex);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GL_ERRORS();
}

void GameMode::compare(GLuint player_tex, GLuint model_tex){
    updated = false;

    glBindTexture(GL_TEXTURE_2D, player_tex);
    GLuint *p_pixels = new GLuint[width*height*4];
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_INT, p_pixels);
    GLuint *m_pixels = new GLuint[width*height*4];
    glBindTexture(GL_TEXTURE_2D, model_tex);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_INT, m_pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    GL_ERRORS();

    int p_xoffset = width;
    int p_yoffset = height;
    int p_xend = 0;
    int p_yend = 0;
    for(int i = 0.25*height; i<0.85*height; i++){
        for(int j = 0.078*width; j<0.4*width; j++){
            int index = (j+i*width)*4;
            if((p_pixels[index]&0xFF)>0){
                p_xoffset = glm::min(p_xoffset, j);
                p_yoffset = glm::min(p_yoffset, i);
                p_xend = glm::max(p_xend, j);
                p_yend = glm::max(p_yend, i);
            }
        }
    }
    int p2_xoffset = width;
    int p2_yoffset = height;
    int p2_xend = 0;
    int p2_yend = 0;
    for(int i = 0.25*height; i<0.85*height; i++){
        for(int j = 0.688*width; j<0.99*width; j++){
            int index = (j+i*width)*4;
            if((p_pixels[index]&0xFF)>0){
                p2_xoffset = glm::min(p2_xoffset, j);
                p2_yoffset = glm::min(p2_yoffset, i);
                p2_xend = glm::max(p2_xend, j);
                p2_yend = glm::max(p2_yend, i);
            }
        }
    }
    //std::cout<<p_xoffset<<" "<<p_xend<<std::endl;
 //   std::cout<<p_yoffset<<" "<<p_yend<<std::endl;

    int m_xoffset = width;
    int m_yoffset = height;
    int m_xend = 0;
    int m_yend = 0;
    for(int i = 0.5*height; i<0.875*height; i++){
        for(int j = 0.45*width; j<0.625*width; j++){
            int index = (j+i*width)*4;
            if((m_pixels[index]&0xFF)>0){
                m_xoffset = glm::min(m_xoffset, j);
                m_yoffset = glm::min(m_yoffset, i);
                m_xend = glm::max(m_xend, j);
                m_yend = glm::max(m_yend, i);
            }
        }
    }
    //std::cout<<m_xoffset<<" "<<m_xend<<std::endl;
    //std::cout<<m_yoffset<<" "<<m_yend<<std::endl;

    score1 = 0;
    int max_score = 1;
    float p_xscale = (float)(p_xend-p_xoffset)/(float)(m_xend-m_xoffset);
    float p_yscale = (float)(p_yend-p_yoffset)/(float)(m_yend-m_yoffset);

    int iend = glm::min(m_yend-m_yoffset, p_yend-p_yoffset);
    int jend = glm::min(m_xend-m_xoffset, p_xend-p_xoffset);
    for(int i = 0; i<iend; i++){
        for(int j = 0; j<jend; j++){
            int mi = (m_yoffset+i)*4, mj = (m_xoffset+j)*4;
            int pi = (p_yoffset+i*p_yscale)*4, pj = (p_xoffset+j*p_xscale)*4;
            int m_index = mj+mi*width;
            int p_index = pj+pi*width;
            int m_color = (m_pixels[m_index]&0xFF00>>8);
            int p_color = (p_pixels[p_index]&0xFF00>>8);
            if(m_color>0)
                max_score++;
            if(m_color>0 && p_color>0)
                score1++;
        }
    }
    score1 = score1*200/max_score;
    score1 = glm::clamp(score1, 0, 100);

    p_xscale = (float)(p2_xend-p2_xoffset)/(float)(m_xend-m_xoffset);
    p_yscale = (float)(p2_yend-p2_yoffset)/(float)(m_yend-m_yoffset);

    iend = glm::min(m_yend-m_yoffset, p2_yend-p2_yoffset);
    jend = glm::min(m_xend-m_xoffset, p2_xend-p2_xoffset);
    for(int i = 0; i<iend; i++){
        for(int j = 0; j<jend; j++){
            int mi = (m_yoffset+i)*4, mj = (m_xoffset+j)*4;
            int pi = (p2_yoffset+i*p_yscale)*4, pj = (p2_xoffset+j*p_xscale)*4;
            int m_index = mj+mi*width;
            int p_index = pj+pi*width;
            int m_color = (m_pixels[m_index]&0xFF00>>8);
            int p_color = (p_pixels[p_index]&0xFF00>>8);
            if(m_color>0 && p_color>0)
                score2++;
        }
    }
    score2 = score2*200/max_score;
    score2 = glm::clamp(score2, 0, 100);

    delete[] p_pixels;
    delete[] m_pixels;
}

void GameMode::set_prim_uniforms(){
    int prim10[10];
    float posX10[10];
    float posY10[10];
    float posZ10[10];
    float rotX10[10];
    float rotY10[10];
    float rotZ10[10];
    float scale10[10];
    int prim10b[10];
    float posX10b[10];
    float posY10b[10];
    float posZ10b[10];
    float rotX10b[10];
    float rotY10b[10];
    float rotZ10b[10];
    float scale10b[10];
    //std::cout<<n<<" "<<nb<<std::endl;
    for(int i = 0; i<10; i++){
        if(i<state1.prim_num){
            prim10[i] = state1.primitives[i].shape;
            posX10[i] = state1.primitives[i].position.x;
            posY10[i] = state1.primitives[i].position.y;
            posZ10[i] = state1.primitives[i].position.z;
            rotX10[i] = state1.primitives[i].rotation.x;
            rotY10[i] = state1.primitives[i].rotation.y;
            rotZ10[i] = state1.primitives[i].rotation.z;
            scale10[i] = state1.primitives[i].scale;
        }
        if(i<state2.prim_num){
            prim10b[i] = state2.primitives[i].shape;
            posX10b[i] = state2.primitives[i].position.x;
            posY10b[i] = state2.primitives[i].position.y;
            posZ10b[i] = state2.primitives[i].position.z;
            rotX10b[i] = state2.primitives[i].rotation.x;
            rotY10b[i] = state2.primitives[i].rotation.y;
            rotZ10b[i] = state2.primitives[i].rotation.z;
            scale10b[i] = state2.primitives[i].scale;
        }
    }
    glUniform1iv(scene_program->primitives, 10, prim10);
    glUniform1fv(scene_program->positionsX, 10, posX10);
    glUniform1fv(scene_program->positionsY, 10, posY10);
    glUniform1fv(scene_program->positionsZ, 10, posZ10);
    glUniform1fv(scene_program->rotationsX, 10, rotX10);
    glUniform1fv(scene_program->rotationsY, 10, rotY10);
    glUniform1fv(scene_program->rotationsZ, 10, rotZ10);
    glUniform1fv(scene_program->scales, 10, scale10);
    glUniform1i(scene_program->selected, selected);
    glUniform1iv(scene_program->primitivesb, 10, prim10b);
    glUniform1fv(scene_program->positionsXb, 10, posX10b);
    glUniform1fv(scene_program->positionsYb, 10, posY10b);
    glUniform1fv(scene_program->positionsZb, 10, posZ10b);
    glUniform1fv(scene_program->rotationsXb, 10, rotX10b);
    glUniform1fv(scene_program->rotationsYb, 10, rotY10b);
    glUniform1fv(scene_program->rotationsZb, 10, rotZ10b);
    glUniform1fv(scene_program->scalesb, 10, scale10b);

}

void GameMode::draw(glm::uvec2 const &drawable_size) {
	textures.allocate(drawable_size);

    {//draw score and timer
		glDisable(GL_DEPTH_TEST);
        std::string message = "SCORE"+std::to_string(score1);
		float height = 0.05f;
        static GLuint fb = 0;
        if(fb==0) glGenFramebuffers(1, &fb);
        glBindFramebuffer(GL_FRAMEBUFFER, fb);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D, textures.text_tex, 0);

        {
            GLenum bufs[1] = {GL_COLOR_ATTACHMENT0};
            glDrawBuffers(1, bufs);
        }
        check_fb();

        GLfloat black[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        glClearBufferfv(GL_COLOR, 0, black);
		draw_text(message, glm::vec2(-1.0,-0.58), height);
        message = "SCORE "+std::to_string(score2);
		draw_text(message, glm::vec2(1.0,-0.58), height);
        message = "TIME "+std::to_string(int(time_left));
        height = 0.1;
		draw_text(message, glm::vec2(-0.2, 0.8), height);
    }
    draw_scene(textures.text_tex, &textures.color_tex, &textures.depth_tex,
            &textures.player_tex, &textures.model_tex);
    if(updated) compare(textures.player_tex, textures.model_tex);

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

void GameMode::reset(){
    time_left = 2.0;
    edit_mode = 0; //0 for translation, 1 for rotation, 2 for scaling
    state1.prim_num = 0;
    state2.prim_num = 0;
    updated = false;
    paused = false;
    score1 = 0;
    score2 = 0;
    selected = 0;
}

void GameMode::show_lose() {
	std::shared_ptr< MenuMode > menu = std::make_shared< MenuMode >();

	std::shared_ptr< Mode > game = shared_from_this();
	menu->background = game;

	menu->choices.emplace_back("YOU LOST");
    menu->choices.emplace_back("CONTINUE", [this, game](){
				level++;
                reset();
				Mode::set_current(game);
			});
	menu->choices.emplace_back("QUIT", [](){
			Mode::set_current(nullptr);
			});

	menu->selected = 2;
	paused = true;

	Mode::set_current(menu);
}

void GameMode::show_win() {
    std::cout<<"FKDSLFJLDSFL"<<std::endl;
	std::shared_ptr< MenuMode > menu = std::make_shared< MenuMode >();

	std::shared_ptr< Mode > game = shared_from_this();
	menu->background = game;

	menu->choices.emplace_back("YOU WON");
	menu->choices.emplace_back("CONTINUE", [this, game](){
				level++;
                reset();
				Mode::set_current(game);
			});
    menu->choices.emplace_back("QUIT", [](){
			Mode::set_current(nullptr);
			});

	menu->selected = 1;
	paused = true;

	Mode::set_current(menu);
}

