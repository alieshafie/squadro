#include "AIPlayer.h"

#include <algorithm>  // for std::sort
#include <climits>
#include <iostream>

#include "GameStateHasher.h"
#include "Heuristics.h"
#include "MoveList.h"  // Include the new MoveList header

namespace SquadroAI {

AIPlayer::AIPlayer(PlayerID ai_id, size_t tt_size_mb)
    : ai_player_id(ai_id), transposition_table(tt_size_mb) {
  GameStateHasher::initialize();
  history_table.fill({});
  killer_moves.fill({});
}

Move AIPlayer::findBestMove(const GameState& initial_state, int time_limit_ms) {
  start_time = std::chrono::steady_clock::now();
  time_limit = time_limit_ms;
  time_is_up = false;
  nodes_visited = 0;

  Move best_move_overall(-1);
  int max_depth = 1;
  int completed_depth = 0;
  std::cout << "AI Player " << (ai_player_id == PlayerID::PLAYER_1 ? "1" : "2")
            << " thinking..." << std::endl;

  MoveList legal_moves;
  initial_state.generateLegalMoves(legal_moves);
  if (legal_moves.size() == 1) {
    return legal_moves[0];
  }

  while (true) {
    // transposition_table.clear(); // Clearing TT between depths can be
    // suboptimal

    std::cout << "Searching depth: " << max_depth << std::endl;

    GameState root_state = initial_state;
    Move root_best_move;
    int score = alphaBeta(root_state, max_depth, LOSS_SCORE - 1, WIN_SCORE + 1,
                          true, &root_best_move);

    auto duration_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start_time)
            .count();

    if (time_is_up) {
      double nps = (nodes_visited * 1000.0) /
                   std::max(1LL, static_cast<long long>(duration_ms));
      std::cout << "Time up at " << duration_ms << "ms. Depth "
                << completed_depth << " completed. Nodes: " << nodes_visited
                << " (" << static_cast<int>(nps) << " N/s)" << std::endl;
      break;
    }

    if (root_best_move.id >= 0) {
      best_move_this_iteration = root_best_move;
    }

    completed_depth = max_depth;
    best_move_overall = best_move_this_iteration;

    double nps = (nodes_visited * 1000.0) /
                 std::max(1LL, static_cast<long long>(duration_ms));
    std::cout << "Depth " << max_depth << " complete. Score: " << score
              << ". Move: piece " << best_move_overall.id
              << ". Nodes: " << nodes_visited << " (" << static_cast<int>(nps)
              << " N/s)"
              << ". TT Hits: " << transposition_table.hits << std::endl;

    if (abs(score) >= WIN_SCORE - max_depth) {
      std::cout << "Found a winning/losing move. Halting search." << std::endl;
      break;
    }

    max_depth++;
  }

  if (best_move_overall.id >= 0) {
    return best_move_overall;
  }

  std::cout << "WARNING: Using fallback move selection" << std::endl;
  if (!legal_moves.empty()) {
    return legal_moves[0];
  }

  throw std::logic_error("No legal moves available!");
}

int AIPlayer::alphaBeta(GameState& state, int depth, int alpha, int beta,
                        bool maximizing_player, Move* best_move) {
  if (std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now() - start_time)
          .count() > time_limit - 50) {
    time_is_up = true;
    return 0;
  }

  if (state.isGameOver()) {
    nodes_visited++;
    return Heuristics::evaluate(state, ai_player_id);
  }

  if (depth == 0) {
    return quiesce(state, alpha, beta, maximizing_player, 0);
  }

  uint64_t key =
      GameStateHasher::computeHash(state.getBoard(), state.getCurrentPlayer());
  Move tt_best_move;
  int tt_score;
  if (transposition_table.probe(key, depth, tt_score, tt_best_move)) {
    if (tt_score != INT_MIN) {
      // Basic score check for cutoff
      if (maximizing_player) {
        if (tt_score >= beta) return tt_score;
      } else {
        if (tt_score <= alpha) return tt_score;
      }
    }
  }

  MoveList legal_moves;
  state.generateLegalMoves(legal_moves);
  sortMoves(legal_moves, depth, state);

  if (legal_moves.empty()) {
    return Heuristics::evaluate(state, ai_player_id);
  }

  Move best_local_move;
  int alpha_orig = alpha;

  if (maximizing_player) {
    int max_eval = LOSS_SCORE - 1;
    for (const auto& move : legal_moves) {
      auto move_info = state.applyMove(move);
      if (!move_info) continue;

      nodes_visited++;
      int eval = alphaBeta(state, depth - 1, alpha, beta, false, nullptr);
      state.undoMove(*move_info);

      if (time_is_up) return 0;

      if (eval > max_eval) {
        max_eval = eval;
        best_local_move = move;
        if (best_move != nullptr) {
          *best_move = move;
        }
      }
      alpha = std::max(alpha, eval);
      if (beta <= alpha) {
        updateKillerMove(move, depth);
        updateHistoryScore(*move_info, depth);
        break;
      }
    }

    EntryType entry_type;
    if (max_eval <= alpha_orig) {
      entry_type = EntryType::UPPER_BOUND;
    } else if (max_eval >= beta) {
      entry_type = EntryType::LOWER_BOUND;
    } else {
      entry_type = EntryType::EXACT;
    }
    transposition_table.store(key, depth, max_eval, entry_type,
                              best_local_move);

    return max_eval;
  } else {  // Minimizing Player
    int min_eval = WIN_SCORE + 1;
    for (const auto& move : legal_moves) {
      auto move_info = state.applyMove(move);
      if (!move_info) continue;

      nodes_visited++;
      int eval = alphaBeta(state, depth - 1, alpha, beta, true, nullptr);
      state.undoMove(*move_info);

      if (time_is_up) return 0;

      if (eval < min_eval) {
        min_eval = eval;
        best_local_move = move;
        if (best_move != nullptr) {
          *best_move = move;
        }
      }
      beta = std::min(beta, eval);
      if (beta <= alpha) {
        updateKillerMove(move, depth);
        updateHistoryScore(*move_info, depth);
        break;
      }
    }

    EntryType entry_type;
    if (min_eval <= alpha) {
      entry_type = EntryType::UPPER_BOUND;
    } else if (min_eval >= beta) {
      entry_type = EntryType::LOWER_BOUND;
    } else {
      entry_type = EntryType::EXACT;
    }
    transposition_table.store(key, depth, min_eval, entry_type,
                              best_local_move);

    return min_eval;
  }
}

void AIPlayer::updateHistoryScore(const Board::AppliedMoveInfo& move_info,
                                int depth) {
  int piece_idx = move_info.mover_id;
  if (piece_idx >= 0 && piece_idx < MAX_PIECE_IDX) {
    int pos_idx = move_info.dest_row * NUM_COLS + move_info.dest_col;
    if (pos_idx >= 0 && pos_idx < NUM_ROWS * NUM_COLS) {
      history_table[piece_idx][pos_idx] += depth * depth;
    }
  }
}

void AIPlayer::updateKillerMove(const Move& move, int ply) {
  if (ply >= MAX_PLY) return;
  if (move.id == -1) return;

  if (killer_moves[ply][0].id != move.id) {
    killer_moves[ply][1] = killer_moves[ply][0];
    killer_moves[ply][0] = move;
  }
}

int AIPlayer::getMoveScore(const Move& move, int ply,
                           const GameState& state) const {
  constexpr int KILLER_FIRST_BONUS = 9000000;
  constexpr int KILLER_SECOND_BONUS = 8000000;

  if (ply < MAX_PLY) {
    if (killer_moves[ply][0].id == move.id) return KILLER_FIRST_BONUS;
    if (killer_moves[ply][1].id == move.id) return KILLER_SECOND_BONUS;
  }

  int piece_idx = move.id;
  if (piece_idx >= 0 && piece_idx < MAX_PIECE_IDX) {
    const auto& piece = state.getBoard().getPiece(piece_idx);
    int pos_idx = piece.row * NUM_COLS + piece.col;
    if (pos_idx >= 0 && pos_idx < NUM_ROWS * NUM_COLS) {
      return history_table[piece_idx][pos_idx];
    }
  }
  return 0;
}

void AIPlayer::sortMoves(MoveList& moves, int ply,
                         const GameState& state) const {
  uint64_t key =
      GameStateHasher::computeHash(state.getBoard(), state.getCurrentPlayer());
  Move tt_best_move;
  int dummy_score;
  const bool tt_hit =
      transposition_table.probe(key, ply, dummy_score, tt_best_move);

  std::sort(moves.begin(), moves.end(),
            [this, ply, &state, tt_hit, &tt_best_move](const Move& a,
                                                       const Move& b) {
              if (tt_hit) {
                if (a.id == tt_best_move.id) return true;
                if (b.id == tt_best_move.id) return false;
              }
              return getMoveScore(a, ply, state) > getMoveScore(b, ply, state);
            });
}

int AIPlayer::quiesce(GameState& state, int alpha, int beta,
                      bool maximizing_player, int depth_left) {
  if (state.isGameOver()) {
    return Heuristics::evaluate(state, ai_player_id);
  }

  int stand_pat = Heuristics::evaluate(state, ai_player_id);

  if (depth_left <= -4) {
    return stand_pat;
  }

  if (maximizing_player) {
    if (stand_pat >= beta) {
      return stand_pat;
    }
    if (stand_pat > alpha) {
      alpha = stand_pat;
    }
  } else {  // Minimizing
    if (stand_pat <= alpha) {
      return stand_pat;
    }
    if (stand_pat < beta) {
      beta = stand_pat;
    }
  }

  MoveList moves;
  state.getBoard().generateCaptureMoves(state.getCurrentPlayer(), moves);
  sortMoves(moves, depth_left, state);

  if (moves.empty()) {
    return stand_pat;
  }

  for (const auto& move : moves) {
    auto move_info = state.applyMove(move);
    if (!move_info) continue;

    nodes_visited++;
    int score =
        quiesce(state, alpha, beta, !maximizing_player, depth_left - 1);
    state.undoMove(*move_info);

    if (maximizing_player) {
      if (score > alpha) {
        alpha = score;
      }
    } else {  // Minimizing
      if (score < beta) {
        beta = score;
      }
    }
    if (alpha >= beta) {
      break;  // Cutoff
    }
  }

  return maximizing_player ? alpha : beta;
}

}  // namespace SquadroAI
