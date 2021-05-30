#include <cmath>
#include <array>
#include <math.h> 
#include <cstring>
using namespace std;

char palette[]=".,-~:;=!*#OB8%&";

float dotProduct(float a[], float b[]) {
    // int arrlength = sizeof(a)/sizeof(a[0]);
    float sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += a[i] * b[i];
    }
    return sum;
}

float dotProduct4(float a[], float b[]) {
    // int arrlength = sizeof(a)/sizeof(a[0]);
    float sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += a[i] * b[i];
    }
    return sum;
}

float length(float v[]) {
    return sqrt((v[0]*v[0]) + (v[1]*v[1]) + (v[2]*v[2]));
}

array<float,3> unit(array<float,3> v) {
    float l = length(&v[0]);
    return {v[0]/l,v[1]/l,v[2]/l};
}

array<float,3> AminusB(array<float,3> a, array<float,3> b) {
    return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}

bool normFacingCamera(array<float,3> normal, array<float,3> triVertex, array<float,3> camPos) {
    //first, get the vector going from triVertex to camPos
    array<float,3> v = AminusB(camPos,triVertex);
    if (dotProduct(&normal[0],&v[0]) <= 0) {
        return false;
    } else {
        return true;
    }
}

// void camVectorMult(float m[12], float v[3], float out[3]) {
//     out[0] = v[0]*m[0] + v[1]*m[1] + v[2]*m[2] + m[3];
//     out[1] = v[0]*m[4] + v[1]*m[5] + v[2]*m[6] + m[7];
//     out[2] = v[0]*m[8] + v[1]*m[9] + v[2]*m[10] + m[11];
// }

//asume v[3] = 1 and m will set out[3] = 1;
void camVectorMult(float m[16], float v[3], float out[3]) {
    out[0] = v[0]*m[0] + v[1]*m[1] + v[2]*m[2] + m[3];
    out[1] = v[0]*m[4] + v[1]*m[5] + v[2]*m[6] + m[7];
    out[2] = v[0]*m[8] + v[1]*m[9] + v[2]*m[10] + m[11];
}

//the intersection of the line segment a-b and the near clipping plane at z=d
void lineSegmentPlaneIntercept(float a[3], float b[3], float d, float out[3]) {
    float c = (a[2]-d)/(a[2]-b[2]);
    out[0] = (c*(b[0]-a[0]))+a[0];
    out[1] = (c*(b[1]-a[1]))+a[1];
    out[2] = d;

}

//both vectors must be unit vectors
char luminance(array<float,3> normal, array<float,3> light) {
    float l = clamp(dotProduct(&normal[0],&light[0]),(float)0,(float)1);
    int c = (int)(l * (float)(strlen(palette)-1));
    return palette[c];
}

bool m3(float a[3], float b[3]) {
    return (a[2] < b[2]);
}

void matMult(float res[16], const float left[16], const float right[16]) {
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            int index = (4*row) + col;
            float a[4], b[4];
            for (int i = 0; i < 4; i++) {
                a[i] = left[(4*row) + i];
                b[i] = right[(4*i) + col];
            }
            res[index] = dotProduct4(a,b);
        }
    }
}

//Code taken from DinoZ1729
void invert(float inv[16],const float matrix[16])
{
	float det;
    int i;

    inv[0] = matrix[5]  * matrix[10] * matrix[15] - 
             matrix[5]  * matrix[11] * matrix[14] - 
             matrix[9]  * matrix[6]  * matrix[15] + 
             matrix[9]  * matrix[7]  * matrix[14] +
             matrix[13] * matrix[6]  * matrix[11] - 
             matrix[13] * matrix[7]  * matrix[10];

    inv[4] = -matrix[4]  * matrix[10] * matrix[15] + 
              matrix[4]  * matrix[11] * matrix[14] + 
              matrix[8]  * matrix[6]  * matrix[15] - 
              matrix[8]  * matrix[7]  * matrix[14] - 
              matrix[12] * matrix[6]  * matrix[11] + 
              matrix[12] * matrix[7]  * matrix[10];

    inv[8] = matrix[4]  * matrix[9] * matrix[15] - 
             matrix[4]  * matrix[11] * matrix[13] - 
             matrix[8]  * matrix[5] * matrix[15] + 
             matrix[8]  * matrix[7] * matrix[13] + 
             matrix[12] * matrix[5] * matrix[11] - 
             matrix[12] * matrix[7] * matrix[9];

    inv[12] = -matrix[4]  * matrix[9] * matrix[14] + 
               matrix[4]  * matrix[10] * matrix[13] +
               matrix[8]  * matrix[5] * matrix[14] - 
               matrix[8]  * matrix[6] * matrix[13] - 
               matrix[12] * matrix[5] * matrix[10] + 
               matrix[12] * matrix[6] * matrix[9];

    inv[1] = -matrix[1]  * matrix[10] * matrix[15] + 
              matrix[1]  * matrix[11] * matrix[14] + 
              matrix[9]  * matrix[2] * matrix[15] - 
              matrix[9]  * matrix[3] * matrix[14] - 
              matrix[13] * matrix[2] * matrix[11] + 
              matrix[13] * matrix[3] * matrix[10];

    inv[5] = matrix[0]  * matrix[10] * matrix[15] - 
             matrix[0]  * matrix[11] * matrix[14] - 
             matrix[8]  * matrix[2] * matrix[15] + 
             matrix[8]  * matrix[3] * matrix[14] + 
             matrix[12] * matrix[2] * matrix[11] - 
             matrix[12] * matrix[3] * matrix[10];

    inv[9] = -matrix[0]  * matrix[9] * matrix[15] + 
              matrix[0]  * matrix[11] * matrix[13] + 
              matrix[8]  * matrix[1] * matrix[15] - 
              matrix[8]  * matrix[3] * matrix[13] - 
              matrix[12] * matrix[1] * matrix[11] + 
              matrix[12] * matrix[3] * matrix[9];

    inv[13] = matrix[0]  * matrix[9] * matrix[14] - 
              matrix[0]  * matrix[10] * matrix[13] - 
              matrix[8]  * matrix[1] * matrix[14] + 
              matrix[8]  * matrix[2] * matrix[13] + 
              matrix[12] * matrix[1] * matrix[10] - 
              matrix[12] * matrix[2] * matrix[9];

    inv[2] = matrix[1]  * matrix[6] * matrix[15] - 
             matrix[1]  * matrix[7] * matrix[14] - 
             matrix[5]  * matrix[2] * matrix[15] + 
             matrix[5]  * matrix[3] * matrix[14] + 
             matrix[13] * matrix[2] * matrix[7] - 
             matrix[13] * matrix[3] * matrix[6];

    inv[6] = -matrix[0]  * matrix[6] * matrix[15] + 
              matrix[0]  * matrix[7] * matrix[14] + 
              matrix[4]  * matrix[2] * matrix[15] - 
              matrix[4]  * matrix[3] * matrix[14] - 
              matrix[12] * matrix[2] * matrix[7] + 
              matrix[12] * matrix[3] * matrix[6];

    inv[10] = matrix[0]  * matrix[5] * matrix[15] - 
              matrix[0]  * matrix[7] * matrix[13] - 
              matrix[4]  * matrix[1] * matrix[15] + 
              matrix[4]  * matrix[3] * matrix[13] + 
              matrix[12] * matrix[1] * matrix[7] - 
              matrix[12] * matrix[3] * matrix[5];

    inv[14] = -matrix[0]  * matrix[5] * matrix[14] + 
               matrix[0]  * matrix[6] * matrix[13] + 
               matrix[4]  * matrix[1] * matrix[14] - 
               matrix[4]  * matrix[2] * matrix[13] - 
               matrix[12] * matrix[1] * matrix[6] + 
               matrix[12] * matrix[2] * matrix[5];

    inv[3] = -matrix[1] * matrix[6] * matrix[11] + 
              matrix[1] * matrix[7] * matrix[10] + 
              matrix[5] * matrix[2] * matrix[11] - 
              matrix[5] * matrix[3] * matrix[10] - 
              matrix[9] * matrix[2] * matrix[7] + 
              matrix[9] * matrix[3] * matrix[6];

    inv[7] = matrix[0] * matrix[6] * matrix[11] - 
             matrix[0] * matrix[7] * matrix[10] - 
             matrix[4] * matrix[2] * matrix[11] + 
             matrix[4] * matrix[3] * matrix[10] + 
             matrix[8] * matrix[2] * matrix[7] - 
             matrix[8] * matrix[3] * matrix[6];

    inv[11] = -matrix[0] * matrix[5] * matrix[11] + 
               matrix[0] * matrix[7] * matrix[9] + 
               matrix[4] * matrix[1] * matrix[11] - 
               matrix[4] * matrix[3] * matrix[9] - 
               matrix[8] * matrix[1] * matrix[7] + 
               matrix[8] * matrix[3] * matrix[5];

    inv[15] = matrix[0] * matrix[5] * matrix[10] - 
              matrix[0] * matrix[6] * matrix[9] - 
              matrix[4] * matrix[1] * matrix[10] + 
              matrix[4] * matrix[2] * matrix[9] + 
              matrix[8] * matrix[1] * matrix[6] - 
              matrix[8] * matrix[2] * matrix[5];

    det = matrix[0] * inv[0] + matrix[1] * inv[4] + matrix[2] * inv[8] + matrix[3] * inv[12];

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        inv[i] *= det;
}

