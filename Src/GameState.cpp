#include "GameState.h"

#include <stdexcept>

namespace SquadroAI {

GameState::GameState()
    : board(),  // سازنده پیش‌فرض Board فراخوانی می‌شود و تخته را مقداردهی
                // می‌کند
      currentPlayer(PlayerID::PLAYER_1),
      winner(PlayerID::NONE),
      turnCount(0) {}

GameState GameState::createNextState(const Move& move) const {
  // 1. یک کپی کامل از وضعیت فعلی ایجاد کن (این عمل به لطف بهینه‌سازی Board
  // سریع است)
  GameState nextState = *this;

  // 2. حرکت را روی تخته‌ی کپی شده اعمال کن
  // تابع applyMove در Board اطلاعاتی برای undo برمی‌گرداند که اینجا به آن
  // نیاز نداریم چون ما با یک کپی کار می‌کنیم.
  auto move_info = nextState.board.applyMove(move, this->currentPlayer);
  if (!move_info.has_value()) {
    // این اتفاق نباید بیفتد اگر فقط حرکات قانونی ارسال شوند
    throw std::logic_error("Attempted to apply an invalid move.");
  }

  // 3. نوبت را به بازیکن بعدی بده
  nextState.currentPlayer = (this->currentPlayer == PlayerID::PLAYER_1)
                                ? PlayerID::PLAYER_2
                                : PlayerID::PLAYER_1;

  // 4. شمارنده نوبت را افزایش بده
  nextState.turnCount++;

  // 5. بررسی کن که آیا با این حرکت بازی تمام شده است یا نه
  nextState.updateGameStatus();

  // 6. وضعیت جدید را برگردان
  return nextState;
}

std::vector<Move> GameState::generateLegalMoves() const {
  if (isGameOver()) {
    return {};  // اگر بازی تمام شده، حرکتی وجود ندارد
  }
  return board.generateLegalMoves(currentPlayer);
}

bool GameState::isGameOver() const { return winner != PlayerID::NONE; }

PlayerID GameState::getWinner() const { return winner; }

// این تابع پس از هر حرکت فراخوانی می‌شود تا وضعیت برد/باخت را بررسی
// کند
void GameState::updateGameStatus() {
  int p1_finished_count = 0;
  int p2_finished_count = 0;

  const auto& all_pieces = board.getAllPieces();
  for (int id = 0; id < NUM_PIECES; ++id) {
    if (all_pieces[id].isFinished()) {
      if (all_pieces[id].owner == PlayerID::PLAYER_1) {
        p1_finished_count++;
      } else {
        p2_finished_count++;
      }
    }
  }

  // طبق قوانین، بازیکنی که ۴ مهره‌اش را به خط پایان برساند برنده
  // است.
  if (p1_finished_count >= 4) {
    winner = PlayerID::PLAYER_1;
  } else if (p2_finished_count >= 4) {
    winner = PlayerID::PLAYER_2;
  }
  // در این بازی حالت تساوی به طور طبیعی رخ نمی‌دهد مگر با قانون خاص (مثلا
  // تعداد حرکات) که فعلا آن را نادیده می‌گیریم.
}

}  // namespace SquadroAI
