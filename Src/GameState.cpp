#include "GameState.h"

void SquadroAI::GameState::initializeNewGame() {}

bool SquadroAI::GameState::applyMove(const SquadroAI::Move& move) {}
bool SquadroAI::GameState::undoLastMove() {}

std::vector<SquadroAI::Move> SquadroAI::GameState::getLegalMoves() const {}

bool SquadroAI::GameState::isGameOver() const {}
SquadroAI::PlayerID SquadroAI::GameState::getWinner() const {}

void SquadroAI::GameState::switchPlayer() {}

int SquadroAI::GameState::getCompletedPieceCount(PlayerID player) const {}

void SquadroAI::GameState::updateZobristHashForMove(const Move& move) {}
void SquadroAI::GameState::recomputeZobristHash() {}

// تابع کپی برای ایجاد وضعیت‌های جدید در جستجو
SquadroAI::GameState SquadroAI::GameState::createChildState(const SquadroAI::Move& move) const {}

void SquadroAI::GameState::printState() const {}