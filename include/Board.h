#pragma once

#include <array>
#include <optional>
#include <vector>

#include "Constants.h"
#include "Move.h"
#include "Piece.h"

namespace SquadroAI {

using Cell = int;
constexpr int EMPTY_CELL = -1;

class GameState;  // Forward declaration

class Board {
 public:
  Board();
  void initializeBoard();  // تنظیم مهره‌ها در شروع بازی

  // اعمال یک حرکت به تخته. این تابع باید تمام منطق بازی را
  // پیاده‌سازی کند: حرکت مهره، گرفتن مهره
  // حریف، دور زدن، و به‌روزرسانی وضعیت
  // مهره‌ها. برگرداندن اطلاعاتی برای undo_move
  // (مثلاً مهره گرفته شده)
  struct AppliedMoveInfo {
    Move move;
    Piece captured_piece;  // اگر مهره‌ای گرفته شده باشد
    bool captured_piece_valid = false;
    PieceStatus original_mover_status;
    PlayerID original_mover_owner;
    int original_mover_row;
    int original_mover_col;
    // سایر اطلاعات لازم برای undo
  };
  // اگر حرکت معتبر نباشد، std::nullopt
  // برمی‌گرداند
  std::optional<AppliedMoveInfo> applyMove(const Move &move,
                                           PlayerID current_player,
                                           std::vector<Piece> &pieces);

  // بازگرداندن آخرین حرکت اعمال شده
  void undoMove(const AppliedMoveInfo &move_info, std::vector<Piece> &pieces);

  // تولید تمام حرکات قانونی برای بازیکن فعلی
  std::vector<Move> generateLegalMoves(PlayerID player,
                                       const std::vector<Piece> &pieces) const;

  // بررسی اینکه آیا یک حرکت خاص برای یک مهره خاص قانونی است
  bool isMoveValid(const Move &move, PlayerID player,
                   const std::vector<Piece> &pieces) const;

  // چاپ تخته برای دیباگ
  void printBoard(const std::vector<Piece> &pieces) const;

  // دسترسی به خانه‌های تخته
  Cell getCell(int r, int c) const;
  void setCell(int r, int c, Cell piece_id);

 private:
  std::array<Cell, NUM_ROWS * NUM_COLS> grid;

  // توابع کمکی برای دسترسی به آرایه تک‌بعدی
  // به صورت دو بعدی
  // با تعریف در هدر، کامپایلر آنها را inline
  // می‌کند و هیچ سربار فراخوانی تابعی وجود نخواهد داشت.
  inline Cell &get_cell_ref(int r, int c) { return grid[r * NUM_COLS + c]; }

  inline const Cell &get_cell_ref(int r, int c) const {
    return grid[r * NUM_COLS + c];
  }
};

}  // namespace SquadroAI
