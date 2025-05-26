#pragma once
#include "GameState.hpp"
#include <array>

class Zobrist
{
public:
    static void init();
    static uint64_t hash(const GameState &state);

private:
    static std::array<uint64_t, Piece::N * GameState::COLUMN_LEN> table;
};