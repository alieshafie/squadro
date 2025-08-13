#pragma once

#include <array>
#include <cstddef>

#include "Constants.h"
#include "Move.h"

namespace SquadroAI {

// A fixed-capacity list of moves designed to avoid heap allocations during move
// generation. The maximum number of legal moves for a player at any point is
// PIECES_PER_PLAYER.
class MoveList {
   public:
    using iterator = typename std::array<Move, PIECES_PER_PLAYER>::iterator;
    using const_iterator =
        typename std::array<Move, PIECES_PER_PLAYER>::const_iterator;

    MoveList() : count_(0) {}

    void push_back(const Move& move) {
        if (count_ < PIECES_PER_PLAYER) {
            moves_[count_++] = move;
        }
        // In a debug build, an assertion here would be useful.
        // In release, we silently fail to avoid overhead in the critical path.
    }

    void clear() { count_ = 0; }

    size_t size() const { return count_; }

    bool empty() const { return count_ == 0; }

    Move& operator[](size_t index) { return moves_[index]; }

    const Move& operator[](size_t index) const { return moves_[index]; }

    iterator begin() { return moves_.begin(); }

    iterator end() { return moves_.begin() + count_; }

    const_iterator begin() const { return moves_.cbegin(); }

    const_iterator end() const { return moves_.cbegin() + count_; }

    const_iterator cbegin() const { return moves_.cbegin(); }

    const_iterator cend() const { return moves_.cbegin() + count_; }

   private:
    std::array<Move, PIECES_PER_PLAYER> moves_;
    size_t count_ = 0;
};

}  // namespace SquadroAI
