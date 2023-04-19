#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <ncurses.h>
#include <math.h>

#include "draw.h"

#define MOVE_SPEED 2.0
#define ROT_SPEED 0.1

int upPressed = 0;


int main(int argc, char *argv[])
{
  // seed random
  srand(time(NULL));
  

  init_terminal();

  //cbreak();
  //noecho();
  //nodelay(stdscr, TRUE);
  //timeout(100);
  scrollok(stdscr, TRUE);

  Screen scr;
  
  while(1) {
    scr.draw();

    int key = getch();

    //std::cout <<  std::endl << key << std::endl;
    if(key == 'q' || key == 'Q') {
      break;
    }
    if(key == 'r') {
      scr.randomizeLight();
      continue;
    }
    float theta = scr.camRot;
    if(key == 258) { // down
      scr.camPos.z -= cos(theta)*MOVE_SPEED;
      scr.camPos.x += sin(theta)*MOVE_SPEED;
    }else if(key == 259) { // up
      scr.camPos.z += cos(theta)*MOVE_SPEED;
      scr.camPos.x -= sin(theta)*MOVE_SPEED;
    }else if(key == 260) { // left
      scr.camRot += ROT_SPEED;
    }else if(key == 261) { // right
      scr.camRot -= ROT_SPEED;
    }else if(key == 97) { // a
      scr.camPos.z -= sin(theta)*MOVE_SPEED;
      scr.camPos.x -= cos(theta)*MOVE_SPEED;
    }else if(key == 100) { // d
      scr.camPos.z += sin(theta)*MOVE_SPEED;
      scr.camPos.x += cos(theta)*MOVE_SPEED;
    }else if(key == 119) { // w
      scr.camPos.y -= MOVE_SPEED;
    }else if(key == 115) { // s
      scr.camPos.y += MOVE_SPEED;
    }
    
  }

  reset_terminal();
  
  return 0;
}
