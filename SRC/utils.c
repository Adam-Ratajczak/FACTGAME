#include "utils.h"

int div_floor(int a, int b) {
    int q = a / b;
    int r = a % b;
    return (r && a < 0) ? (q - 1) : q;
}

int mod_floor(int a, int b) {
    int r = a % b;
    return (r < 0) ? (r + b) : r;
}

int normalize_rotation(int rotation)
{
    rotation %= 360;
    if (rotation < 0)
        rotation += 360;

    return rotation;
}
