#pragma once

#include <vector>
#include <chrono>
#include "Constants.h"
#include "Board.h"
#include "Piece.h"
#include "Move.h"

namespace SquadroAI
{
    class GameState
    {
    private:
        Board board;
        std::vector<Piece> pieces; // تمام مهره‌های بازی (هر دو بازیکن)
        PlayerID current_player;
        int turn_count;

        // شمارش مهره‌های به خانه رسانده شده توسط هر بازیکن
        int player1_completed_pieces;
        int player2_completed_pieces;

        // تاریخچه حرکات (برای undo و تشخیص تکرار وضعیت اگر لازم باشد)
        std::vector<Board::AppliedMoveInfo> move_history;

        uint64_t zobrist_hash; // برای جدول انتقال

    public:
        GameState();
        void initializeNewGame();

        bool applyMove(const Move &move); // اعمال حرکت و به‌روزرسانی وضعیت
        bool undoLastMove();              // بازگرداندن آخرین حرکت

        std::vector<Move> getLegalMoves() const;

        bool isGameOver() const;
        PlayerID getWinner() const; // برگرداندن برنده یا PlayerID::DRAW یا PlayerID::NONE

        PlayerID getCurrentPlayer() const { return current_player; }
        void setCurrentPlayer(PlayerID player) { current_player = player; };
        void switchPlayer();

        int getCompletedPieceCount(PlayerID player) const;

        uint64_t getZobristHash() const { return zobrist_hash; }
        void updateZobristHashForMove(const Move &move); // این باید پیچیده‌تر باشد
        void recomputeZobristHash();                     // برای اطمینان یا مقداردهی اولیه

        // تابع کپی برای ایجاد وضعیت‌های جدید در جستجو
        GameState createChildState(const Move &move) const;

        void printState() const; // برای دیباگ
    };

} // namespace SquadroAI