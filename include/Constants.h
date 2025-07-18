#pragma once

namespace SquadroAI
{

    // ابعاد تخته و تعداد مهره‌ها
    constexpr int NUM_ROWS = 7; // تعداد سطرها (با احتساب خانه‌های شروع و پایان)
    constexpr int NUM_COLS = 7; // تعداد ستون‌ها
    constexpr int PIECES_PER_PLAYER = 5;

    // شناسه‌های بازیکنان
    enum class PlayerID
    {
        NONE,
        PLAYER_1 = 1,
        PLAYER_2 = 2,
        DRAW
    };

    // قدرت حرکت اولیه مهره‌ها برای هر بازیکن
    // این مقادیر باید بر اساس قوانین دقیق بازی Squadro تنظیم شوند
    // مقادیر زیر صرفاً نمونه هستند و باید با قوانین بازی تطبیق داده شوند.
    // این آرایه‌ها قدرت حرکت مهره‌ها را از چپ به راست (یا از بالا به پایین بسته به جهت‌گیری بازیکن) نشان می‌دهند.
    // قدرت حرکت مهره‌ها در مسیر رفت
    constexpr int PLAYER_1_FWD_POWERS[PIECES_PER_PLAYER] = {1, 3, 2, 3, 1}; // مثال
    constexpr int PLAYER_2_FWD_POWERS[PIECES_PER_PLAYER] = {3, 1, 2, 1, 3}; // مثال
    // قدرت حرکت مهره‌ها در مسیر برگشت (پس از رسیدن به انتهای تخته)
    constexpr int PLAYER_1_BCK_POWERS[PIECES_PER_PLAYER] = {3, 1, 2, 1, 3}; // مثال
    constexpr int PLAYER_2_BCK_POWERS[PIECES_PER_PLAYER] = {1, 3, 2, 3, 1}; // مثال

    // امتیازات برای تابع ارزیابی (مقادیر اولیه، نیاز به تنظیم دقیق دارند)
    constexpr int WIN_SCORE = 100000;
    constexpr int LOSS_SCORE = -100000;
    constexpr int DRAW_SCORE = 0;
    constexpr int PIECE_COMPLETED_WEIGHT = 5000;
    constexpr int PIECE_PROGRESS_WEIGHT = 10;
    constexpr int PIECE_MATERIAL_WEIGHT = 100; // ارزش داشتن مهره روی تخته
    constexpr int MOBILITY_WEIGHT = 5;

    // سایر ثابت‌های مورد نیاز
    //...

} // namespace SquadroAI