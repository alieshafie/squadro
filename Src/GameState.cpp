#include "GameState.hpp"

GameState::GameState() : turn(0)
{
    // TODO: initialize pieces
}

void GameState::applyMove(const Move &m)
{
    // TODO: implement rules
}

std::vector<Move> GameState::legalMoves() const
{
    // TODO: generate legal moves
    return {};
}

bool GameState::isTerminal() const
{
    // TODO: implement terminal state check
    return false;
}

uint64_t GameState::zobristHash() const
{
    return Zobrist::hash(*this);
}