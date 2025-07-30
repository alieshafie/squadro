#pragma once

#include <string>

#include "Constants.h"

namespace SquadroAI {

enum class PieceStatus { ON_BOARD_FORWARD, ON_BOARD_BACKWARD, FINISHED };

// این کلاس اکنون بیشتر شبیه یک struct ساده است، که برای سرعت عالی است.
class Piece {
 public:
  PlayerID owner;
  int id;  // شناسه سراسری 0 تا 9

  int row;
  int col;
  PieceStatus status;
  int forward_power;
  int backward_power;

  // سازنده پیش‌فرض برای سادگی
  Piece()
      : owner(PlayerID::NONE),
        id(-1),
        row(-1),
        col(-1),
        status(PieceStatus::ON_BOARD_BACKWARD),
        forward_power(0),
        backward_power(0) {}

  // توابع کمکی سریع
  inline bool isFinished() const { return status == PieceStatus::FINISHED; }

  inline int getCurrentMovePower() const {
    return (status == PieceStatus::ON_BOARD_FORWARD) ? forward_power
                                                     : backward_power;
  }

  // این تابع برای دیباگ مفید است و روی عملکرد تأثیر ندارد
  std::string to_string() const {
    char buffer[4];  // e.g., "P1>", "p8<"
    char player_char = 'p';
    char dir_char =
        (owner == PlayerID::PLAYER_1)
            ? ((status == PieceStatus::ON_BOARD_FORWARD) ? '>' : '<')
            : ((status == PieceStatus::ON_BOARD_FORWARD) ? '^' : 'v');
    snprintf(buffer, sizeof(buffer), "%c%d%c", player_char, id % 5, dir_char);
    return std::string(buffer);
  }
};

}  // namespace SquadroAI
