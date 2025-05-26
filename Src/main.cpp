#include "GameState.hpp"
#include "AIEngine.hpp"
#include <iostream>
#include <chrono>

int main()
{
    GameState state;
    // TODO: Read initial state from stdin or socket

    AIEngine ai;
    auto start = std::chrono::high_resolution_clock::now();
    Move best = ai.getBestMove(state, 28.0);
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << int(best.pieceId) << std::endl;
    return 0;
}