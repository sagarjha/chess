#include "point.hpp"

point& point::operator++() {
    if(y == 7) {
        x = x + 1;
        y = 0;
    } else {
        y = y + 1;
    }
    return *this;
}

bool point::valid() {
    return x >= 0 && x < 8 && y >= 0 && y < 8;
}

point::point(int x, int y)
        : x(x),
          y(y) {
}

point point::add(const point other) const{
    return {x + other.x, y + other.y};
}
