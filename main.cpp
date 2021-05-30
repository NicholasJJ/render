#include <iostream>
#include <stdio.h>
#include <array>
#include <list>
#include <vector>
#include <algorithm>
#include "draw.h"

#include "objReader.h"
#include <math.h>
#include <unordered_map>
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

void initializeDepth(float d[HEIGHT][WIDTH]) {
    for(int i = 0; i < HEIGHT; i++) {
      for(int j = 0; j < WIDTH; j++) {
          d[i][j] = 0;
      }
  }
}

class camera {
    public:
        float x,y,z,b,a,g;
        float matrix[16];
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
            matrix[4] = (sin(a)*cos(b)) + (cos(a)*sin(b)*sin(g));
            matrix[5] = cos(a)*cos(g);
            matrix[6] = (sin(a)*sin(b)) - (cos(a)*cos(b)*sin(g));
            matrix[8] = -sin(b)*cos(g);
            matrix[9] = sin(g);
            matrix[10] = cos(b)*cos(g);
            matrix[3] = -(matrix[0]*x + matrix[1]*y + matrix[2]*z);
            matrix[7] = -(matrix[4]*x + matrix[5]*y + matrix[6]*z);
            matrix[11] = -(matrix[8]*x + matrix[9]*y + matrix[10]*z);
            matrix[12] = 0;
            matrix[13] = 0;
            matrix[14] = 0;
            matrix[15] = 1;
        }
        //Covert worldpoint to point relative to camera, returns 0 if the resulting point can't be rendered
        int convertWorldToCam(float v[3], float camCoords[3]) {
            // printf("%f,%f,%f --->", v[0],v[1],v[2]);
            // float temp[3];
            // temp[0] = v[0] - x;
            // temp[1] = v[1] - y;
            // temp[2] = v[2] - z;
            camVectorMult(matrix,v,camCoords);
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
            float Iy = (c[1]/c[2]) * fov;
            screenCoords[0] = (int)(Ix * 5 + HEIGHT/2);
            screenCoords[1] = (int)(Iy * 5 + WIDTH/2);
        }

        //Scrren should be screenX,screenY, 1/camZ;
        void convertScreenToCam(float screen[3], float world[3]) {
            float Ix = (screen[0] - (HEIGHT/2))/5;
            float Iy = (screen[1] - (WIDTH/2))/5;
            world[0] = -(Ix/fov)/screen[2];
            world[1] = (Iy/fov)/screen[2];
            world[2] = 1/screen[2];
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

        vector<array<float,3>> renderTriangle(float am[3], float bm[3], float cm[3], char screen[HEIGHT][WIDTH], bool drawInterior, float depth[HEIGHT][WIDTH], char color) {
            // printf("rendering!");
            float a[3],b[3],c[3];
            int aIn,bIn,cIn;
            aIn = convertWorldToCam(am,a);
            bIn = convertWorldToCam(bm,b);
            cIn = convertWorldToCam(cm,c);
            // float al[3],bl[3],cl[3];
            // LC.convertWorldToCam(am,al);
            // LC.convertWorldToCam(bm,bl);
            // LC.convertWorldToCam(cm,cl);
            if (aIn + bIn + cIn == 0) {
                vector<array<float,3>> nothing;
                return nothing;
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
                float asz,aspz,bsz,bspz;
                asz = rest[0][2];
                aspz = rest[1][2];
                bsz = rest[2][2];
                bspz = rest[3][2];
                convertCamToScreen(rest[0],as);
                convertCamToScreen(rest[1],asp);
                convertCamToScreen(rest[2],bs);
                convertCamToScreen(rest[3],bsp);
                // drawTriangle(screen,as,asp,bs,'X','X',drawInterior,drawInterior);
                // drawTriangle(screen,bs,bsp,asp,'X','X',drawInterior,drawInterior);
                // drawLine(screen,as,asp,'*');
                // drawLine(screen,as,bs,'*');
                // drawLine(screen,bs,bsp,'*');

                vector<array<float,3>> t1 = drawDepthTriangle(screen,depth,as,1/asz,asp,1/aspz,bs,1/bspz,color,color);
                vector<array<float,3>> t2 = drawDepthTriangle(screen,depth,bs,1/bsz,bsp,1/bspz,asp,1/aspz,color,color);
                t1.insert(t1.end(),t2.begin(),t2.end());

                return t1;
            }
            int as[2],bs[2],cs[2];
            convertCamToScreen(a,as);
            convertCamToScreen(b,bs);
            convertCamToScreen(c,cs);
            // drawTriangle(screen,as,bs,cs,'X','*',drawInterior,true);
            return drawDepthTriangle(screen, depth, as, 1/a[2], bs, 1/b[2], cs, 1/c[2],color,color);
        }

        vector<array<float,3>> renderOrthoTriangle(float am[3], float bm[3], float cm[3]) {
            float a[3],b[3],c[3];
            convertWorldToCam(am,a);
            convertWorldToCam(bm,b);
            convertWorldToCam(cm,c);
            return addTriToDist(a,b,c);
        }

        array<float,3> position() {
            return {x,y,z};
        }

        void SetAsCameraMatrix(float m[16]) {
            for (int i = 0; i< 16; i++) {
                m[i] = matrix[i];
            }
            return;
        }
};

void change(unordered_map<int, float> test) {
    printf("\n %f \n", test[1]);
    return;
}

void print4x4(float m[16]) {
    printf("\n");
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            printf("%f  ",m[(4*row)+col]);
        }
        printf("\n");
    }
}

int main(int argc, char* argv[])
{
  printf("start");

//   unordered_map<int, float> test;
//   test[1] = 5;
//   change(test);
  
//   return 1;





  char screen[HEIGHT][WIDTH];
  clearScreen(screen);
  float depth[HEIGHT][WIDTH];
  initializeDepth(depth);
  string fileName;
  if (argc == 1) {
      fileName = "plane2.obj";
  } else {
      fileName = argv[1];
  }
  printf("%s",fileName.c_str());
  vector<array<array<float,3>,4>> triangles = objReader(fileName);

  array<float, 3> light = {-1,1,1};
  array<float, 3> unitLight = unit(light);

  //Light Triangles
  char lit[triangles.size()];
  for (int i = 0; i < triangles.size(); i++) {
    array<float, 3> unitNorm = unit(triangles[i][3]);
    lit[i] = luminance(unitNorm,unitLight);
  }

  //construct light distance array
  float lightMat[16];
  unordered_map<int, unordered_map<int,float>> distToL; 
  float lrAngle = atan(light[0]/light[2]);
  float temp = sqrt((light[0]*light[0]) + (light[2]*light[2]));
  float udAngle = atan(light[1]/temp);
  lrAngle *= -1;
//   udAngle += PI;
  camera lightCam(0,0,0,lrAngle,0,udAngle,1);
  lightCam.SetAsCameraMatrix(lightMat);
  for (array<array<float,3>,4> tri : triangles) {
    //   printf("returned");
      vector<array<float,3>> n = lightCam.renderOrthoTriangle(&tri[0][0],&tri[1][0],&tri[2][0]);
      for (array<float,3> point : n) {
          int px = round(point[0]);
          int py = round(point[1]);
          float pz = point[2];
          if (distToL[px][py] == 0 || distToL[px][py] < pz) {
            // printf("Set %i,%i to %f",x,y,z);
            distToL[px][py] = pz;
            // printf(" it is now %f     ", m[x][y]);
          } else {
            // printf("%f remain the champ", m[x][y]);
          }
      }
  }
  printf("%f",distToL[-62][-36]);


  float a[3] = {0,0,8};
  float b[3] = {2,0,8};
  float c[3] = {-2,2,8};

  float x = 0;
  float z = 0;
  float pitch = 0;
  float yaw = 0;
  float roll = 0;
  float fov = 6.8;
  float teh = 0;
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
    initializeDepth(depth);
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
    if (ch == '9')
        teh+= 1;
    if (ch == '0')
        teh-= 1;

    camera cam(x,1,z,pitch,roll - PI/2,yaw,fov);
    float camMat[16];
    cam.SetAsCameraMatrix(camMat);
    float camMatInv[16];
    invert(camMatInv,camMat);
    float camSpaceToLightSpace[16];
    matMult(camSpaceToLightSpace,lightMat,camMatInv);
    
    // printf(" ||| fov: %f |||", fov);
    printf("\n");

    // print4x4(camMat);
    // print4x4(camMatInv);
    //Render Triangles
    array<float,3> camPos = cam.position();
    for (int i = 0; i < triangles.size(); i++) {
        array<array<float,3>,4> tri = triangles[i];
        if (normFacingCamera(tri[3],tri[0],camPos)) {
            vector<array<float,3>> p = cam.renderTriangle(&tri[0][0],&tri[1][0],&tri[2][0],screen,false, depth, lit[i]);
            for (array<float,3> pix : p) {
                float relativeCamPos[3];
                cam.convertScreenToCam(&pix[0],relativeCamPos);
                float worldPos[3];
                // float temp;
                // temp = relativeCamPos[0]; relativeCamPos[0] = relativeCamPos[1]; relativeCamPos[1] = temp;
                camVectorMult(camMatInv,relativeCamPos,worldPos);
                char ch = '.';
                // if (relativeCamPos[2] > teh && relativeCamPos[1] > teh) {
                //     ch = '!';
                // } else if (relativeCamPos[2] > teh) {
                //     ch = '?';
                // } else if (relativeCamPos[1] > teh) {
                //     ch = '&';
                // }

                // check if in light
                float lightCoords[3];
                int roundedLightCoords[2];
                camVectorMult(lightMat,worldPos,lightCoords);
                for (int i = 0; i < 2; i++) {
                    roundedLightCoords[i] = round(lightCoords[i] * SHADOWRESOLUTION);
                }
                if (distToL[roundedLightCoords[0]][roundedLightCoords[1]] <= lightCoords[2] +.5 ) {
                    ch = lit[i];
                }


                if (true) {
                    // if (pix[1] > HEIGHT/2)
                    //     ch = '?';
                    drawPoint(screen,round(pix[0]),round(pix[1]),ch);
                }
            }
        } 
    }

    // for (array<array<float,3>,4> tri : triangles) {
    //     if (normFacingCamera(tri[3],tri[0],camPos)) {
    //         cam.renderTriangle(&tri[0][0],&tri[1][0],&tri[2][0],screen,false, depth,);
    //     }  
    // }
    // cam.renderTriangle(a,b,c,screen,false,depth);
    
    printScreen(screen);
    // printf("\n %i, %i", a[0], a[1]);
    printf("\n");
    system("stty raw");
  }
  
}

