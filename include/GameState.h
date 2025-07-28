#pragma once

#include <memory>  // برای سادگی در پیاده‌سازی اولیه، بعدا می‌توان بهینه کرد
#include <vector>

#include "Board.h"
#include "Constants.h"
#include "Move.h"

namespace SquadroAI {

class GameState {
 public:
  // سازنده، بازی را در وضعیت اولیه قرار
  // می‌دهد
  GameState();

  // این تابع یک وضعیت جدید بر اساس حرکت داده شده ایجاد
  // می‌کند
  // این روش برای الگوریتم‌های جستجو بسیار تمیز و
  // مناسب است
  GameState createNextState(const Move& move) const;

  // لیستی از تمام حرکات قانونی برای بازیکن فعلی را
  // برمی‌گرداند
  std::vector<Move> generateLegalMoves() const;

  // بررسی می‌کند که آیا بازی تمام شده است یا
  // نه
  bool isGameOver() const;

  // برنده بازی را برمی‌گرداند (اگر بازی تمام
  // شده باشد)
  PlayerID getWinner() const;

  // توابع Getter برای دسترسی به وضعیت داخلی
  PlayerID getCurrentPlayer() const { return currentPlayer; }
  const Board& getBoard() const { return board; }
  int getTurnCount() const { return turnCount; }

 private:
  Board board;             // وضعیت فیزیکی تخته (سریع و فشرده)
  PlayerID currentPlayer;  // بازیکنی که نوبت اوست
  PlayerID winner;         // برای ذخیره برنده بازی
  int turnCount;           // تعداد نوبت‌های گذشته

  // تابع کمکی خصوصی برای به‌روزرسانی وضعیت
  // پایان بازی
  void updateGameStatus();
};

}  // namespace SquadroAI
