#pragma once

#include <array>
#include <optional>
#include <vector>

#include "Constants.h"
#include "Move.h"
#include "Piece.h"

namespace SquadroAI {
// بهینه‌سازی ۱: استفاده از مقدار نگهبان
// (Sentinel Value)
using Cell = int;
constexpr int EMPTY_CELL = -1;

class Board {
 public:
  // اطلاعات مورد نیاز برای بازگرداندن یک حرکت
  struct AppliedMoveInfo {
    Move move;
    Piece captured_piece;
    bool captured_piece_valid = false;
    PieceStatus original_mover_status;
  };

  Board();

  // توابع اصلی دیگر به 'pieces' به عنوان آرگومان نیاز ندارند
  std::optional<AppliedMoveInfo> applyMove(const Move& move,
                                           PlayerID current_player);
  void undoMove(const AppliedMoveInfo& move_info, PlayerID original_player);
  std::vector<Move> generateLegalMoves(PlayerID player) const;
  bool isMoveValid(const Move& move, PlayerID player) const;

  void printBoard() const;

  // توابع Getter برای دسترسی ایمن از خارج
  const Piece& getPiece(int piece_id) const;
  Cell getCell(int row, int col) const;

 private:
  // این تابع فقط برای مقداردهی اولیه است
  void initializeBoard();

  // بهینه‌سازی ۲: آرایه مسطح برای grid
  std::array<Cell, NUM_ROWS * NUM_COLS> grid;

  // بهینه‌سازی ۳: آرایه مهره‌ها به داخل Board
  // منتقل شد این کار Data Locality را به حداکثر
  // می‌رساند.
  std::array<Piece, NUM_PIECES> pieces;

  // توابع کمکی inline برای دسترسی سریع به grid
  inline Cell& get_cell_ref(int r, int c) { return grid[r * NUM_COLS + c]; }
  inline const Cell& get_cell_ref(int r, int c) const {
    return grid[r * NUM_COLS + c];
  }
};

}  // namespace SquadroAI
