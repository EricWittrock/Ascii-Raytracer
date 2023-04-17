#include <unistd.h>
#include <ncurses.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <string>
#include <iostream>
#include <vector>

#include "draw.h"
#include "vec3.h"
#include "geometry.h"

#define screen_width 80*2
#define screen_height 24*2
#define focal_length 4.0
#define sensor_scale 0.15

#define MAX_TRIANGLES 10 // max triangles per box
#define MAX_DEPTH 6 // max depth of oct tree

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
        std::cout << "adding triangles to box (depth = " << depth << ", numTri = " << numTriangles << ")" << std::endl;
        for(int i = 0; i < numTriangles; i++){
            Triangle *t;
            if(parent == NULL){
                t = &allTriangles[i];
                std::cout << "box has no parent (is root)" << std::endl;
            }else{
                t = parent->triangles[i];
                std::cout << "box has a parent" << std::endl;
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
                    inBox = 0;
                    break;
                }
            }
            if(inBox){
                triangles.push_back(t);
            }
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
    /*std::cout << "num allTriangles: " << (int) allTriangles.size() << std::endl;
    hashbox.addTriangles();
    hashbox.split();
    std::cout << "\n ----- hashbox is inited ------- \n";
    std::cout << "has children: " << hashbox.hasChildren << std::endl;
    std::cout << "num triangles: " << (int) hashbox.triangles.size() << std::endl;
    for(int i = 0; i<8; i++){
        std::cout << "child " << i << " num triangles: " << (int) hashbox.children[i]->triangles.size() << std::endl;
    }
    for(int i = 0; i<8; i++){
        std::cout << "child " << i << " has children: " << hashbox.children[i]->hasChildren << std::endl;
    }
    // log for child[0]'s children
    for(int i = 0; i<8; i++){
        std::cout << "child[0] child " << i << " num triangles: " << (int) hashbox.children[0]->children[i]->triangles.size() << std::endl;
    }
    // log for child[0][6]'s children
    for(int i = 0; i<8; i++){
        std::cout << "child[0][6] child " << i << " num triangles: " << (int) hashbox.children[0]->children[6]->children[i]->triangles.size() << std::endl;
    }
    // log for child[0][6][5]'s chlidren
    for(int i = 0; i<8; i++){
        std::cout << "child[0][6][5] child " << i << " num triangles: " << (int) hashbox.children[0]->children[6]->children[5]->children[i]->triangles.size() << std::endl;
    }
    // log for child[0][6][5][2]'s chlidren
    for(int i = 0; i<8; i++){
        std::cout << "child[0][6][5][2] child " << i << " num triangles: " << (int) hashbox.children[0]->children[6]->children[5]->children[2]->children[i]->triangles.size() << std::endl;
    }
    // TODO 6th index (instead of 2) causes segfault

    exit(0);*/
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

            float minDist = INT_MAX;
            Triangle *closest = NULL;
            Vec3 closestPoint;
            for(int k = 0; k < (int) allTriangles.size(); k++){
                //triangles[k].calcNormal();
                Triangle *t = &allTriangles[k];
                if(t->rayIntersects(r)){
                    Vec3 point = t->interectPoint(r);
                    float dist = r.pos.dist2(point);
                    if(dist < minDist){
                        minDist = dist;
                        closest = t;
                        closestPoint = point;
                    }
                }
            }
            if(closest != NULL){
                //Vec3 lightDir(1,4,0);
                //lightDir.normalize();

                float surfaceBrightness = closest->normal.dot((closestPoint-sun).norm());
                //float surfaceBrightness = closest->normal.dot(lightDir);
                if(surfaceBrightness < 0) surfaceBrightness *= -1;
                //float surfaceBrightness = closest->normal.x*5+closest->normal.y*2+closest->normal.z*1;
                //surfaceBrightness *= 1/(closestPoint.dist2(sun));
                
                //data[i][j] = brightness(2);

                closestPoint -= r.dir*0.001;

                Ray toSun;
                toSun.pos = closestPoint;
                toSun.dir = (sun-closestPoint).norm();

                for(int k = 0; k < (int) allTriangles.size(); k++) {
                    Triangle *t = &allTriangles[k];
                    if(t->rayIntersects(toSun)){
                        surfaceBrightness *= 0.25;
                    }
                }

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