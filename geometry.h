#ifndef GEO_H
# define GEO_H

#include "vec3.h"
#include <vector>
#include <string>

class Triangle {
public:
    Vec3 p1;
    Vec3 p2;
    Vec3 p3;
    Vec3 normal;
    Vec3 center;
    void calcNormal();
    void calcCenter();
    int rayIntersects(Ray r);
    Vec3 interectPoint(Ray r);
    Triangle(Vec3 p1, Vec3 p2, Vec3 p3);
};

void load_geometry(std::vector<Triangle> &triangles);

#endif