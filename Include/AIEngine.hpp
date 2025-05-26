#pragma once
#include "GameState.hpp"

class AIEngine
{
public:
    Move getBestMove(const GameState &state, double timeLimit);

private:
    int evaluate(const GameState &state);
    int alphabeta(const GameState &state, int depth, int alpha, int beta);
};