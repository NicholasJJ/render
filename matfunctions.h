#include <cmath>

float dotProduct(float a[], float b[]) {
    // int arrlength = sizeof(a)/sizeof(a[0]);
    float sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += a[i] * b[i];
    }
    return sum;
}

// void camVectorMult(float m[12], float v[3], float out[3]) {
//     out[0] = v[0]*m[0] + v[1]*m[1] + v[2]*m[2] + m[3];
//     out[1] = v[0]*m[4] + v[1]*m[5] + v[2]*m[6] + m[7];
//     out[2] = v[0]*m[8] + v[1]*m[9] + v[2]*m[10] + m[11];
// }

void camVectorMult(float m[12], float v[3], float out[3]) {
    out[0] = v[0]*m[0] + v[1]*m[1] + v[2]*m[2];
    out[1] = v[0]*m[3] + v[1]*m[4] + v[2]*m[5];
    out[2] = v[0]*m[6] + v[1]*m[7] + v[2]*m[8];
}

//the intersection of the line segment a-b and the near clipping plane at z=d
void lineSegmentPlaneIntercept(float a[3], float b[3], float d, float out[3]) {
    float c = (d-a[2])/(a[2]-b[2]);
    out[0] = (c*(a[0]-b[0]))+a[0];
    out[1] = (c*(a[1]-b[1]))+a[1];
    out[2] = d;
}

bool m3(float a[3], float b[3]) {
    return (a[2] < b[2]);
}

