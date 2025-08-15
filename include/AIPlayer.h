#pragma once

#include <array>
#include <chrono>
#include <vector>

#include "GameState.h"
#include "Move.h"
#include "MoveList.h"
#include "TranspositionTable.h"

namespace SquadroAI {

struct ScoredMove {
  Move move;
  int score;
  bool operator>(const ScoredMove& other) const { return score > other.score; }
};

class AIPlayer {
 public:
  AIPlayer(PlayerID ai_id, size_t tt_size_mb = 64);
  Move findBestMove(const GameState& initial_state, int time_limit_ms);

  long long getNodesVisited() const { return nodes_visited; }
  void resetStatistics() {
    nodes_visited = 0;
    transposition_table.hits = 0;
  }
  long long getTTHits() const { return transposition_table.hits; }

 private:
  int alphaBeta(GameState& state, int depth, int alpha, int beta,
                bool maximizing_player, Move* best_move);
  int quiesce(GameState& state, int alpha, int beta, bool maximizing_player,
              int depth_left);

  void updateHistoryScore(const Board::AppliedMoveInfo& move_info, int depth);
  void updateKillerMove(const Move& move, int ply);
  int getMoveScore(const Move& move, int ply, const GameState& state) const;
  void sortMoves(MoveList& moves, int ply, const GameState& state,
                 const Move& tt_best_move);

  PlayerID ai_player_id;
  TranspositionTable transposition_table;
  Move best_move_this_iteration;

  static constexpr int MAX_PIECE_IDX = NUM_PIECES;
  std::array<std::array<int, NUM_ROWS * NUM_COLS>, MAX_PIECE_IDX> history_table;

  static constexpr int MAX_PLY = 64;
  std::array<std::array<Move, 2>, MAX_PLY> killer_moves;

  std::chrono::steady_clock::time_point start_time;
  int time_limit;
  bool time_is_up;

  long long nodes_visited;
};

}  // namespace SquadroAI
