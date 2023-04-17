#ifndef VEC3_H
# define VEC3_H

#include "vec3.h"

class Vec3 {
public:
    float x;
    float y;
    float z;
    void normalize();
    Vec3 norm();
    float mag();
    Vec3 copy();
    void rotateY(float theta);
    static Vec3 cross(Vec3 a, Vec3 b);
    float dot(Vec3 v);
    float dist(Vec3 v);
    float dist2(Vec3 v);

    Vec3 operator+(Vec3 v);
    Vec3 operator-(Vec3 v);
    Vec3 operator+(float f);
    Vec3 operator-(float f);
    Vec3 operator*(float f);
    Vec3 operator/(float f);
    void operator=(Vec3 v);
    void operator=(float f);
    void operator+=(Vec3 v);
    void operator-=(Vec3 v);
    void operator+=(float f);
    void operator-=(float f);
    void operator*=(float f);
    void operator/=(float f);

    Vec3(float x, float y, float z);
    Vec3();
};

class Ray {
public:
    Vec3 pos;
    Vec3 dir;
    Ray(Vec3 origin, Vec3 direction);
    Ray();
};

#endif