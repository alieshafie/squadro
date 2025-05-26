#include "GameState.hpp"
#include "AIEngine.hpp"
#include <iostream>

int main()
{
    Zobrist::init();
    GameState state;
    // TODO: read initial JSON or stream

    AIEngine ai;
    auto best = ai.getBestMove(state, 28.0);
    std::cout << int(best.pieceId) << std::endl;
    return 0;
}