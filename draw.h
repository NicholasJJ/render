#include <iostream>
#include <stdlib.h>
#include <cmath>

#define HEIGHT 40
#define WIDTH 140

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