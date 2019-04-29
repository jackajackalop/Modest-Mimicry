#pragma once

#include "Mode.hpp"
#include "Load.hpp"
#include "Game.hpp"

#include "MeshBuffer.hpp"
#include "GL.hpp"
#include "Connection.hpp"

#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <chrono>

#include <vector>

// The 'GameMode' mode is the main gameplay mode:

struct GameMode : public Mode {
	GameMode(Client &client);
	virtual ~GameMode();

	//handle_event is called when new mouse or keyboard events are received:
	// (note that this might be many times per frame or never)
	//The function should return 'true' if it handled the event.
	virtual bool handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) override;

	//update is called at the start of a new frame, after events are handled:
	virtual void update(float elapsed) override;

	//draw is called after update:
	virtual void draw(glm::uvec2 const &drawable_size) override;
    void draw_surface(GLuint* color_tex_, GLuint* model_tex_, GLuint* ui_tex_);
    void draw_main(GLuint text_tex, GLuint bg_tex, GLuint model_tex,
            GLuint ui_tex_, GLuint* color_tex_, GLuint* player_tex_);
    void compare(GLuint player_tex, GLuint model_tex);
    void add_primitive(int primitive_type);
    void set_prim_uniforms();
    void reset();
    void show_lose();
    void show_win();

	float camera_spin = 0.0f;
	float spot_spin = 0.0f;
    int level = 0;
    int score1 = 0;
    int score2 = 0;
    int selected = 0;
    char playerNum;
	//------ networking ------
	Client &client; //client object; manages connection to server.
};
