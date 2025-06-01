#pragma once

#include "Constants.h"

namespace SquadroAI
{

    enum class PieceStatus
    {
        ON_BOARD_FORWARD,  // در حال حرکت به سمت جلو
        ON_BOARD_BACKWARD, // در حال حرکت به سمت عقب (پس از دور زدن)
        FINISHED,          // سفر را تمام کرده و از بازی خارج شده
        NOT_STARTED        // هنوز وارد بازی نشده (اگر چنین حالتی وجود دارد)
    };

    class Piece
    {
    public:
        PlayerID owner;
        int id;                 // یک شناسه منحصربه‌فرد برای مهره در کل بازی (0-4 برای بازیکن 1، 5-9 برای بازیکن 2)
        int player_piece_index; // اندیس مهره برای بازیکن خودش (0-4)

        int row;
        int col;
        PieceStatus status;
        int forward_power;  // قدرت حرکت در مسیر رفت
        int backward_power; // قدرت حرکت در مسیر برگشت

        Piece(PlayerID p_owner = PlayerID::NONE, int p_id = -1, int p_player_piece_index = -1,
              int p_row = -1, int p_col = -1,
              int fwd_power = 0, int bck_power = 0);

        int getCurrentMovePower() const;
        bool isFinished() const;
        // سایر توابع کمکی مورد نیاز
    };

} // namespace SquadroAI