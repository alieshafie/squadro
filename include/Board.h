#pragma once

#include <vector>
#include <array>
#include <optional>
#include "Constants.h"
#include "Piece.h"
#include "Move.h"

namespace SquadroAI
{

    // نمایانگر یک خانه روی تخته. می‌تواند خالی باشد یا یک مهره داشته باشد.
    // در اینجا از PieceID استفاده می‌کنیم که شناسه منحصربه‌فرد مهره است.
    using Cell = std::optional<int>; // int شناسه مهره (Piece::id)

    class GameState; // Forward declaration

    class Board
    {
    public:
        Board();
        void initializeBoard(); // تنظیم مهره‌ها در شروع بازی

        // اعمال یک حرکت به تخته. این تابع باید تمام منطق بازی را پیاده‌سازی کند:
        // حرکت مهره، گرفتن مهره حریف، دور زدن، و به‌روزرسانی وضعیت مهره‌ها.
        // برگرداندن اطلاعاتی برای undo_move (مثلاً مهره گرفته شده)
        struct AppliedMoveInfo
        {
            Move move;
            Piece captured_piece; // اگر مهره‌ای گرفته شده باشد
            bool captured_piece_valid = false;
            PieceStatus original_mover_status;
            PlayerID original_mover_owner;
            int original_mover_row;
            int original_mover_col;
            // سایر اطلاعات لازم برای undo
        };
        // اگر حرکت معتبر نباشد، std::nullopt برمی‌گرداند
        std::optional<AppliedMoveInfo> applyMove(const Move &move, PlayerID current_player, std::vector<Piece> &pieces);

        // بازگرداندن آخرین حرکت اعمال شده
        void undoMove(const AppliedMoveInfo &move_info, std::vector<Piece> &pieces);

        // تولید تمام حرکات قانونی برای بازیکن فعلی
        std::vector<Move> generateLegalMoves(PlayerID player, const std::vector<Piece> &pieces) const;

        // بررسی اینکه آیا یک حرکت خاص برای یک مهره خاص قانونی است
        bool isMoveValid(const Move &move, PlayerID player, const std::vector<Piece> &pieces) const;

        // چاپ تخته برای دیباگ
        void printBoard(const std::vector<Piece> &pieces) const;

        // دسترسی به خانه‌های تخته
        Cell getCell(int r, int c) const;
        void setCell(int r, int c, Cell piece_id);

    private:
        // نمایش تخته: یک آرایه دوبعدی از Cell ها
        // بازیکن 2 معمولاً از ردیف 0 حرکت می‌کند، بازیکن 1 از ستون 0 (بسته به توافق)
        // در اینجا فرض می‌کنیم هر دو از "لبه" خود حرکت می‌کنند.
        // Player 1 pieces (0-4) start at (1,0), (2,0), (3,0), (4,0), (5,0) and move towards col 6
        // Player 2 pieces (5-9) start at (0,1), (0,2), (0,3), (0,4), (0,5) and move towards row 6
        // این باید با قوانین دقیق Squadro تطبیق داده شود.
        std::array<std::array<Cell, NUM_COLS>, NUM_ROWS> grid;

        // توابع کمکی داخلی برای منطق بازی
        // مثلاً: getPieceAt(int r, int c, const std::vector<Piece>& pieces)
        //         getStartRowColForPiece(PlayerID player, int piece_idx)
        //         isOutOfBounds(int r, int c)
        //        ...
    };

} // namespace SquadroAI