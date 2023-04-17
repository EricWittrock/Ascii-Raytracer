#include "vec3.h"

#include <math.h>

Vec3::Vec3() {
    x = 0;
    y = 0;
    z = 0;
}
Vec3::Vec3(float x, float y, float z) {
    this->x = x;
    this->y = y;
    this->z = z;
}
void Vec3::normalize() {
    float mag = sqrt(x * x + y * y + z * z);
    x /= mag;
    y /= mag;
    z /= mag;
}
Vec3 Vec3::norm(){
    float mag = sqrt(x * x + y * y + z * z);
    return Vec3(x/mag, y/mag, z/mag);
}
float Vec3::mag(){
    return sqrt(x * x + y * y + z * z);
}
Vec3 Vec3::copy() {
    return Vec3(x, y, z);
}
void Vec3::rotateY(float theta){
    float x1 = x*cos(theta) - z*sin(theta);
    float z1 = x*sin(theta) + z*cos(theta);
    x = x1;
    z = z1;
}
Vec3 Vec3::cross(Vec3 a, Vec3 b) {
    return Vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
float Vec3::dot(Vec3 v){
    return x*v.x+y*v.y+z*v.z;
}
float Vec3::dist2(Vec3 v){
    return pow(x-v.x,2) + pow(y-v.y,2) + pow(z-v.z,2);
}
float Vec3::dist(Vec3 v){
    return sqrt(pow(x-v.x,2) + pow(y-v.y,2) + pow(z-v.z,2));
}

// define vec3 operators
Vec3 Vec3::operator+(Vec3 v){
    return Vec3(v.x+x, v.y+y, v.z+z);
}
Vec3 Vec3::operator-(Vec3 v){
    return Vec3(x-v.x, y-v.y, z-v.z);
}
Vec3 Vec3::operator+(float f){
    return Vec3(x+f, y+f, z+f);
}
Vec3 Vec3::operator-(float f){
    return Vec3(x-f, y-f, z-f);
}
Vec3 Vec3::operator*(float f){
    return Vec3(x*f, y*f, z*f);
}
Vec3 Vec3::operator/(float f){
    return Vec3(x/f, y/f, z/f);
}
void Vec3::operator=(Vec3 v){
    x = v.x;
    y = v.y;
    z = v.z;
}
void Vec3::operator=(float f){
    x = f;
    y = f;
    z = f;
}
void Vec3::operator+=(Vec3 v){
    x += v.x;
    y += v.y;
    z += v.z;
}
void Vec3::operator-=(Vec3 v){
    x -= v.x;
    y -= v.y;
    z -= v.z;
}
void Vec3::operator+=(float f){
    x += f;
    y += f;
    z += f;
}
void Vec3::operator-=(float f){
    x -= f;
    y -= f;
    z -= f;
}
void Vec3::operator*=(float f){
    x *= f;
    y *= f;
    z *= f;
}
void Vec3::operator/=(float f){
    x /= f;
    y /= f;
    z /= f;
}


Ray::Ray() {
    pos = Vec3();
    dir = Vec3();
}

Ray::Ray(Vec3 origin, Vec3 direction) {
    pos = origin;
    dir = direction;
}

