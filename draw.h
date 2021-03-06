#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <vector>
#include <unordered_map>
#include "matfunctions.h"
using namespace std;

#define HEIGHT 68
#define WIDTH 235

#define SHADOWRESOLUTION 2

void drawPoint(char screen[HEIGHT][WIDTH], int x, int y, char c) {
    if (x < 0 || y < 0 || x >= HEIGHT || y >= WIDTH)
        return;
    screen[x][y] = c;
}

class point {
    public: float x, y;
    point(float xi, float yi) {
        x = xi;
        y = yi;
    }
};

float lerp(float start, float end, float t) {
    return start + (t*(end-start));
}

point lerpPoint(point a, point b, float t) {
    return point(lerp(a.x, b.x, t), lerp(a.y, b.y, t));
}

float diagonalDistance(point a, point b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return fmax(abs(dx),abs(dy));
}

void drawLine(char screen[HEIGHT][WIDTH], int am[2], int bm[2], char c) {
    point p0((float)am[0],(float)am[1]);
    point p1((float)bm[0],(float)bm[1]);
    float N = diagonalDistance(p0,p1);
    for (float step = 0; step <= N; step++) {
        float t = step / N;
        point ip = lerpPoint(p0, p1, t);
        drawPoint(screen, round(ip.x), round(ip.y), c);
    }
}

//apparently this is twice the area, shouldn't affect comparisons
float triangleArea(point a, point b, point c) {
    return ((a.x - c.x) * (b.y - c.y)) - ((b.x - c.x) * (a.y - c.y));
}

bool pointInTriangle(point p, point v1, point v2, point v3) {
    float d1, d2, d3;
    bool neg,pos;
    d1 = triangleArea(p,v1,v2);
    d2 = triangleArea(p,v2,v3);
    d3 = triangleArea(p,v3,v1);
    //point is in the triangle it's on the same side of each line, which is also the area of the 
    //triangle made with that line.
    neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
    return !(neg&&pos);
}

void drawTriangleInterior(char screen[HEIGHT][WIDTH], int am[2], int bm[2], int cm[2], char d) {
    point a((float)am[0],(float)am[1]);
    point b((float)bm[0],(float)bm[1]);
    point c((float)cm[0],(float)cm[1]);
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            point p((float)i + 0.5f, (float)j + 0.5f);
            if (pointInTriangle(p,a,b,c))
                drawPoint(screen,i,j,d);
        }
    }
}

void drawTriangle(char screen[HEIGHT][WIDTH], int am[2], int bm[2], int cm[2], char inChar, char edgeChar, bool fill, bool edge) {
    if (fill) {
        drawTriangleInterior(screen, am, bm, cm, inChar);
    }
    if (edge) {
        drawLine(screen,am,bm,edgeChar);
        drawLine(screen,bm,cm,edgeChar);
        drawLine(screen,am,cm,edgeChar);
    }
}

//Returns values of integer points on line from (i0,d0) to (i1,d1), assumes i0 < i1;
vector<float> lerpLine(int i0, float d0, int i1, float d1) {
    vector<float> vec;
    if (i0 == i1) {
        vec.push_back(d0);
        return vec;
    }
    float a = (d1 - d0) / ((float)(i1 - i0));
    float d = d0;
    for (int i = i0; i <= i1; i++) {
        vec.push_back(d);
        d += a;
    }
    return vec;
}

void swapArray(int a[2], float* az, int b[2], float* bz) {
    int t;
    float tz;
    t = a[0]; a[0] = b[0]; b[0] = t;
    t = a[1]; a[1] = b[1]; b[1] = t;
    tz = *az; *az = *bz; *bz = tz;
}

void swapFloatArray(float a[3], float b[3]) {
    float t;
    t = a[0]; a[0] = b[0]; b[0] = t;
    t = a[1]; a[1] = b[1]; b[1] = t;
    t = a[2]; a[2] = b[2]; b[2] = t;
}

bool outOfView(int p[2]) {
    return (p[0] < 0 || p[1] < 0 || p[0] > HEIGHT - 1 || p[1] > WIDTH - 1);
}

bool left(int a[2], int b[2], int c[2]) {
    return (a[1] < 0 && b[1] < 0 && c[1] < 0);
}

bool right(int a[2], int b[2], int c[2]) {
    return (a[1] > WIDTH - 1 && b[1] > WIDTH - 1 && c[1] > WIDTH - 1);
}

bool top(int a[2], int b[2], int c[2]) {
    return (a[0] > HEIGHT - 1 && b[0] > HEIGHT - 1 && c[0] > HEIGHT - 1);
}

bool bottom(int a[2], int b[2], int c[2]) {
    return (a[0] < 0 && b[0] < 0 && c[0] < 0);
}

bool inLight(int screenX, int screenY, float invZFromCam, float c2l[16], unordered_map<int, unordered_map<int,float>> l2h) {
    //convert screen to cam coords, since z is really 1/z we divide instead of multiply
    float camCoords[3];
    camCoords[0] = ((float)screenX) / invZFromCam;
    camCoords[1] = ((float)screenY) / invZFromCam;
    camCoords[2] = 1/invZFromCam;
    //Convert cam coords to light coords
    float lightCoords[3];
    int roundedLightCoords[2];
    camVectorMult(c2l,camCoords,lightCoords);
    for (int i = 0; i < 2; i++) {
        roundedLightCoords[i] = round(lightCoords[i] * SHADOWRESOLUTION);
    }
    //Check if point has highest z
    printf("%f     ", l2h[roundedLightCoords[0]][roundedLightCoords[1]]);
    if (l2h[roundedLightCoords[0]][roundedLightCoords[1]] <= lightCoords[2]) {
        return true;
    } else {
        return false;
    }
}

vector<array<float,3>> drawDepthLine(char screen[HEIGHT][WIDTH], float depth[HEIGHT][WIDTH], int am[2], float z0, int bm[2], float z1, char d, bool bump = true) {
    point p0((float)am[0],(float)am[1]);
    point p1((float)bm[0],(float)bm[1]);
    float N = diagonalDistance(p0,p1);
    float nudge = 0;
    if (bump)
        nudge = .01f;
    vector<array<float,3>> ret;
    for (float step = 0; step <= N; step++) {
        float t = step / N;
        point ip = lerpPoint(p0, p1, t);
        float z = lerp(z0,z1,t);
        int x = round(ip.x);
        int y = round(ip.y);
        int xy[2] = {x,y};
        if (!outOfView(xy) && z >= depth[x][y] - nudge) {
            depth[x][y] = z;
            // if (!inLight(x,y,z,c2l,l2h)) {
            //     d = '.';
            // }
            // drawPoint(screen, x, y, d);
            ret.push_back({(float)x,(float)y,z});
        }
    }
    return ret;
}

//arrays should be {x,y,1/z}
vector<array<float,3>> drawDepthTriangle(char screen[HEIGHT][WIDTH], float depth[HEIGHT][WIDTH], int a[2], float az, int b[2], float bz, int c[2], float cz, char inChar, char edgeChar) {

    //culling partial triangles doesn't speed up much (and it might create 2 triangles), so this was is simpler and is probably good enough.
    vector<array<float,3>> pixels;
    if (top(a,b,c) || bottom(a,b,c) || left(a,b,c) || right(a,b,c))
        return pixels;

    //Sort points bottom to top so a.y <= b.y <= c.y
    if (b[1] < a[1])
        swapArray(a,&az,b,&bz);
    if (c[1] < a[1])
        swapArray(a,&az,c,&cz);
    if (c[1] < b[1])
        swapArray(c,&cz,b,&bz);

    // printf("0:(%i,%i,%f) 1:(%i,%i,%f) 2:(%i,%i,%f)\n", a[0],a[1],az,b[0],b[1],bz,c[0],c[1],cz);

    //get z and x data for edges of triangle;
    vector<float> x01 = lerpLine(a[1], (float) a[0], b[1], (float) b[0]);
    vector<float> z01 = lerpLine(a[1], az, b[1], bz);

    vector<float> x12 = lerpLine(b[1], (float) b[0], c[1], (float) c[0]);
    vector<float> z12 = lerpLine(b[1], bz, c[1], cz);

    vector<float> x02 = lerpLine(a[1], (float) a[0], c[1], (float) c[0]);
    vector<float> z02 = lerpLine(a[1], az, c[1], cz);

    // printf("x01:  ");
    // for (int i = 0; i < x01.size(); i++)
    //     printf("%f  ", x01[i]);

    // printf("\nx12:  ");
    // for (int i = 0; i < x12.size(); i++)
    //     printf("%f  ", x12[i]);

    // printf("\nx02:  ");
    // for (int i = 0; i < x02.size(); i++)
    //     printf("%f  ", x02[i]);

    //make 01 the 012 line
    x01.pop_back();
    x01.insert(x01.end(), x12.begin(), x12.end());
    z01.pop_back();
    z01.insert(z01.end(), z12.begin(), z12.end());

    // printf("\nx012:  ");
    // for (int i = 0; i < x01.size(); i++)
    //     printf("%f  ", x01[i]);
    
    for (int y = a[1]; y <= c[1]; y++) {
        int p0x = round(x01[y - a[1]]);
        int p0[2] = {p0x, y};
        float p0z = z01[y - a[1]];
        int p1x = round(x02[y - a[1]]);
        int p1[2] = {p1x, y};
        float p1z = z02[y - a[1]];
        vector<array<float,3>> n = drawDepthLine(screen,depth,p0,p0z,p1,p1z,inChar,false);
        pixels.insert(pixels.end(),n.begin(),n.end());
    }
    return pixels;

    //do edges
    // drawDepthLine(screen,depth,a,az,b,bz,edgeChar,c2l,l2h);
    // drawDepthLine(screen,depth,a,az,c,cz,edgeChar,c2l,l2h);
    // drawDepthLine(screen,depth,c,cz,b,bz,edgeChar,c2l,l2h);
}

vector<array<float,3>> addLineToDist(int am[2], float z0, int bm[2], float z1) {
    vector<array<float,3>> ret;
    point p0((float)am[0],(float)am[1]);
    point p1((float)bm[0],(float)bm[1]);
    float N = diagonalDistance(p0,p1);
    for (float step = 0; step <= N; step++) {
        float t = step / N;
        point ip = lerpPoint(p0, p1, t);
        float z = lerp(z0,z1,t);
        int x = round(ip.x);
        int y = round(ip.y);
        ret.push_back({ip.x,ip.y,z});
        // if (m[x][y] == 0 || m[x][y] < z) {
        //     // printf("Set %i,%i to %f",x,y,z);
        //     m[x][y] = z;
        //     // printf(" it is now %f     ", m[x][y]);
        // } else {
        //     // printf("%f remain the champ", m[x][y]);
        // }
        
    }
    return ret;
}

vector<array<float,3>> addTriToDist(float af[3], float bf[3], float cf[3]) {
    vector<array<float,3>> ret;
    //Convert float cordinates to ints;
    int ax = round(af[0] * SHADOWRESOLUTION);
    int ay = round(af[1] * SHADOWRESOLUTION);
    int a[2] = {ax,ay};
    float az = af[2];
    int bx = round(bf[0] * SHADOWRESOLUTION);
    int by = round(bf[1] * SHADOWRESOLUTION);
    int b[2] = {bx,by};
    float bz = bf[2];
    int cx = round(cf[0] * SHADOWRESOLUTION);
    int cy = round(cf[1] * SHADOWRESOLUTION);
    int c[2] = {cx,cy};
    float cz = cf[2];

    //Sort points bottom to top so a.y <= b.y <= c.y
    if (b[1] < a[1])
        swapArray(a,&az,b,&bz);
    if (c[1] < a[1])
        swapArray(a,&az,c,&cz);
    if (c[1] < b[1])
        swapArray(c,&cz,b,&bz);
    
    //get z and x data for edges of triangle;
    vector<float> x01 = lerpLine(a[1], (float) a[0], b[1], (float) b[0]);
    vector<float> z01 = lerpLine(a[1], az, b[1], bz);

    vector<float> x12 = lerpLine(b[1], (float) b[0], c[1], (float) c[0]);
    vector<float> z12 = lerpLine(b[1], bz, c[1], cz);

    vector<float> x02 = lerpLine(a[1], (float) a[0], c[1], (float) c[0]);
    vector<float> z02 = lerpLine(a[1], az, c[1], cz);

    //make 01 the 012 line
    x01.pop_back();
    x01.insert(x01.end(), x12.begin(), x12.end());
    z01.pop_back();
    z01.insert(z01.end(), z12.begin(), z12.end());

    //render horizontal lines from 012 to 01
    for (int y = a[1]; y <= c[1]; y++) {
        int p0x = round(x01[y - a[1]]);
        int p0[2] = {p0x, y};
        float p0z = z01[y - a[1]];
        int p1x = round(x02[y - a[1]]);
        int p1[2] = {p1x, y};
        float p1z = z02[y - a[1]];
        vector<array<float,3>> n = addLineToDist(p0,p0z,p1,p1z);
        ret.insert(ret.end(),n.begin(),n.end());
    }
    return ret;
}

void moveDot(char screen[HEIGHT][WIDTH], int p[4]) {
    screen[p[0]][p[1]] = '*';
    if (p[0] + p[2] >= WIDTH || p[0] + p[2] < 0)
        p[2] = -p[2];
    if (p[1] + p[3] >= HEIGHT || p[1] + p[3] < 0)
        p[3] = -p[3];
    p[0] += p[2];
    p[1] += p[3];
    screen[p[0]][p[1]] = '0';
}
