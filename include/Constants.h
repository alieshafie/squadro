#pragma once

namespace SquadroAI {

// ابعاد تخته و تعداد مهره‌ها
constexpr int NUM_ROWS = 7;  // تعداد سطرها (با احتساب
                             // خانه‌های شروع و پایان)
constexpr int NUM_COLS = 7;  // تعداد ستون‌ها
constexpr int PIECES_PER_PLAYER = 5;
constexpr int NUM_PIECES = 2 * PIECES_PER_PLAYER;

// شناسه‌های بازیکنان
enum class PlayerID { NONE, PLAYER_1 = 1, PLAYER_2 = 2, DRAW };

constexpr int FWD_POWERS[NUM_PIECES] = {1, 3, 2, 3, 1, 3, 1, 2, 1, 3};
constexpr int BCK_POWERS[NUM_PIECES] = {3, 1, 2, 1, 3, 1, 3, 2, 3, 1};
// امتیازات برای تابع ارزیابی (مقادیر اولیه، نیاز به تنظیم دقیق دارند)
constexpr int WIN_SCORE = 100000;
constexpr int LOSS_SCORE = -100000;
constexpr int DRAW_SCORE = 0;
constexpr int PIECE_COMPLETED_WEIGHT = 2500;
constexpr int PIECE_PROGRESS_WEIGHT = 10;

// سایر ثابت‌های مورد نیاز
//...

}  // namespace SquadroAI
