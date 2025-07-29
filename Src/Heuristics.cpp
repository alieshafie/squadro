#include "Heuristics.h"

#include "GameState.h"

namespace SquadroAI {

int Heuristics::evaluate(const GameState& state, PlayerID ai_player_id) {
  // --- 1. بررسی حالت‌های پایانی (مهم‌ترین بخش)
  // ---
  if (state.isGameOver()) {
    if (state.getWinner() == ai_player_id) {
      return WIN_SCORE;
    } else if (state.getWinner() == PlayerID::NONE ||
               state.getWinner() == PlayerID::DRAW) {
      // در Squadro تساوی تعریف نشده مگر با قوانین خانه
      return DRAW_SCORE;
    } else {
      return LOSS_SCORE;
    }
  }

  int my_score = 0;
  int opponent_score = 0;

  PlayerID opponent_id = (ai_player_id == PlayerID::PLAYER_1)
                             ? PlayerID::PLAYER_2
                             : PlayerID::PLAYER_1;

  const auto& pieces = state.getBoard().getAllPieces();

  // --- 2. محاسبه امتیاز بر اساس پیشرفت و مهره‌های تمام شده
  // ---
  for (const auto& piece : pieces) {
    int piece_score = 0;

    // الف) امتیاز برای تمام کردن مهره
    if (piece.isFinished()) {
      piece_score += PIECE_COMPLETED_WEIGHT;
    } else {
      // ب) امتیاز برای پیشرفت در مسیر
      int progress = 0;
      if (piece.owner == PlayerID::PLAYER_1) {
        // مسیر بازیکن ۱: ستون 0 -> 6 (پیشروی)، ستون 6 -> 0 (بازگشت)
        if (piece.status == PieceStatus::ON_BOARD_FORWARD) {
          progress = piece.col;  // 0 to 6
        } else {                 // ON_BOARD_BACKWARD
          // پیشرفت کل: 6 برای رفت + (6 - col) برای برگشت
          progress = (NUM_COLS - 1) + ((NUM_COLS - 1) - piece.col);
        }
      } else {  // Player 2
        // مسیر بازیکن ۲: سطر 0 -> 6 (پیشروی)، سطر 6 -> 0 (بازگشت)
        if (piece.status == PieceStatus::ON_BOARD_FORWARD) {
          progress = piece.row;  // 0 to 6
        } else {                 // ON_BOARD_BACKWARD
          progress = (NUM_ROWS - 1) + ((NUM_ROWS - 1) - piece.row);
        }
      }
      piece_score += progress * PIECE_PROGRESS_WEIGHT;
    }

    // اضافه کردن امتیاز به بازیکن مربوطه
    if (piece.owner == ai_player_id) {
      my_score += piece_score;
    } else {
      opponent_score += piece_score;
    }
  }

  // امتیاز نهایی تفاوت امتیاز ما و حریف است
  return my_score - opponent_score;
}

}  // namespace SquadroAI
