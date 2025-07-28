#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

#include "Constants.h"
#include "Piece.h"  // برای دسترسی به وضعیت مهره‌ها

namespace SquadroAI {

class GameState;  // Forward declaration

class GameStateHasher {
 public:
  GameStateHasher();  // مقداردهی اولیه جداول Zobrist

  // محاسبه هش کامل برای یک وضعیت
  uint64_t computeHash(const GameState &state) const;

  // به‌روزرسانی هش به صورت افزایشی پس از یک
  // حرکت این تابع باید اطلاعات کافی در مورد تغییرات وضعیت را دریافت کند
  uint64_t updateHash(uint64_t current_hash,
                      /* اطلاعات تغییرات */ const Piece &moved_piece, int old_r,
                      int old_c, int new_r, int new_c,
                      const std::optional<Piece> &captured_piece_opt,
                      PlayerID next_player_to_move) const;

 private:
  // جداول Zobrist: اعداد تصادفی برای هر (مهره، خانه، وضعیت مهره) و نوبت بازیکن
  //[Col]
  std::array<std::array<std::array<uint64_t, NUM_COLS>, NUM_ROWS>,
             PIECES_PER_PLAYER * 2>
      piece_position_keys;
  //
  std::array<std::array<uint64_t, 4>, PIECES_PER_PLAYER * 2>
      piece_status_keys;                     // 4 وضعیت ممکن برای PieceStatus
  std::array<uint64_t, 3> player_turn_keys;  // برای Player1, Player2

  void initializeKeys();
};

}  // namespace SquadroAI
