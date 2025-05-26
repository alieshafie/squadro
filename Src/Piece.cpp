#include "Piece.hpp"

Piece::Piece(int id, int pos, int distance)
    : id{id}, distance{distance}, pos{0}, forward{true}, finished{false}
{
}

void Piece::move()
{
    if (!finished)
    {
        if (forward)
        {
            pos += distance;
            if (pos >= N - 1)
            {
                pos = N - 1;
                forward = false;
            }
        }

        else
        {
            pos -= 4 - distance;
            if (pos <= 0)
            {
                pos = 0;
                finished = true;
            }
        }
    }
}

void Piece::reset()
{
    if (forward)
        pos = 0;

    else
        pos = N - 1;
}