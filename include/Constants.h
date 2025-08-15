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
// Score for a piece that has successfully returned to its base
constexpr int PIECE_COMPLETED_SCORE = 2500;

// Positional bonus based on how far a piece has traveled.
// Index represents progress (0-12). 0=start, 6=turnaround, 12=finish.
// The scores are non-linear, rewarding progress near the end of the track more.
constexpr int PIECE_PROGRESS_SCORE[13] = {
    0,    // Progress 0 (At base)
    5,    // Progress 1
    10,   // Progress 2
    20,   // Progress 3
    35,   // Progress 4
    55,   // Progress 5
    80,   // Progress 6 (Turnaround point)
    110,  // Progress 7 (First step back)
    145,  // Progress 8
    185,  // Progress 9
    230,  // Progress 10
    280,  // Progress 11
    350   // Progress 12 (One step away from finishing)
};

// سایر ثابت‌های مورد نیاز
//...

}  // namespace SquadroAI
