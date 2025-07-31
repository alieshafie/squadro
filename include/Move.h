#pragma once

#include <stdexcept>
#include <string>

#include "Constants.h"

namespace SquadroAI {

struct Move {
  // تبدیل شناسه سراسری به اندیس نسبی برای بازیکن
  // داده‌شده
  int getRelativeIndex(PlayerID player) const {
    if (id < 0 || id >= 2 * PIECES_PER_PLAYER) return -1;
    if (player == PlayerID::PLAYER_1 && id < PIECES_PER_PLAYER) return id;
    if (player == PlayerID::PLAYER_2 && id >= PIECES_PER_PLAYER)
      return id - PIECES_PER_PLAYER;
    return -1;
  }
  int id;  // شناسه سراسری مهره (0-9)

  // مقدار ثابت برای حرکت نامعتبر
  static constexpr int INVALID_ID = -1;
  static Move NULL_MOVE() { return Move(INVALID_ID); }

  Move() : Move(INVALID_ID) {}
  explicit Move(int index) : id(index) {}

  bool operator==(const Move& other) const { return id == other.id; }
  bool operator<(const Move& other) const { return id < other.id; }

  // ساخت حرکت از اندیس نسبی و بازیکن
  static Move fromRelativeIndex(int relative_index, PlayerID player) {
    if (relative_index < 0 || relative_index >= PIECES_PER_PLAYER)
      return NULL_MOVE();
    if (player == PlayerID::PLAYER_1) return Move(relative_index);
    if (player == PlayerID::PLAYER_2)
      return Move(relative_index + PIECES_PER_PLAYER);
    return NULL_MOVE();
  }

  std::string to_string() const {
    if (id == INVALID_ID) return "NULL_MOVE";
    return "P" + std::to_string(id);
  }
};

}  // namespace SquadroAI
