#pragma once

#include "Constants.h"

namespace SquadroAI {

class GameState; // Forward declaration

class Heuristics {
public:
    // ارزیابی وضعیت بازی از دید بازیکن ai_player_id
    // امتیاز مثبت به معنای برتری ai_player_id است.
    static int evaluate(const GameState& state, PlayerID ai_player_id);

private:
    // توابع کمکی برای محاسبه مولفه‌های مختلف هیوریستیک
    // static int calculateMaterialScore(const GameState& state, PlayerID ai_player_id);
    // static int calculateProgressScore(const GameState& state, PlayerID ai_player_id);
    // static int calculateMobilityScore(const GameState& state, PlayerID ai_player_id);
    //...
};

} // namespace SquadroAI