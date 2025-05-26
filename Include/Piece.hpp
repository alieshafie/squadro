#pragma once

class Piece
{
public:
    int id;
    int pos;
    int point;
    bool forward;
    bool finished;
    Piece(int id = 0, int point);
    void move();
    void reset();
};