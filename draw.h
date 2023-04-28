#ifndef SCREEN_H
# define SCREEN_H

#include "vec3.h"

class Screen {
public:
    float focal;
    float sensorScale;
    int width;
    int height;
    float **data;
    Vec3 camPos;
    Vec3 sun;
    float camRot;
    void draw();
    void trace(int q);
    void randomizeLight();
    Screen();
};

void init_terminal();
void reset_terminal();

#endif