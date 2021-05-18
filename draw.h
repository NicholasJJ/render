#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <vector>
using namespace std;

#define HEIGHT 40
#define WIDTH 40

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

bool outOfView(int p[2]) {
    return (p[0] < 0 || p[1] < 0 || p[0] > HEIGHT - 1 || p[1] > WIDTH - 1);
}

void drawDepthLine(char screen[HEIGHT][WIDTH], float depth[HEIGHT][WIDTH], int am[2], float z0, int bm[2], float z1, char d, bool bump = true) {
    point p0((float)am[0],(float)am[1]);
    point p1((float)bm[0],(float)bm[1]);
    float N = diagonalDistance(p0,p1);
    float nudge = 0;
    if (bump)
        nudge = .0f;
    for (float step = 0; step <= N; step++) {
        float t = step / N;
        point ip = lerpPoint(p0, p1, t);
        float z = lerp(z0,z1,t);
        int x = round(ip.x);
        int y = round(ip.y);
        int xy[2] = {x,y};
        if (!outOfView(xy) && z >= depth[x][y] - nudge) {
            depth[x][y] = z;
            drawPoint(screen, x, y, d);
        }
    }
}

//arrays should be {x,y,1/z}
void drawDepthTriangle(char screen[HEIGHT][WIDTH], float depth[HEIGHT][WIDTH], int a[2], float az, int b[2], float bz, int c[2], float cz, char inChar, char edgeChar) {

    if (outOfView(a) && outOfView(b) && outOfView(c))
        return;

    //Sort points bottom to top so a.y <= b.y <= c.y
    if (b[1] < a[1])
        swapArray(a,&az,b,&bz);
    if (c[1] < a[1])
        swapArray(a,&az,c,&cz);
    if (c[1] < b[1])
        swapArray(c,&cz,b,&bz);

    printf("0:(%i,%i,%f) 1:(%i,%i,%f) 2:(%i,%i,%f)\n", a[0],a[1],az,b[0],b[1],bz,c[0],c[1],cz);

    //get z and x data for edges of triangle;
    vector<float> x01 = lerpLine(a[1], (float) a[0], b[1], (float) b[0]);
    vector<float> z01 = lerpLine(a[1], az, b[1], bz);

    vector<float> x12 = lerpLine(b[1], (float) b[0], c[1], (float) c[0]);
    vector<float> z12 = lerpLine(b[1], bz, c[1], cz);

    vector<float> x02 = lerpLine(a[1], (float) a[0], c[1], (float) c[0]);
    vector<float> z02 = lerpLine(a[1], az, c[1], cz);

    printf("x01:  ");
    for (int i = 0; i < x01.size(); i++)
        printf("%f  ", x01[i]);

    printf("\nx12:  ");
    for (int i = 0; i < x12.size(); i++)
        printf("%f  ", x12[i]);

    printf("\nx02:  ");
    for (int i = 0; i < x02.size(); i++)
        printf("%f  ", x02[i]);

    //make 01 the 012 line
    x01.pop_back();
    x01.insert(x01.end(), x12.begin(), x12.end());
    z01.pop_back();
    z01.insert(z01.end(), z12.begin(), z12.end());

    printf("\nx012:  ");
    for (int i = 0; i < x01.size(); i++)
        printf("%f  ", x01[i]);

    for (int y = a[1]; y <= c[1]; y++) {
        int p0x = round(x01[y - a[1]]);
        int p0[2] = {p0x, y};
        float p0z = z01[y - a[1]];
        int p1x = round(x02[y - a[1]]);
        int p1[2] = {p1x, y};
        float p1z = z02[y - a[1]];
        drawDepthLine(screen,depth,p0,p0z,p1,p1z,inChar, false);
    }
    
    // vector<float> xLline;
    // vector<float> xRline;
    // vector<float> zLline;
    // vector<float> zRline;
    // //determine if 012 of 02 has the lower x values
    // int m = x01.size()/2;
    // printf("\nm=%i\n",m);
    // if (x01[m] < x02[m]) {
    //     xLline = x01; zLline = z01; xRline = x02; zRline = z02;
    // } else {
    //     xRline = x01; zRline = z01; xLline = x02; zLline = z02;
    // }
    // printf("\nLeft:  ");
    // for (int i = 0; i < xLline.size(); i++)
    //     printf("%f  ", xLline[i]);
    // printf("\nRight:  ");
    // for (int i = 0; i < xRline.size(); i++)
    //     printf("%f  ", xRline[i]);
    // //for each y pixel, draw the horizontal line form xLline to xRline;
    // for (int y = a[1]; y <= c[1]; y++) {
    //     if (y < 0 || y > WIDTH)
    //         continue;
    //     printf("fl,fr = %f,%f",xLline[y - a[1]],xRline[y - a[1]]);
    //     int xl = round(xLline[y - a[1]]);
    //     int xr = round(xRline[y - a[1]]);
    //     printf("y= %i  l,r = %i,%i\n",y, xl,xr);
    //     vector<float> zhorizontal = lerpLine(xl, zLline[y - a[1]], xr, zRline[y - a[1]]);
    //     for (int x = xl; x <= xr; x++) {
    //         if (x < 0 || x > HEIGHT)
    //             continue;
    //         float z = zhorizontal[x - xl];
    //         // printf("x,y = %i,%i\n", x,y);
    //         if (z > depth[x][y]) {
    //             depth[x][y] = z;
    //             drawPoint(screen, x, y, inChar);
    //             // printf("2\n");
    //         } else {
    //             printf("occluded!!!!!!!!!!!!!!!!\n");
    //         }
    //     }
        
    // }

    //do edges
    drawDepthLine(screen,depth,a,az,b,bz,edgeChar);
    drawDepthLine(screen,depth,a,az,c,cz,edgeChar);
    drawDepthLine(screen,depth,c,cz,b,bz,edgeChar);
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