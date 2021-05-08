#include <iostream>
#include <stdio.h>
#include <array>
#include <list>
#include <vector>
#include <algorithm>
#include "draw.h"
#include "matfunctions.h"
#include "objReader.h"
#define PI 3.14159265358979323846
#define NEAR 0.05
using namespace std;

void printScreen(char screen[HEIGHT][WIDTH]) {
    // printf("\n");
    // for(int i = WIDTH; i > 0; i--) {
    //         for(int j = 0; j < HEIGHT; j++) {
    //             printf("%c", screen[j][i]);
    //         }
    //         printf("\n");
    // }

    for(int i = 0; i < HEIGHT; i++) {
            for(int j = 0; j < WIDTH; j++) {
                printf("%c", screen[i][j]);
            }
            printf("\n");
    }
}

void clearScreen(char screen[HEIGHT][WIDTH]) {
  for(int i = 0; i < HEIGHT; i++) {
      for(int j = 0; j < WIDTH; j++) {
          screen[i][j] = ' ';
      }
  }
}

class camera {
    public:
        float x,y,z,b,a,g;
        float matrix[9];
        float fov;
        camera(float ix,float iy,float iz,float ipitch,float iyaw, float iroll, float ifov) {
            x=ix;y=iy;z=iz;b=ipitch;a=iyaw;g=iroll;fov=ifov;
            //rotation matrix from https://en.wikipedia.org/wiki/Rotation_matrix#General_rotations
            // matrix[0] = cos(a)*cos(b);
            // matrix[1] = (sin(a)*sin(b)*sin(g)) - (sin(a)*cos(g));
            // matrix[2] = (cos(a)*sin(b)*cos(g)) + (sin(a)*sin(g));
            // matrix[3] = sin(a)*cos(b);
            // matrix[4] = (sin(a)*sin(b)*sin(g)) + (cos(a)*cos(g));
            // matrix[5] = (sin(a)*sin(b)*cos(g)) - (cos(a)*sin(g));
            // matrix[6] = -sin(b);
            // matrix[7] = cos(b)*sin(g);
            // matrix[8] = cos(b)*cos(g);

            matrix[0] = (cos(a)*cos(b)) - (sin(a)*sin(b)*sin(g));
            matrix[1] = -sin(a)*cos(g);
            matrix[2] = (cos(a)*sin(b)) + (sin(a)*cos(b)*sin(g));
            matrix[3] = (sin(a)*cos(b)) + (cos(a)*sin(b)*sin(g));
            matrix[4] = cos(a)*cos(g);
            matrix[5] = (sin(a)*sin(b)) - (cos(a)*cos(b)*sin(g));
            matrix[6] = -sin(b)*cos(g);
            matrix[7] = sin(g);
            matrix[8] = cos(b)*cos(g);
        }
        //Covert worldpoint to point relative to camera, returns 0 if the resulting point can't be rendered
        int convertWorldToCam(float v[3], float camCoords[3]) {
            // printf("%f,%f,%f --->", v[0],v[1],v[2]);
            float temp[3];
            temp[0] = v[0] - x;
            temp[1] = v[1] - y;
            temp[2] = v[2] - z;
            camVectorMult(matrix,temp,camCoords);
            if (camCoords[2] < NEAR) {
                // printf("can't render point");
                return 0;
            }
            return 1;
            // printf("(%f,%f,%f) ---> (%f,%f,%f) ---> (%f,%f) ---> (%d,%d) \n", v[0],v[1],v[2],ou[0],ou[1],ou[2],Ix,Iy, screenCoords[0],screenCoords[1]);
        }
        //Convert point relative to camera to screen point
        void convertCamToScreen(float c[3], int screenCoords[2]) {
            float Ix = (-c[0]/c[2]) * fov;
            float Iy = (-c[1]/c[2]) * fov;
            screenCoords[0] = (int)(Ix * 5 + HEIGHT/2);
            screenCoords[1] = (int)(Iy * 5 + WIDTH/2);
        }

        void renderPoint(float vm[3], char c, char screen[HEIGHT][WIDTH]) {
            float v[3];
            int vs[2];
            convertWorldToCam(vm,v);
            convertCamToScreen(v,vs);
            drawPoint(screen, vs[0], vs[1], c);
        }

        void renderLine(float am[3], float bm[3], char c, char screen[HEIGHT][WIDTH]) {
            float a[3],b[3];
            int as[2],bs[2];
            convertWorldToCam(am,a);
            convertWorldToCam(bm,b);
            convertCamToScreen(b,bs);
            convertCamToScreen(a,as);
            // printf("(%f,%f): (%d,%d)---(%d,%d)   \n",x,z,a[0],a[1],b[0],b[1]);
            drawLine(screen,as,bs,c);
        }

        void renderTriangle(float am[3], float bm[3], float cm[3], char screen[HEIGHT][WIDTH], bool drawInterior) {
            // printf("rendering!");
            float a[3],b[3],c[3];
            int aIn,bIn,cIn;
            aIn = convertWorldToCam(am,a);
            bIn = convertWorldToCam(bm,b);
            cIn = convertWorldToCam(cm,c);
            if (aIn + bIn + cIn == 0) {
                return;
            }
            if (aIn + bIn + cIn == 1) {
                float* t;
                t = max({a,b,c},m3);
                for (float* g : {a,b,c}) {
                    if (t[2] != g[2]) {
                        float n[3];
                        lineSegmentPlaneIntercept(t,g,NEAR,n);
                        g[0]=n[0];g[1]=n[1];g[2]=n[2];
                    }
                }
            }
            if (aIn + bIn + cIn == 2) {
                float* t;
                t = min({a,b,c},m3);
                vector<float*> rest;
                //populate rest st if the clipped point is c, rest = {a,a',b,b'}
                for (float* g : {a,b,c}) {
                    if (t[2] != g[2]) {
                        float n[3];
                        lineSegmentPlaneIntercept(t,g,NEAR,n);
                        rest.push_back(g);
                        rest.push_back(n);
                    }
                }
                int as[2],asp[2],bs[2],bsp[2];
                convertCamToScreen(rest[0],as);
                convertCamToScreen(rest[1],asp);
                convertCamToScreen(rest[2],bs);
                convertCamToScreen(rest[3],bsp);
                drawTriangle(screen,as,asp,bs,'X','X',drawInterior,drawInterior);
                drawTriangle(screen,bs,bsp,asp,'X','X',drawInterior,drawInterior);
                drawLine(screen,as,asp,'*');
                drawLine(screen,as,bs,'*');
                drawLine(screen,bs,bsp,'*');
                return;
            }
            int as[2],bs[2],cs[2];
            convertCamToScreen(a,as);
            convertCamToScreen(b,bs);
            convertCamToScreen(c,cs);
            drawTriangle(screen,as,bs,cs,'X','*',drawInterior,true);
        }
};

int main()
{
  char screen[HEIGHT][WIDTH];
  clearScreen(screen);
  vector<array<array<float,3>,3>> triangles = objReader("plane2.obj");
//   for (array<array<float,3>,3> t : triangles) {
//     printf("--------\n");
//     for(int i = 0; i < 3; i++) {
//         printf("--(");
//         for (int j = 0; j < 3; j++) {
//             printf(" %f ",t[i][j]);
//         }
//         printf(")--");
//     }
//     printf("\n");
//   }
  float x = 0;
  float z = 0;
  float pitch = 0;
  float yaw = 0;
  float roll = 0;
  float fov = 6.8;
  printScreen(screen);
  char ch;
  system("stty raw");//seting the terminal in raw mode
  while(1) {
    ch=getchar();
    if(ch=='~') {          //terminate or come out of raw mode on "~" pressed
      system("stty cooked");
      //while(1);//you may still run the code 
      exit(0); //or terminate
    }
    system("stty cooked");
    clearScreen(screen);
    if (ch == 'w') {
        z+=0.1*cos(pitch);
        x-=0.1*sin(pitch);
    }
    if (ch == 's') {
        z-=0.1*cos(pitch);
        x+=0.1*sin(pitch);
    } 
    if (ch == 'a') {
        x-=0.1*cos(pitch);
        z-=0.1*sin(pitch);
    }
    if (ch == 'd') {
        x+=0.1*cos(pitch);
        z+=0.1*sin(pitch);
    }
    if (ch == 'q')
        pitch+= .03;
    if (ch == 'e')
        pitch-= .03;
    if (ch == 'r')
        yaw-= .03;
    if (ch == 'f')
        yaw+= .03;
    if (ch == 'i')
        roll-= .03;
    if (ch == 'o')
        roll+= .03;
    if (ch == 'n')
        fov-= .03;
    if (ch == 'm')
        fov+= .03;
    camera cam(x,1,z + 0.5,pitch,roll - PI/2,yaw,fov);
    // for (int i = 0; i < 9; i++) {
    //     printf("%f,", cam.matrix[i]);
    // }
    // printf(" ||| pitch: %f |||", pitch);
    // printf("\n");
    for (array<array<float,3>,3> tri : triangles)
        cam.renderTriangle(&tri[0][0],&tri[1][0],&tri[2][0],screen,false);
    printScreen(screen);
    // printf("\n %i, %i", a[0], a[1]);
    printf("\n");
    system("stty raw");
  }
  
}
