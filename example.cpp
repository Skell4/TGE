#include "TGE.hpp"

struct Vec3 {
    float x, y, z;
};

Vec3 rotateX(Vec3 p, float a) {
    float s = sin(a), c = cos(a);
    return {p.x, p.y * c - p.z * s, p.y * s + p.z * c};
}

Vec3 rotateY(Vec3 p, float a) {
    float s = sin(a), c = cos(a);
    return {p.x * c + p.z * s, p.y, -p.x * s + p.z * c};
}

Vec3 rotateZ(Vec3 p, float a) {
    float s = sin(a), c = cos(a);
    return {p.x * c - p.y * s, p.x * s + p.y * c, p.z};
}

Vec3 rotate(Vec3 p, float ax, float ay, float az) {
    p = rotateX(p, ax);
    p = rotateY(p, ay);
    p = rotateZ(p, az);
    return p;
}

Vec3 project(Vec3 p, int w, int h, float fov, float dist) {
    float factor = fov / (dist + p.z);
    return {
        p.x * factor + w / 2,
        p.y * factor + h / 2,
        p.z
    };
}

int main() {


    Canvas canvas(120, 60, "Cube 3D");

    canvas.sleep(1500);

    Vec3 cube[8] = {
        {-1, -1, -1}, {1, -1, -1},
        {1,  1, -1}, {-1,  1, -1},
        {-1, -1,  1}, {1, -1,  1},
        {1,  1,  1}, {-1,  1,  1}
    };

    int edges[12][2] = {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };

    float ax = 0, ay = 0, az = 0;

    while (true) {
        canvas.setBG(CONSOLE_BG_COLOR);

        Vec3 projected[8];

        for (int i = 0; i < 8; i++) {
            Vec3 r = rotate(cube[i], ax, ay, az);
            projected[i] = project(r, canvas.getWidth(), canvas.getHigh(), 40.0f, 3.0f);
        }

        for (auto &e : edges) {
            Vec3 p1 = projected[e[0]];
            Vec3 p2 = projected[e[1]];

            canvas.drawLine(
                (int)p1.x, (int)p1.y,
                (int)p2.x, (int)p2.y,
                1,
                Color::white
            );
        }

        canvas.print();

        ax += 0.03f;
        ay += 0.02f;
        az += 0.01f;

        Canvas::sleep(16);
    }
}