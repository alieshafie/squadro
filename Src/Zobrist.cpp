#include "Zobrist.hpp"
#include <random>

std::array<uint64_t, GameState::N * GameState::COLUMN_LEN> Zobrist::table;

void Zobrist::init()
{
    std::mt19937_64 gen(42);
    std::uniform_int_distribution<uint64_t> dist;
    for (auto &v : table)
        v = dist(gen);
}

uint64_t Zobrist::hash(const GameState &state)
{
    return state.hash;
}