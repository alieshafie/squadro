#pragma once

#include <array>
#include <cstdint>
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
  static constexpr int MAX_POSSIBLE_CAPTURES = PIECES_PER_PLAYER;

  struct CapturedInfo {
    uint8_t id;
    int8_t prev_row;
    int8_t prev_col;
  };

  struct AppliedMoveInfo {
    uint8_t mover_id;
    int8_t start_row;
    int8_t start_col;
    uint8_t original_mover_status;
    uint8_t captured_count = 0;
    std::array<CapturedInfo, MAX_POSSIBLE_CAPTURES> captures;
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
  std::array<Cell, NUM_ROWS * NUM_COLS> grid;

  // **بهینه‌سازی کلیدی شماره ۲: Single Source of Truth**
  std::array<Piece, NUM_PIECES> pieces;

  inline Cell& cell_ref(int r, int c) {
    return grid[static_cast<size_t>(r * NUM_COLS + c)];
  }
  inline const Cell& cell_ref(int r, int c) const {
    return grid[static_cast<size_t>(r * NUM_COLS + c)];
  }

  inline bool isPositionValid(int row, int col) const {
    return row >= 0 && row < NUM_ROWS && col >= 0 && col < NUM_COLS;
  }
};

}  // namespace SquadroAI
