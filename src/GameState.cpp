#include "GameState.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace SquadroAI {

GameState::GameState()
    : board(),  // سازنده پیش‌فرض Board فراخوانی می‌شود و تخته را
                // مقداردهی می‌کند
      currentPlayer(PlayerID::PLAYER_1),
      winner(PlayerID::NONE),
      turnCount(0) {}

GameState GameState::createNextState(const Move& move) const {
  // First check the move against generateLegalMoves
  auto legal_moves = generateLegalMoves();
  if (std::find(legal_moves.begin(), legal_moves.end(), move) == legal_moves.end()) {
    std::cerr << "Move validation failed: move " << move.piece_index << " not in legal moves list.\n";
    std::cerr << "Legal moves: ";
    for (const auto& m : legal_moves) {
      std::cerr << m.piece_index << " ";
    }
    std::cerr << "\n";
    throw std::logic_error("Attempted to create next state with illegal move");
  }

  // Check that the board considers the move valid
  if (!board.isMoveValid(move, currentPlayer)) {
    std::cerr << "Move validation failed: Board::isMoveValid returned false\n";
    throw std::logic_error("Board rejected move that was in legal moves list");
  }

  // Create next state
  GameState nextState = *this;
  auto move_info = nextState.board.applyMove(move, this->currentPlayer);
  
  if (!move_info.has_value()) {
    // Print board state at the time of failure
    std::cerr << "Move application failed. Current board state:\n";
    board.printBoard();
    std::cerr << "Move details: Player " << (currentPlayer == PlayerID::PLAYER_1 ? "1" : "2") 
              << ", Piece " << move.piece_index << "\n";
    throw std::logic_error("Internal error: move validation inconsistency");
  }

  // 3. نوبت را به بازیکن بعدی بده
  nextState.currentPlayer = (this->currentPlayer == PlayerID::PLAYER_1)
                                ? PlayerID::PLAYER_2
                                : PlayerID::PLAYER_1;

  // 4. شمارنده نوبت را افزایش بده
  nextState.turnCount++;

  // 5. بررسی کن که آیا با این حرکت بازی تمام شده است یا نه
  nextState.updateGameStatus();

  // 6. وضعیت جدید را برگردان
  return nextState;
}

std::vector<Move> GameState::generateLegalMoves() const {
  if (isGameOver()) {
    return {};  // اگر بازی تمام شده، حرکتی وجود ندارد
  }
  return board.generateLegalMoves(currentPlayer);
}

bool GameState::isGameOver() const { return winner != PlayerID::NONE; }

PlayerID GameState::getWinner() const { return winner; }

// این تابع پس از هر حرکت فراخوانی می‌شود تا وضعیت برد/باخت را
// بررسی کند
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
  if (p1_finished_count >= 4 && p2_finished_count >= 4) {
    // If somehow both players finish on the same move,
    // player 1 wins (since they went first)
    winner = PlayerID::PLAYER_1;
  } else if (p1_finished_count >= 4) {
    winner = PlayerID::PLAYER_1;
  } else if (p2_finished_count >= 4) {
    winner = PlayerID::PLAYER_2;
  }
  // Draw is not possible in normal play unless forced by move limit
}

}  // namespace SquadroAI
