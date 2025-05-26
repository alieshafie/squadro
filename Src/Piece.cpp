#include "Piece.hpp"

Piece::Piece(int _id = 0, int _point)
    : id{ _id }
    , point{ _point }
    , pos{ 0 }
    , forward{ true }
    , finished{ false }
{
}

void Piece::move() {
    if (!finished) {
        if (forward)
        {
            pos += point;
            if (pos >= 4) {
                pos = 4;
                forward = false;
            }
        }

        else
        {
            pos -= 4 - point;
            if (pos <= 0) {
                pos = 0;
                finished = true;
            }
        }
    }
}

void Piece::reset() {
    if (forward)
        pos = 0;

    else
        pos = 4;
}