#pragma once

#include <array>
#include <cstdint>
#include <random>

#include "Board.h"
#include "Constants.h"

namespace SquadroAI {

class GameStateHasher {
 public:
  // این تابع باید یک بار در ابتدای برنامه فراخوانی شود
  static void initialize();

  // محاسبه هش کامل برای یک وضعیت اولیه
  static uint64_t computeHash(const Board& board, PlayerID currentPlayer);

  // **مهم‌ترین تابع:** به‌روزرسانی هش به صورت افزایشی بعد از یک
  // حرکت
  // این تابع هنوز پیاده‌سازی نشده چون به اطلاعات کامل حرکت نیاز
  // دارد.
  // در الگوریتم جستجو، ما معمولاً هش را از اول محاسبه می‌کنیم چون ساده‌تر
  // است و به لطف بهینگی، هنوز هم بسیار سریع است. ما از `computeHash` استفاده
  // خواهیم کرد.

 private:
  // جداول اعداد تصادفی برای هر ویژگی بازی
  // [piece_id][row][col]
  static std::array<std::array<std::array<uint64_t, NUM_COLS>, NUM_ROWS>,
                    NUM_PIECES>
      piece_hashes;

  // [player_id] -> 0 for P1, 1 for P2
  static std::array<uint64_t, 2> player_turn_hashes;

  // [piece_id][status] -> status 0: FWD, 1: BWD
  static std::array<std::array<uint64_t, 2>, NUM_PIECES> piece_status_hashes;

  static bool is_initialized;
};

}  // namespace SquadroAI
