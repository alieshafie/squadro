#include "Zobrist.hpp"

std::array<uint64_t, 64> Zobrist::table;

void Zobrist::init()
{
    // TODO: generate random values for hashing
}

uint64_t Zobrist::hash(const GameState &state)
{
    // TODO: implement Zobrist hashing based on pieces
    return 0;
}