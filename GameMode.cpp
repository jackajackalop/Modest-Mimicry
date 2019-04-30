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
#include "effect_program.hpp"
#include "scene_program.hpp"
#include "main_program.hpp"
#include "surface_program.hpp"
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
                "    gl_Position = vec4(4 * (gl_VertexID & 1) - 1,  2 * (gl_VertexID & 2) - 1, 0.0, 1.0);\n"
                "}\n"
                ,
                //NOTE on reading screen texture:
                //texelFetch() gives direct pixel access with integer coordinates, but accessing out-of-bounds pixel is undefined:
                //    vec4 color = texelFetch(tex, ivec2(gl_FragCoord.xy), 0);
                //texture() requires using [0,1] coordinates, but handles out-of-bounds more gracefully (using wrap settings of underlying texture):
                //    vec4 color = texture(tex, gl_FragCoord.xy / textureSize(tex,0));

                "#version 330\n"
                "uniform sampler2D tex;\n"
                "out vec4 fragColor;\n"
                "void main() {\n"
                "    fragColor = texelFetch(tex, ivec2(gl_FragCoord.xy), 0);\n"
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
GLuint load_pic(std::string const &filename) {
    glm::uvec2 size;
    std::vector< glm::u8vec4 > data;
    load_png(filename, &size, &data, LowerLeftOrigin);

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
Load< GLuint > bg(LoadTagDefault, [](){
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
Load< GLuint > menu0_tex(LoadTagDefault, [](){
        return new GLuint(load_pic(data_path("textures/menu1.png")));
        });
Load< GLuint > menu1_tex(LoadTagDefault, [](){
        return new GLuint(load_pic(data_path("textures/menu2.png")));
        });
Load< GLuint > menu2_tex(LoadTagDefault, [](){
        return new GLuint(load_pic(data_path("textures/menu3.png")));
        });
Load< GLuint > menu3_tex(LoadTagDefault, [](){
        return new GLuint(load_pic(data_path("textures/menu4.png")));
        });
Load< GLuint > menu4_tex(LoadTagDefault, [](){
        return new GLuint(load_pic(data_path("textures/menu5.png")));
        });
Load< GLuint > expand_tex(LoadTagDefault, [](){
        return new GLuint(load_pic(data_path("textures/expand.png")));
        });
Load< GLuint > expand2_tex(LoadTagDefault, [](){
        return new GLuint(load_pic(data_path("textures/expand2.png")));
        });
Load< GLuint > sphere_tex(LoadTagDefault, [](){
        return new GLuint(load_pic(data_path("textures/sphere.png")));
        });
Load< GLuint > cube_tex(LoadTagDefault, [](){
        return new GLuint(load_pic(data_path("textures/cube.png")));
        });
Load< GLuint > cone_tex(LoadTagDefault, [](){
        return new GLuint(load_pic(data_path("textures/cone.png")));
        });
Load< GLuint > cylinder_tex(LoadTagDefault, [](){
        return new GLuint(load_pic(data_path("textures/cylinder.png")));
        });
Load< GLuint > clock_tex(LoadTagDefault, [](){
        return new GLuint(load_pic(data_path("textures/clock.png")));
        });
Load< GLuint > hand_tex(LoadTagDefault, [](){
        return new GLuint(load_pic(data_path("textures/hand.png")));
        });
Load< GLuint > spotlight_tex(LoadTagDefault, [](){
        return new GLuint(load_pic(data_path("textures/spotlight.png")));
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
int time_left = 100;
int elapsed_time = 0;
int edit_mode = 0; //0 for translation, 1 for rotation, 2 for scaling, 3 for adding primitives
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
            /*        if (t->name == "SpotParent") {
                    if (spot_parent_transform) throw std::runtime_error("Multiple 'SpotParent' transforms in scene.");
                    spot_parent_transform = t;
                    }
                    */
        }
        if (!camera_parent_transform) throw std::runtime_error("No 'CameraParent' transform in scene.");
        //    if (!spot_parent_transform) throw std::runtime_error("No 'SpotParent' transform in scene.");

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

    float z;
    if(playerNum=='0'){
        z = 0.8;
    }else{
        z= -1.4;
    }
    new_prim.position = glm::vec3(0, -0.3, z);
    state1.primitives[state1.prim_num] = new_prim;
    state1.prim_num++;
    edit_mode = 0;
    std::cout<<"added primitive "<<primitive_type<<std::endl;
    updated = true;
}

bool GameMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
    //ignore any keys that are the result of automatic key repeat:

    if (evt.type == SDL_KEYDOWN) {
        updated = true;
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
        int x = evt.button.x;
        int y = evt.button.y;
        if(y>0.8175*height){
            if(x>0.30*width && x<0.35*width) edit_mode = 0;
            else if(x>0.39*width && x<0.44*width) edit_mode = 1;
            else if(x>0.48*width && x<0.53*width) edit_mode = 2;
            else if(x>0.56*width && x<0.61*width) edit_mode = 3;
            else if(x>0.65*width && x<0.70*width) edit_mode = 4;
        }else if(edit_mode==3 && y>0.675*height && y<0.70*height){
            if(x>0.36*width && x<0.41*width) add_primitive(1);
            else if(x>0.42*width && x<0.47*width) add_primitive(2);
            else if(x>0.48*width && x<0.53*width) add_primitive(3);
            else if(x>0.54*width && x<0.60*width) add_primitive(4);
        }else if(edit_mode==4 && y>0.67*height && y<0.735*height){
            if(x>0.26*width && x<0.30*width) selected=0;
            else if(x>0.30*width && x<0.34*width) selected=1;
            else if(x>0.34*width && x<0.38*width) selected=2;
            else if(x>0.39*width && x<0.43*width) selected=3;
            else if(x>0.43*width && x<0.47*width) selected=4;
            else if(x>0.48*width && x<0.52*width) selected=5;
            else if(x>0.52*width && x<0.56*width) selected=6;
            else if(x>0.56*width && x<0.60*width) selected=7;
            else if(x>0.60*width && x<0.65*width) selected=8;
            else if(x>0.65*width && x<0.70*width) selected=9;
            if(x>0.26*width && x<0.70*width) edit_mode = 0;
        }
    }
    return false;
}

void GameMode::update(float elapsed) {
    if(paused) return;
    camera_parent_transform->rotation = glm::angleAxis(camera_spin, glm::vec3(0.0f, 0.0f, 1.0f));
    //    spot_parent_transform->rotation = glm::angleAxis(spot_spin, glm::vec3(0.0f, 0.0f, 1.0f));
    time_left = (100*(level+1))-elapsed_time;
    if(time_left<=0){
        if(state1.score>state2.score){
            show_win();
        }else{
            show_lose();
        }
    }

    if (client.connection) {
        //send game state to server:
        client.connection.send_raw("a", 1);
        client.connection.send_raw(&state1, sizeof(state1));
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
            if (c->recv_buffer.size() < 1 + sizeof(state2)) {
            return; //wait for more data
            } else {
            memcpy(&state2, c->recv_buffer.data()+1,
                    sizeof(state2));
            memcpy(&elapsed_time,
                    c->recv_buffer.data()+1+sizeof(state2),
                    sizeof(int));
            c->recv_buffer.erase(c->recv_buffer.begin(),
                    c->recv_buffer.begin()+ 1 + sizeof(state2)+sizeof(int));
            //std::cout<<state1.score<<" "<<state2.score<<std::endl;
            }
            }
    });

}

//GameMode will render to some offscreen framebuffer(s).
//This code allocates and resizes them as needed:
struct Textures {
    glm::uvec2 size = glm::uvec2(0,0); //remember the size of the framebuffer

    GLuint color_tex = 0;
    GLuint bg_tex = 0;
    GLuint hatched_tex = 0;
    GLuint depth_tex = 0;
    GLuint player_tex = 0;
    GLuint model_tex = 0;
    GLuint text_tex = 0;
    GLuint ui_tex = 0;
    GLuint final_tex = 0;
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
            alloc_tex(&bg_tex, GL_RGBA, GL_RGBA);
            alloc_tex(&hatched_tex, GL_RGBA, GL_RGBA);
            alloc_tex(&depth_tex, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT);
            alloc_tex(&player_tex, GL_RGBA, GL_RGBA);
            alloc_tex(&model_tex, GL_RGBA, GL_RGBA);
            alloc_tex(&text_tex, GL_RGBA, GL_RGBA);
            alloc_tex(&ui_tex, GL_RGBA, GL_RGBA);
            alloc_tex(&final_tex, GL_RGBA, GL_RGBA);
            GL_ERRORS();
        }

    }
} textures;

void GameMode::draw_surface(GLuint *bg_tex_, GLuint *model_tex_,
        GLuint *ui_tex_){
    assert(bg_tex_);
    assert(model_tex_);
    assert(ui_tex_);
    auto &bg_tex = *bg_tex_;
    auto &model_tex = *model_tex_;
    auto &ui_tex = *ui_tex_;

    static GLuint fb = 0;
    if(fb==0) glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            bg_tex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
            model_tex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
            ui_tex, 0);
    GLenum bufs[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                        GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, bufs);
    check_fb();

    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glViewport(0,0, textures.size.x, textures.size.y);
    camera->aspect = textures.size.x / float(textures.size.y);

    GLfloat black[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    glClearBufferfv(GL_COLOR, 0, black);

    //set up basic OpenGL state:
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint level_tex = 0;
    if(level==0) level_tex= *level0_tex;
    else if(level==1) level_tex = *level1_tex;
    else if(level==2) level_tex = *level2_tex;
    else if(level==3) level_tex = *level3_tex;
    else if(level==4) level_tex = *level4_tex;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, *bg);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, level_tex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, *menu0_tex);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, *menu1_tex);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, *menu2_tex);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, *menu3_tex);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, *menu4_tex);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, *expand_tex);
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, *expand2_tex);
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, *sphere_tex);
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, *cube_tex);
    glActiveTexture(GL_TEXTURE11);
    glBindTexture(GL_TEXTURE_2D, *cone_tex);
    glActiveTexture(GL_TEXTURE12);
    glBindTexture(GL_TEXTURE_2D, *cylinder_tex);
    glActiveTexture(GL_TEXTURE13);
    glBindTexture(GL_TEXTURE_2D, *clock_tex);
    glActiveTexture(GL_TEXTURE14);
    glBindTexture(GL_TEXTURE_2D, *hand_tex);
    glUseProgram(surface_program->program);

    int prim10[10] = {0};
    for(int i = 0; i<10; i++){
        if(i<state1.prim_num){
            prim10[i] = state1.primitives[i].shape;
        }
    }
    glUniform1iv(surface_program->primitives, 10, prim10);
    glUniform1i(surface_program->width, width);
    glUniform1i(surface_program->height, height);
    glUniform1i(surface_program->time_left, time_left);
    glUniform1i(surface_program->edit_mode, edit_mode);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, 0);

}

void GameMode::draw_main(GLuint text_tex, GLuint bg_tex, GLuint model_tex,
        GLuint ui_tex, GLuint* color_tex_, GLuint* player_tex_){
    assert(color_tex_);
    assert(player_tex_);
    auto &color_tex = *color_tex_;
    auto &player_tex = *player_tex_;

    static GLuint fb = 0;
    if(fb==0) glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            color_tex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
            player_tex, 0);
    GLenum bufs[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, bufs);
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

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bg_tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, *hatch0_tex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, *hatch1_tex);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, *hatch2_tex);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, *hatch3_tex);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, *hatch4_tex);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, *hatch5_tex);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, model_tex);
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, text_tex);
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, ui_tex);

    glUseProgram(main_program->program);
    set_prim_uniforms();
    glDrawArrays(GL_TRIANGLES, 0, 3);
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
    int jmin, jmax;
    if(playerNum=='0'){
        jmin = 0.078*width;
        jmax = 0.4*width;
    }else{
        jmin = 0.688*width;
        jmax = 0.99*width;
    }
    for(int i = 0.25*height; i<0.85*height; i++){
        for(int j = jmin; j<jmax; j++){
            int index = (j+i*width)*4;
            if((p_pixels[index]&0xFF)>0){
                p_xoffset = glm::min(p_xoffset, j);
                p_yoffset = glm::min(p_yoffset, i);
                p_xend = glm::max(p_xend, j);
                p_yend = glm::max(p_yend, i);
            }
        }
    }
    //std::cout<<p_xoffset<<" "<<p_xend<<std::endl;
    //std::cout<<p2_xoffset<<" "<<p2_xend<<std::endl;
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

    state1.score = 0;
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
                state1.score++;
        }
    }
    state1.score = state1.score*200/max_score;
    state1.score = glm::clamp(state1.score, 0, 100);

    delete[] p_pixels;
    delete[] m_pixels;
}

void GameMode::set_prim_uniforms(){
    int prim10[10] = {0};
    float posX10[10] = {0.0};
    float posY10[10] = {0.0};
    float posZ10[10] = {0.0};
    float rotX10[10] = {0.0};
    float rotY10[10] = {0.0};
    float rotZ10[10] = {0.0};
    float scale10[10] = {1.0f};
    int prim10b[10] = {0};
    float posX10b[10] = {0.0};
    float posY10b[10] = {0.0};
    float posZ10b[10] = {0.0};
    float rotX10b[10] = {0.0};
    float rotY10b[10] = {0.0};
    float rotZ10b[10] = {0.0};
    float scale10b[10] = {1.0f};
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
    glUniform1iv(main_program->primitives, 10, prim10);
    glUniform1fv(main_program->positionsX, 10, posX10);
    glUniform1fv(main_program->positionsY, 10, posY10);
    glUniform1fv(main_program->positionsZ, 10, posZ10);
    glUniform1fv(main_program->rotationsX, 10, rotX10);
    glUniform1fv(main_program->rotationsY, 10, rotY10);
    glUniform1fv(main_program->rotationsZ, 10, rotZ10);
    glUniform1fv(main_program->scales, 10, scale10);
    glUniform1i(main_program->selected, selected);
    glUniform1iv(main_program->primitivesb, 10, prim10b);
    glUniform1fv(main_program->positionsXb, 10, posX10b);
    glUniform1fv(main_program->positionsYb, 10, posY10b);
    glUniform1fv(main_program->positionsZb, 10, posZ10b);
    glUniform1fv(main_program->rotationsXb, 10, rotX10b);
    glUniform1fv(main_program->rotationsYb, 10, rotY10b);
    glUniform1fv(main_program->rotationsZb, 10, rotZ10b);
    glUniform1fv(main_program->scalesb, 10, scale10b);
    glUniform1i(main_program->width, width);
    glUniform1i(main_program->height, height);
}

void GameMode::draw_effect(GLuint color_tex, GLuint *final_tex_){
    assert(final_tex_);
    auto &final_tex = *final_tex_;

    static GLuint fb = 0;
    if(fb==0) glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            final_tex, 0);
    GLenum bufs[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, bufs);
    check_fb();

    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glViewport(0,0, textures.size.x, textures.size.y);
    camera->aspect = textures.size.x / float(textures.size.y);

    GLfloat black[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    glClearBufferfv(GL_COLOR, 0, black);

    //set up basic OpenGL state:
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, color_tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, *spotlight_tex);
    glUseProgram(effect_program->program);

    int scoreL = (playerNum=='0' ? state1.score : state2.score);
    int scoreR = (playerNum=='0' ? state2.score : state1.score);
    glUniform1i(effect_program->score1, scoreL);
    glUniform1i(effect_program->score2, scoreR);
    glUniform1i(effect_program->width, width);
    glUniform1i(effect_program->height, height);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);

}

void GameMode::draw(glm::uvec2 const &drawable_size) {
    textures.allocate(drawable_size);
    {//draw score and timer
        glDisable(GL_DEPTH_TEST);
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
       /* float height = 0.05f;
        std::string message ="SCORE"+std::to_string(playerNum=='0'?state1.score:state2.score);
        draw_text(message, glm::vec2(-1.0,-0.58), height);
        message = "SCORE "+std::to_string(playerNum=='1'?state1.score:state2.score);
        draw_text(message, glm::vec2(1.0,-0.58), height);
        message = "TIME "+std::to_string(int(time_left));
        height = 0.1;
        draw_text(message, glm::vec2(-0.2, 0.8), height);
        */
    }
    width = textures.size.x;
    height = textures.size.y;

    glUseProgram(scene_program->program);
    glUniform3fv(scene_program->viewPos, 1,
            glm::value_ptr(camera->transform->make_local_to_world()));
    scene->draw(camera);
    draw_surface(&textures.bg_tex, &textures.model_tex, &textures.ui_tex);
    draw_main(textures.text_tex, textures.bg_tex, textures.model_tex,
            textures.ui_tex, &textures.color_tex, &textures.player_tex);
    draw_effect(textures.color_tex, &textures.final_tex);
    if(updated) compare(textures.player_tex, textures.model_tex);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GL_ERRORS();

    //Copy scene from color buffer to screen, performing post-processing effects:
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures.final_tex);

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
    time_left = 100.0*(level+1);
    edit_mode = 0; //0 for translation, 1 for rotation, 2 for scaling
    state1.prim_num = 0;
    state2.prim_num = 0;
    updated = false;
    paused = false;
    state1.score = 0;
    state2.score = 0;
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

    menu->selected = 1;
    paused = true;

    Mode::set_current(menu);
}

void GameMode::show_win() {
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

