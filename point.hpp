#pragma once

#include <cstdint>

class point {
public:
    int x;
    int y;

    point& operator++();
    bool valid();
    point add(const point other) const;

    point(int x, int y);
};
