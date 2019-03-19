#pragma once

#include <glm/glm.hpp>
#include <math.h>

struct Game {
	glm::vec2 body_pos = glm::vec2(0.0f, 0.0f);
	float thighR_angle = 0.0f;
	float thighL_angle = 0.0f;
	float calfR_angle = 0.0f;
	float calfL_angle = 0.0f;
	float calfR_old = 0.0f;
	float calfL_old = 0.0f;

	glm::vec2 body_pos2 = glm::vec2(0.0f, 0.0f);
    float thighR_angle2 = 0.0f;
	float thighL_angle2 = 0.0f;
	float calfR_angle2 = 0.0f;
	float calfL_angle2 = 0.0f;
	float calfR_old2 = 0.0f;
	float calfL_old2 = 0.0f;

	glm::vec2 body_velocity = glm::vec2(0.0f, 0.0f);
	glm::vec2 body_velocity2 = glm::vec2(0.0f, 0.0f);

	void update(float time, char playerNum);

};
