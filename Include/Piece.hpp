#pragma once
#include "Position.hpp"

class Piece
{
public:
    int id;
    Position pos;
    bool forward;
    bool finished;
    Piece(int id = 0);
    void move(int distance);
    void reset();
};