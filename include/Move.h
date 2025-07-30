#pragma once

#include <stdexcept>
#include <string>

#include "Constants.h"

namespace SquadroAI {

struct Move {
  int piece_index;  //(0-4) نسبی
  Move() : piece_index(-1) {}

  explicit Move(int index) : piece_index(index) {}

  int getid(PlayerID player) const {
    return (player == PlayerID::PLAYER_1) ? piece_index : piece_index + 5;
  }

  bool operator==(const Move &other) const {
    return piece_index == other.piece_index;
  }

  // برای استفاده در std::map یا std::set اگر لازم باشد
  bool operator<(const Move &other) const {
    return piece_index < other.piece_index;
  }

  int getRelativeIndex() const {
    if (piece_index >= 0 && piece_index < PIECES_PER_PLAYER) {
      return piece_index;  // Player 1's relative index is same as global
    }
    if (piece_index >= PIECES_PER_PLAYER && piece_index < NUM_PIECES) {
      return piece_index - PIECES_PER_PLAYER;  // Player 2's relative index
    }
    return -1;  // Invalid or null move
  }

  static Move fromRelativeIndex(int relative_index, PlayerID player) {
    static const Move NULL_MOVE = Move(-1);
    if (relative_index < 0 || relative_index >= PIECES_PER_PLAYER) {
      return NULL_MOVE;
    }
    if (player == PlayerID::PLAYER_1) {
      return Move(relative_index);
    }
    if (player == PlayerID::PLAYER_2) {
      return Move(relative_index + PIECES_PER_PLAYER);
    }
    return NULL_MOVE;  // Should not happen
  }

  std::string to_string() const {
    if (piece_index == -1) return "NULL_MOVE";
    return "P" + std::to_string(piece_index);
  }
};

// یک حرکت "نامعتبر" یا "تهی"
const Move NULL_MOVE = Move(-1);

}  // namespace SquadroAI
