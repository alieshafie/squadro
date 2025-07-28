#pragma once

#include <array>
#include <optional>
#include <vector>

#include "Constants.h"
#include "Move.h"
#include "Piece.h"

namespace SquadroAI {
using Cell = int;
constexpr Cell EMPTY_CELL = -1;

class Board {
 public:
  struct AppliedMoveInfo {
    Move move;
    Piece captured_piece;  // یک کپی کامل از مهره‌ای که
                           // گرفته شده
    PieceStatus original_mover_status;
    bool piece_was_captured = false;
  };

  Board();  // سازنده، تخته را به وضعیت اولیه مقداردهی
            // می‌کند

  // این دو تابع، قلب تپنده موتور بازی هستند
  std::optional<AppliedMoveInfo> applyMove(const Move& move,
                                           PlayerID current_player);
  void undoMove(const AppliedMoveInfo& move_info);

  // تولید حرکات قانونی برای یک بازیکن
  std::vector<Move> generateLegalMoves(PlayerID player) const;

  // بررسی اینکه آیا یک حرکت در وضعیت فعلی معتبر است یا نه
  bool isMoveValid(const Move& move, PlayerID player) const;

  void printBoard() const;  // برای دیباگ

  // توابع Getter برای دسترسی ایمن و فقط خواندنی از خارج (مثلا از Heuristics)
  const Piece& getPiece(int piece_id) const;
  Cell getCell(int row, int col) const;
  const std::array<Piece, NUM_PIECES>& getAllPieces() const;

 private:
  // این تابع فقط یک بار در سازنده فراخوانی
  // می‌شود
  void initializeBoard();

  // **بهینه‌سازی کلیدی شماره ۱: Data Locality**
  // به جای grid دو بعدی، از یک آرایه یک بعدی استفاده
  // می‌کنیم.
  // این کار باعث می‌شود کل تخته در یک بلوک
  // حافظه پیوسته قرار گیرد.
  std::array<Cell, NUM_ROWS * NUM_COLS> grid;

  // **بهینه‌سازی کلیدی شماره ۲: Single Source of Truth**
  // تمام مهره‌ها به صورت یک آرایه با اندازه ثابت درون خود
  // کلاس
  // Board ذخیره می‌شوند. `grid` و `pieces` کنار هم در حافظه
  // قرار می‌گیرند و سرعت دسترسی به اوج می‌رسد.
  std::array<Piece, NUM_PIECES> pieces;

  // توابع کمکی inline برای دسترسی سریع به
  // سلول‌های grid
  inline Cell& cell_ref(int r, int c) { return grid[r * NUM_COLS + c]; }
  inline const Cell& cell_ref(int r, int c) const {
    return grid[r * NUM_COLS + c];
  }
};

}  // namespace SquadroAI
