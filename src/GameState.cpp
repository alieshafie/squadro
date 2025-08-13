#include "GameState.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace SquadroAI {

GameState::GameState()
    : board(),  // سازنده پیش‌فرض Board فراخوانی می‌شود
                // و تخته را مقداردهی می‌کند
      currentPlayer(PlayerID::PLAYER_1),
      winner(PlayerID::NONE),
      turnCount(0) {}

std::optional<Board::AppliedMoveInfo> GameState::applyMove(const Move& move) {
  auto move_info = board.applyMove(move, currentPlayer);

  if (move_info.has_value()) {
    // Check for a win *before* switching the player.
    // This correctly assigns the win to the player who made the move.
    updateGameStatus();
    currentPlayer = (currentPlayer == PlayerID::PLAYER_1) ? PlayerID::PLAYER_2
                                                          : PlayerID::PLAYER_1;
    turnCount++;
  }

  return move_info;
}

void GameState::undoMove(const Board::AppliedMoveInfo& move_info) {
  board.undoMove(move_info);
  // Restore the player and turn count first to correctly reflect the previous
  // state.
  currentPlayer = (currentPlayer == PlayerID::PLAYER_1) ? PlayerID::PLAYER_2
                                                        : PlayerID::PLAYER_1;
  turnCount--;

  // Reset winner status and re-evaluate based on the restored state.
  winner = PlayerID::NONE;
  updateGameStatus();
}

void GameState::generateLegalMoves(MoveList& moves) const {
  moves.clear();
  if (isGameOver()) {
    return;  // اگر بازی تمام شده، حرکتی وجود ندارد
  }
  board.generateLegalMoves(currentPlayer, moves);
}

bool GameState::isGameOver() const { return winner != PlayerID::NONE; }

PlayerID GameState::getWinner() const { return winner; }

// این تابع پس از هر حرکت فراخوانی می‌شود تا
// وضعیت برد/باخت را بررسی کند
void GameState::updateGameStatus() {
  int p1_finished_count = 0;
  int p2_finished_count = 0;

  const auto& all_pieces = board.getAllPieces();
  for (int id = 0; id < NUM_PIECES; ++id) {
    if (all_pieces[id].isFinished()) {
      if (all_pieces[id].owner == PlayerID::PLAYER_1) {
        p1_finished_count++;
      } else {
        p2_finished_count++;
      }
    }
  }

  // Handle win conditions carefully, checking both players
  // to handle potential race conditions
  bool p1_wins = p1_finished_count >= 4;
  bool p2_wins = p2_finished_count >= 4;

  if (p1_wins && !p2_wins) {
    winner = PlayerID::PLAYER_1;
  } else if (p2_wins && !p1_wins) {
    winner = PlayerID::PLAYER_2;
  } else if (p1_wins && p2_wins) {
    // If both players finish on the same turn, the player whose move it was
    // wins. The *current* player is the one who just moved.
    winner = currentPlayer;
  } else {
    winner = PlayerID::NONE;
  }
}

}  // namespace SquadroAI
