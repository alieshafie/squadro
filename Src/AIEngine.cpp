#include "AIEngine.hpp"

Move AIEngine::getBestMove(const GameState &state, double timeLimit)
{
    // TODO: iterative deepening, time management, opening book, killer moves
    return Move();
}

int AIEngine::evaluate(const GameState &state)
{
    // TODO: implement heuristic evaluation
    return 0;
}

int AIEngine::alphabeta(const GameState &state, int depth, int alpha, int beta)
{
    // TODO: implement alpha-beta pruning
    return 0;
}