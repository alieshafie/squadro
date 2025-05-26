#pragma once

class Piece
{
public:
    static constexpr int N = 5;
    int id;
    int pos;
    int distance;
    bool forward;
    bool finished;

    Piece(int id, int pos = 0, int distance);
    void move();
    void reset();
};