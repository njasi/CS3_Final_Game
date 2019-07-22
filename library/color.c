#include "color.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

const uint8_t HSV[61] =
{0, 4, 8, 13, 17, 21, 25, 30, 34, 38, 42, 47, 51, 55, 59, 64, 68, 72, 76, 81, 85, 89, 93, 98, 102, 106, 110, 115, 119, 123, 127, 132, 136, 140, 144, 149, 153, 157, 161, 166, 170, 174, 178, 183, 187, 191, 195, 200, 204, 208, 212, 217, 221, 225, 229, 234, 238, 242, 246, 251, 255};

RGBColor color_rand(void){
  double r = ((double)rand())/((double)RAND_MAX);
  double g = ((double)rand())/((double)RAND_MAX);
  double b = ((double)rand())/((double)RAND_MAX);
  // printf("R:%f\tG:%f\tB:%f\n",r,g,b);
  return (RGBColor){r,g,b};
}

RGBColor color_rainbow(int amount, int count){
    int angle = (int)(count * (360.0/(double)amount));
    int red, green, blue;

    if (angle<60) {
      red = 255;
      green = HSV[angle];
      blue = 0;
    }
    else if (angle<120) {
      red = HSV[120-angle];
      green = 255;
      blue = 0;
    }
    else if (angle<180) {
      red = 0;
      green = 255;
      blue = HSV[angle-120];
    }
    else if (angle<240) {
      red = 0;
      green = HSV[240-angle];
      blue = 255;
    }
    else if (angle<300) {
      red = HSV[angle-240];
      green = 0;
      blue = 255;
    }
    else{
        red = 255;
        green = 0;
        blue = HSV[360-angle];
      }
    return (RGBColor){((double)red)/255.0,
                      ((double)green)/255.0,
                      ((double)blue)/255.0};
}
