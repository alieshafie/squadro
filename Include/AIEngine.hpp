#pragma once
#include "GameState.hpp"
#include <unordered_map>

struct TTEntry
{
    int value, depth;
};

class AIEngine
{
public:
    Move getBestMove(const GameState &state, double timeLimit);

private:
    int evaluate(const GameState &state);
    int alphabeta(const GameState &state, int depth, int alpha, int beta);
    std::unordered_map<uint64_t, TTEntry> transpositionTable;
};