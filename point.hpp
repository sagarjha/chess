#pragma once

#include <cstdint>

class point {
public:
    int x;
    int y;

    point& operator++();
    point& operator--();
    bool valid();
    point add(const point other) const;

    void print();

    point();
    point(int x, int y);
};
