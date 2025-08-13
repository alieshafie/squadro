#pragma once

#include <chrono>  // برای مدیریت زمان

#include "GameState.h"
#include "Move.h"
#include "MoveList.h"
#include "TranspositionTable.h"

namespace SquadroAI {

class AIPlayer {
 public:
  // سازنده: هویت بازیکن و اندازه جدول انتقال را مشخص
  // می‌کند.
  AIPlayer(PlayerID ai_id, size_t tt_size_mb = 64);

  // تابع اصلی: بهترین حرکت را در زمان مشخص شده پیدا
  // می‌کند.
  Move findBestMove(const GameState& initial_state, int time_limit_ms);

  // Getter methods for statistics
  long long getNodesVisited() const { return nodes_visited; }
  long long getTTHits() const { return transposition_table.hits; }
  void resetStatistics() {
    nodes_visited = 0;
    transposition_table.hits = 0;
  }

 private:
  // الگوریتم اصلی جستجوی Minimax با هرس آلفا-بتا و جدول انتقال
  int alphaBeta(GameState& state, int depth, int alpha, int beta,
                bool maximizing_player, Move* best_move = nullptr);

  // Quiescence search to handle tactical positions
  int quiesce(GameState& state, int alpha, int beta, bool maximizing_player,
              int depth_left);

  PlayerID ai_player_id;                   // هویت این AI (بازیکن ۱ یا ۲)
  TranspositionTable transposition_table;  // جدول انتقال اختصاصی این AI
  Move best_move_this_iteration;           // بهترین حرکت پیدا شده در عمق فعلی

  // History heuristic tables for move ordering
  static constexpr int MAX_PIECE_IDX = NUM_PIECES;
  std::array<std::array<int, NUM_ROWS * NUM_COLS>, MAX_PIECE_IDX> history_table;

  // Killer moves for each ply
  static constexpr int MAX_PLY = 64;  // Maximum search depth
  std::array<std::array<Move, 2>, MAX_PLY> killer_moves;

  // برای مدیریت زمان
  std::chrono::steady_clock::time_point start_time;
  int time_limit;
  bool time_is_up;

  // برای آمار و دیباگ
  long long nodes_visited;

  // Helper methods for move ordering
  void updateHistoryScore(const Board::AppliedMoveInfo& move_info, int depth);
  void updateKillerMove(const Move& move, int ply);
  int getMoveScore(const Move& move, int ply, const GameState& state) const;
  void sortMoves(MoveList& moves, int ply, const GameState& state) const;
};

}  // namespace SquadroAI
