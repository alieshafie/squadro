#pragma once

#include "Board.h"
#include "Constants.h"
#include "MoveList.h"
#include "Move.h"

namespace SquadroAI {

class GameState {
 public:
  // سازنده، بازی را در وضعیت اولیه قرار
  // می‌دهد
  GameState();

  // Applies a move to the current state, mutating it.
  // Returns the information needed to undo the move.
  std::optional<Board::AppliedMoveInfo> applyMove(const Move& move);

  // Reverts a move, restoring the previous state.
  void undoMove(const Board::AppliedMoveInfo& move_info);

  // Fills the provided MoveList with all legal moves for the current player.
  void generateLegalMoves(MoveList& moves) const;

  // بررسی می‌کند که آیا بازی تمام شده است یا
  // نه
  bool isGameOver() const;

  // برنده بازی را برمی‌گرداند (اگر بازی تمام
  // شده باشد)
  PlayerID getWinner() const;

  // توابع Getter برای دسترسی به وضعیت داخلی
  PlayerID getCurrentPlayer() const { return currentPlayer; }
  const Board& getBoard() const { return board; }
  int getTurnCount() const { return turnCount; }

 private:
  Board board;             // وضعیت فیزیکی تخته (سریع و فشرده)
  PlayerID currentPlayer;  // بازیکنی که نوبت اوست
  PlayerID winner;         // برای ذخیره برنده بازی
  int turnCount;           // تعداد نوبت‌های گذشته

  // تابع کمکی خصوصی برای به‌روزرسانی وضعیت
  // پایان بازی
  void updateGameStatus();
};

}  // namespace SquadroAI
