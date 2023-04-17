#include "vec3.h"
#include "geometry.h"
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>


void Triangle::calcNormal() {
    normal = Vec3::cross(p2 - p1, p3 - p1);
    normal.normalize();
}

void Triangle::calcCenter() {
    center = (p1 + p2 + p3) / 3;
}

Triangle::Triangle(Vec3 p1, Vec3 p2, Vec3 p3) {
    this->p1 = p1;
    this->p2 = p2;
    this->p3 = p3;
    calcNormal();
    calcCenter();
}

// return 1 if ray intersects triangle
// return 0 if ray does not intersect triangle
int Triangle::rayIntersects(Ray r) {
    // TODO
    // use dot product of r.pos-triangle_pos and r.dir to make sure ray is pointing at triangle (and not a backwards intersection)
    Vec3 e1 = p2 - p1;
    Vec3 e2 = p3 - p1;
    Vec3 p = Vec3::cross(r.dir, e2);
    float a = e1.dot(p);
    if (a > -0.00001 && a < 0.00001) {
        return 0;
    }
    float f = 1 / a;
    Vec3 s = r.pos - p1;
    float u = f * s.dot(p);
    if (u < 0.0 || u > 1.0) {
        return 0;
    }
    Vec3 q = Vec3::cross(s, e1);
    float v = f * r.dir.dot(q);
    if (v < 0.0 || u + v > 1.0) {
        return 0;
    }
    float t = f * e2.dot(q);
    if (t > 0.00001) {
        return 1;
    } else {
        return 0;
    }
}

Vec3 Triangle::interectPoint(Ray r) {
    Vec3 e1 = p2 - p1;
    Vec3 e2 = p3 - p1;
    Vec3 p = Vec3::cross(r.dir, e2);
    float a = e1.dot(p);
    float f = 1 / a;
    Vec3 s = r.pos - p1;
    //float u = f * s.dot(p);
    Vec3 q = Vec3::cross(s, e1);
    //float v = f * r.dir.dot(q);
    float t = f * e2.dot(q);
    return r.pos + r.dir * t;
}


// add cube from 6 planes
// use the add_plane function
void add_cube(std::vector<Triangle> &triangles, Vec3 centerPos) {
    const float hw = 5; // half width
    triangles.push_back(Triangle(Vec3(hw, hw, hw)+centerPos, Vec3(hw, hw, -hw)+centerPos, Vec3(hw, -hw, -hw)+centerPos));
    triangles.push_back(Triangle(Vec3(hw, hw, hw)+centerPos, Vec3(hw, -hw, -hw)+centerPos, Vec3(hw, -hw, hw)+centerPos));
    triangles.push_back(Triangle(Vec3(-hw, hw, hw)+centerPos, Vec3(-hw, hw, -hw)+centerPos, Vec3(-hw, -hw, -hw)+centerPos));
    triangles.push_back(Triangle(Vec3(-hw, hw, hw)+centerPos, Vec3(-hw, -hw, -hw)+centerPos, Vec3(-hw, -hw, hw)+centerPos));
    triangles.push_back(Triangle(Vec3(hw, hw, hw)+centerPos, Vec3(hw, hw, -hw)+centerPos, Vec3(-hw, hw, -hw)+centerPos));
    triangles.push_back(Triangle(Vec3(hw, hw, hw)+centerPos, Vec3(-hw, hw, -hw)+centerPos, Vec3(-hw, hw, hw)+centerPos));
    triangles.push_back(Triangle(Vec3(hw, -hw, hw)+centerPos, Vec3(hw, -hw, -hw)+centerPos, Vec3(-hw, -hw, -hw)+centerPos));
    triangles.push_back(Triangle(Vec3(hw, -hw, hw)+centerPos, Vec3(-hw, -hw, -hw)+centerPos, Vec3(-hw, -hw, hw)+centerPos));
    triangles.push_back(Triangle(Vec3(hw, hw, hw)+centerPos, Vec3(hw, -hw, hw)+centerPos, Vec3(-hw, -hw, hw)+centerPos));
    triangles.push_back(Triangle(Vec3(hw, hw, hw)+centerPos, Vec3(-hw, -hw, hw)+centerPos, Vec3(-hw, hw, hw)+centerPos));
    triangles.push_back(Triangle(Vec3(hw, hw, -hw)+centerPos, Vec3(hw, -hw, -hw)+centerPos, Vec3(-hw, -hw, -hw)+centerPos));
    triangles.push_back(Triangle(Vec3(hw, hw, -hw)+centerPos, Vec3(-hw, -hw, -hw)+centerPos, Vec3(-hw, hw, -hw)+centerPos));
}

Vec3 parse_vert_line(std::string str){
    Vec3 v(0,0,0);
    int vec_index = 0;
    std::string num = "";
    for(int i = 2; i < (int) str.length(); i++){
        num += str[i];
        if(str[i] == ' ' || i == ((int)str.length())-1){
            float f = std::stof(num);
            if(vec_index == 0){
                v.x = f;
            }else if(vec_index == 1){
                v.y = f;
            }else if(vec_index == 2){
                v.z = f;
            }
            vec_index++;
            num = "";
        }

    }
    return v;
}

void parse_face_line(std::string str, int indices[3]){
    std::string num = "";
    int dontread = 0;
    int index = 0;
    for(int i = 2; i < (int) str.length(); i++){
        if(str[i] == '/') {
            dontread = 1;
        }else if(str[i] == ' ' || i == ((int)str.length())-1) {
            dontread = 0;
            int f = std::stoi(num);
            if(index > 2){
                std::cout << "error: more than 3 vertices in face" << std::endl;
                exit(1);
            }
            indices[index++] = f;
            num = "";
        }else {
            if(!dontread){
                num += str[i];
            }
        }
    }
}

void load_obj(std::vector<Triangle> &triangles, std::string path, float scale, Vec3 offset){
    std::vector<Vec3> verts;

    std::ifstream file;

    std::cout << "trying to open: " << path << std::endl;
    file.open(path);
    if(file.is_open()){
        std::cout << "file opened" << std::endl;
        char c;
        std::string line = "";
        while(file){
            c = file.get();
            //std::cout << c;
            if(c == '\n'){
                if(line[1] == ' '){
                    if(line[0] == 'v'){
                        Vec3 v = parse_vert_line(line)*(-scale) + offset;
                        verts.push_back(v);
                    }else if(line[0] == 'f'){
                        int indices[3];
                        parse_face_line(line, indices);
                        Triangle t(verts[indices[0]-1], verts[indices[1]-1], verts[indices[2]-1]);
                        triangles.push_back(t);
                    }
                }
                line = "";
            }else{
                line += c;
            }
        }

    } else{
        std::cout << "failed to open " << path << std::endl;
    }
}

void load_geometry(std::vector<Triangle> &triangles) {

    triangles.push_back(Triangle(Vec3(-200, 5, -200), Vec3(200, 5, -200), Vec3(0, 5, 300)));
    load_obj(triangles, "./dog.obj", 4.0, Vec3(0, 4, 20));
    // print all the triangles
    // for (int i = 0; i < (int) triangles.size(); i++) {
    //     std::cout << "triangle " << i << ": " << triangles[i].p1 << ", " << triangles[i].p2 << ", " << triangles[i].p3 << std::endl;
    // }
}

