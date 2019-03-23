#pragma once

#include <glm/glm.hpp>
#include <math.h>
#include <vector>

class Primitive{
        public:
            glm::vec3 position = glm::vec3(0.0, 0.0, 0.0);//2.0,0.5,0.25);
            glm::vec3 rotation = glm::vec3(0.0,0.0,0.0); //euler angle values
            float scale = 1.0f;
            int shape = 0;
    };


struct Game {
    Primitive primitives[10];
    int prim_num = 0;
};
