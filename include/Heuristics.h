#pragma once

#include "Constants.h"

namespace SquadroAI {

// Forward declaration to avoid circular dependency
class GameState;

/**
 * @class Heuristics
 * @brief A utility class containing static methods for game state evaluation.
 *
 * This keeps the evaluation logic separate from the AI's search logic,
 * promoting better code organization (Single Responsibility Principle).
 */
class Heuristics {
 public:
  // Evaluates the given game state from a specific player's perspective.
  static int evaluate(const GameState& state, PlayerID perspective_player);
};

}  // namespace SquadroAI
