#include <unistd.h>
#include <ncurses.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
//#include <string.h>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>

#include "draw.h"
#include "vec3.h"
#include "geometry.h"

#define screen_width 80*2
#define screen_height 24*2
#define focal_length 4.0
#define sensor_scale 0.15

#define MAX_TRIANGLES 10 // max triangles per box
#define MAX_DEPTH 10 // max depth of oct tree

std::vector<Triangle> allTriangles;

//const char brightMap[] = " .,-:+=&%#@";
const char brightMap[] = " .'`^,:;Il!i><~+_-?][}{1)(|/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
const int brightMapLen = sizeof(brightMap)/sizeof(brightMap[0])-1;

class Box { // oct tree hash
public:
    Vec3 rt; // right top
    Vec3 lb; // left bottom
    Box *children[8];
    Box *parent;
    int hasChildren;
    int depth;
    std::vector<Triangle*> triangles;
    std::vector<Triangle*> uniqueTris;

    Box(Vec3 rt, Vec3 lb) {
        hasChildren = 0;
        parent = NULL;
        depth = 0;
        for(int i = 0; i < 8; i++){
            children[i] = NULL;
        }
        this->rt = rt;
        this->lb = lb;
        if(lb.y < rt.y){ // up is negative
            std::cout << "error: box has negative height" << std::endl;
            exit(1);
        }
        if(lb.x > rt.x){ // right is positive
            std::cout << "error: box has negative width" << std::endl;
            exit(1);
        }
        if(lb.z > rt.z){ // forward is positive
            std::cout << "error: box has negative depth" << std::endl;
            exit(1);
        }
    }
    void addTriangles(){ // add triangles to box from global vector<Triangle> triangles
        int numTriangles;
        if(parent == NULL){
            numTriangles = (int) allTriangles.size();
        }else{
            numTriangles = (int) parent->triangles.size();
        }
        //std::cout << "adding triangles to box (depth = " << depth << ", numTri = " << numTriangles << ")" << std::endl;
        for(int i = 0; i < numTriangles; i++){
            Triangle *t;
            if(parent == NULL){
                t = &allTriangles[i];
                //std::cout << "root box has no parent (depth = " << depth << ", numTri = " << numTriangles << ")" << std::endl;
            }else {
                t = parent->triangles[i];
                //std::cout << "box has a parent" << std::endl;
            }
            int inBox = 1;
            for(int p = 0; p<3; p++){
                Vec3 point;
                if(p == 0){
                    point = t->p1;
                }else if(p == 1){
                    point = t->p2;
                }else if(p == 2){
                    point = t->p3;
                }

                if(point.x > rt.x || point.x < lb.x || point.y < rt.y || point.y > lb.y || point.z > rt.z || point.z < lb.z){
                    inBox=0;
                    //break;
                }
            }
            if(inBox){
                triangles.push_back(t);
                if(depth > t->depth){
                    t->depth = depth;
                }
            }
        }
        if(parent != NULL){
            //parent->triangles.clear();
            std::vector<Triangle*> v2;
            int parentDepth = parent->depth;
            for(int i = 0; i < (int) parent->triangles.size(); i++){
                if(parent->triangles[i]->depth <= parentDepth){
                    v2.push_back(parent->triangles[i]);
                }
            }
            parent->triangles = v2;
        }
    }
    void split() {
        if(hasChildren){
            reset_terminal();
            std::cout << "error: box already has children" << std::endl;
            exit(1);
        }
        Vec3 center = (rt+lb)/2;
        children[0] = new Box(rt, center);
        children[1] = new Box(Vec3(center.x, rt.y, rt.z), Vec3(lb.x, center.y, center.z));
        children[2] = new Box(Vec3(center.x, rt.y, center.z), Vec3(lb.x, center.y, lb.z));
        children[3] = new Box(Vec3(rt.x, rt.y, center.z), Vec3(center.x, center.y, lb.z));
        children[4] = new Box(Vec3(rt.x, center.y, rt.z), Vec3(center.x, lb.y, center.z));
        children[5] = new Box(Vec3(center.x, center.y, rt.z), Vec3(lb.x, lb.y, center.z));
        children[6] = new Box(center, lb);
        children[7] = new Box(Vec3(rt.x, center.y, center.z), Vec3(center.x, lb.y, lb.z));
        for(int i = 0; i < 8; i++){
            children[i]->parent = this;
            children[i]->depth = depth+1;
            children[i]->addTriangles();
            if((int) children[i]->triangles.size() > MAX_TRIANGLES && depth+1 < MAX_DEPTH){
                children[i]->split();
            }
        }
        hasChildren = 1;
    }
    int rayIntersects(Ray r){
        // r.dir is unit direction vector of ray
        Vec3 dirfrac;
        dirfrac.x = 1.0f / r.dir.x;
        dirfrac.y = 1.0f / r.dir.y;
        dirfrac.z = 1.0f / r.dir.z;

        float t1 = (lb.x - r.pos.x)*dirfrac.x;
        float t2 = (rt.x - r.pos.x)*dirfrac.x;
        float t3 = (lb.y - r.pos.y)*dirfrac.y;
        float t4 = (rt.y - r.pos.y)*dirfrac.y;
        float t5 = (lb.z - r.pos.z)*dirfrac.z;
        float t6 = (rt.z - r.pos.z)*dirfrac.z;

        float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
        float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

        //float t;
        // ray is faceing away from box
        if (tmax < 0)
        {
            //t = tmax;
            return 0;
        }

        if (tmin > tmax)
        {
            //t = tmax;
            return 0;
        }

        //t = tmin;
        return 1;
    }
};

Box hashbox(Vec3(80,-80,80), Vec3(-80,80,-80));

char brightness(float b){
    if(b < 0){
        b = 0;
    }else if(b >= 1){
        b = 0.999;
    }
    int b2 = b*brightMapLen;

    return brightMap[b2];
}


Screen::Screen() {
    load_geometry(allTriangles);
    std::cout << "num allTriangles: " << (int) allTriangles.size() << std::endl;
    hashbox.addTriangles();
    hashbox.split();
    /*std::cout << "\n ----- hashbox is inited ------- \n";
    std::cout << "num triangles (should = allTriangles): " << (int) hashbox.triangles.size() << std::endl;
    Box *chld = &hashbox;
    std::cout << "___parent depth: " << chld->depth << " (num Tris: " << chld->triangles.size() << ")" << std::endl;
    for(int i = 0; i<8; i++){
        std::cout << "child " << i << " num triangles: " << (int) chld->children[i]->triangles.size() << std::endl;
    }

    chld = chld->children[4];
    std::cout << "___parent depth: " << chld->depth << " (num Tris: " << chld->triangles.size() << ")" << std::endl;
    if(chld->hasChildren){
        for(int i = 0; i<8; i++){
        std::cout << "child " << i << " num triangles: " << (int) chld->children[i]->triangles.size() << std::endl;
        }
    }else{
        std::cout << "no children" << std::endl;
    }

    chld = chld->children[2];
    std::cout << "___parent depth: " << chld->depth << " (num Tris: " << chld->triangles.size() << ")" << std::endl;
    if(chld->hasChildren){
        for(int i = 0; i<8; i++){
        std::cout << "child " << i << " num triangles: " << (int) chld->children[i]->triangles.size() << std::endl;
        }
    }else{
        std::cout << "no children" << std::endl;
    }

    chld = chld->children[2];
    std::cout << "___parent depth: " << chld->depth << " (num Tris: " << chld->triangles.size() << ")" << std::endl;
    if(chld->hasChildren){
        for(int i = 0; i<8; i++){
        std::cout << "child " << i << " num triangles: " << (int) chld->children[i]->triangles.size() << std::endl;
        }
    }else{
        std::cout << "no children" << std::endl;
    }

    chld = chld->children[2];
    std::cout << "___parent depth: " << chld->depth << " (num Tris: " << chld->triangles.size() << ")" << std::endl;
    if(chld->hasChildren){
        for(int i = 0; i<8; i++){
        std::cout << "child " << i << " num triangles: " << (int) chld->children[i]->triangles.size() << std::endl;
        }
    }else{
        std::cout << "no children" << std::endl;
    }

    chld = chld->children[2];
    std::cout << "___parent depth: " << chld->depth << " (num Tris: " << chld->triangles.size() << ")" << std::endl;
    if(chld->hasChildren){
        for(int i = 0; i<8; i++){
        std::cout << "child " << i << " num triangles: " << (int) chld->children[i]->triangles.size() << std::endl;
        }
    }else{
        std::cout << "no children" << std::endl;
    }*/

    //exit(0);

    width = screen_width;
    height = screen_height;
    focal = focal_length;
    sensorScale = sensor_scale;
    sun = Vec3(80,-50,5);

    data = new float*[height];
    for (int i = 0; i < height; i++) {
        data[i] = new float[width];
    }
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            data[i][j] = 0.0;
        }
    }
}
void Screen::randomizeLight(){
    sun = Vec3(rand()%160-80, rand()%160-80, rand()%160-80);
}

void Screen::draw() {
    trace();
    for (int i = -1; i < height+1; i++) {
        for (int j = -1; j < width+1; j++) {
            if(i == -1 || i == height) {
                mvaddch(i+1, j+1, '-');
                continue;
            }
            if(j == -1 || j == width) {
                mvaddch(i+1, j+1, '|');
                continue;
            }
            char pixle = brightness(data[i][j]);
            mvaddch(i+1, j+1, pixle);
        }
    }
    refresh();
}

class IntersectPoint{
public:
    Triangle *closest;
    Vec3 closestPoint;
    float dist;
    IntersectPoint() {
        dist = 99999999.9;
        closest = NULL;
    }
    int hit(){
        if(dist < 99999999.8) return 1;
        return 0;
    }
};

void getClosest(Box *b, Ray r, IntersectPoint *ipnt) {

    if(b->hasChildren){
        for(int i = 0; i<8; i++){
            if(b->children[i]->rayIntersects(r)){
                getClosest(b->children[i], r, ipnt);
            }
        }
    }
    int numTris = (int) b->triangles.size();
    float minDist = ipnt->dist;
    //Triangle *closest_ = ipnt->closest;
    //Vec3 *closestPoint_ = ipnt->closestPoint;
    for(int i = 0; i<numTris; i++){
        Triangle *t = b->triangles[i];
        if(t->rayIntersects(r)){
            Vec3 point = t->interectPoint(r);
            float dist = r.pos.dist2(point);
            if(dist < minDist){
                minDist = dist;
                ipnt->closest = t;
                ipnt->closestPoint = point;
                ipnt->dist = dist;
            }
        }
    }
    

    /*int numTris = (int) b->uniqueTris.size();
    float minDist = ipnt->dist;
    for(int i = 0; i<numTris; i++){
        Triangle *t = b->uniqueTris[i];
        if(t->rayIntersects(r)){
            Vec3 point = t->interectPoint(r);
            float dist = r.pos.dist2(point);
            if(dist < minDist){
                minDist = dist;
                ipnt->closest = t;
                ipnt->closestPoint = point;
                ipnt->dist = dist;
            }
        }
    }
    if(b->hasChildren){
        for(int i = 0; i<8; i++){
            if(b->children[i]->rayIntersects(r)){
                getClosest(b->children[i], r, ipnt);
            }
        }
    }*/

    /*if(b->hasChildren){
        for(int i = 0; i<8; i++){
            if(b->children[i]->rayIntersects(r)){
                getClosest(b->children[i], r, ipnt);
            }
        }
    }
    int numTris = (int) b->uniqueTris.size();
    float minDist = ipnt->dist;
    for(int i = 0; i<numTris; i++){
        Triangle *t = b->uniqueTris[i];
        if(t->rayIntersects(r)){
            Vec3 point = t->interectPoint(r);
            float dist = r.pos.dist2(point);
            if(dist < minDist){
                minDist = dist;
                ipnt->closest = t;
                ipnt->closestPoint = point;
                ipnt->dist = dist;

            }
        }
    }*/

}

void Screen::trace() {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            float sensorY = (i-height/2.0)*sensorScale;
            float sensorX = (j-width/2.0)*sensorScale*0.5;

            Vec3 pos = camPos.copy();
            Vec3 dir(sensorX,sensorY,focal);
            dir.normalize();
            Ray r = Ray(pos, dir);
            r.dir.rotateY(camRot);
            
            data[i][j] = 0;

            
            //Triangle *closest = NULL;
            //Vec3 closestPoint;
            IntersectPoint ip;

            getClosest(&hashbox, r, &ip);
            
            if(ip.hit()){
                float surfaceBrightness = ip.closest->normal.dot((ip.closestPoint-sun).norm());
                if(surfaceBrightness < 0) surfaceBrightness *= -1;
                //surfaceBrightness *= 1/(closestPoint.dist2(sun));

                
                // shadows
                Vec3 closestPoint = ip.closestPoint-(r.dir*0.001);                
                Ray toSun;
                toSun.pos = closestPoint;
                toSun.dir = (sun-closestPoint).norm();

                IntersectPoint ip2;
                getClosest(&hashbox, toSun, &ip2);
                if(ip2.hit()) surfaceBrightness *= 0.25;



                data[i][j] = surfaceBrightness*0.5;                
            }
            
        }
    }
}

void init_terminal()
{

/*for(float j = 0; j<1.1; j+=0.05){
    std::cout << j << ": " << brightness(j) << std::endl;
}
exit(0);*/

  initscr();
  raw();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  start_color();
  init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
  init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
  init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
  init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
  init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
  init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
}

void reset_terminal()
{
    endwin();
}