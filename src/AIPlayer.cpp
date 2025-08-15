#include "AIPlayer.h"

#include <algorithm>
#include <climits>
#include <iostream>
#include <stdexcept>

#include "GameStateHasher.h"
#include "Heuristics.h"  // Include the new Heuristics header
#include "MoveList.h"

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
  transposition_table.hits = 0;

  Move best_move_overall(-1);
  int max_depth = 1;

  while (true) {
    GameState root_state = initial_state;
    Move root_best_move;
    int score = alphaBeta(root_state, max_depth, LOSS_SCORE - 1, WIN_SCORE + 1,
                          true, &root_best_move);

    if (time_is_up) {
      std::cout << "Time is up, returning best move from previous depth."
                << std::endl;
      break;
    }

    best_move_overall = root_best_move;
    std::cout << "Depth " << max_depth
              << " finished. Best move: " << best_move_overall.to_string()
              << " Score: " << score << std::endl;

    if (abs(score) >= WIN_SCORE - max_depth) {
      std::cout << "Found winning move." << std::endl;
      break;
    }
    if (max_depth > 100) break;  // Safety break
    max_depth++;
  }
  return best_move_overall;
}

int AIPlayer::alphaBeta(GameState& state, int depth, int alpha, int beta,
                        bool maximizing_player, Move* best_move) {
  if (time_is_up) return 0;
  if (std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now() - start_time)
          .count() > time_limit - 20) {
    time_is_up = true;
    return 0;
  }

  if (state.isGameOver()) {
    nodes_visited++;
    return Heuristics::evaluate(state, ai_player_id);
  }

  if (depth == 0) {
    return quiesce(state, alpha, beta, maximizing_player,
                   4);  // Quiescence search
  }

  uint64_t key =
      GameStateHasher::computeHash(state.getBoard(), state.getCurrentPlayer());
  Move tt_best_move;
  const TTEntry* tt_entry = transposition_table.probe(key);
  if (tt_entry) {
    tt_best_move = tt_entry->best_move;
    if (tt_entry->depth >= depth) {
      int score = tt_entry->score;
      if (tt_entry->type == EntryType::EXACT) return score;
      if (tt_entry->type == EntryType::LOWER_BOUND && score >= beta)
        return score;
      if (tt_entry->type == EntryType::UPPER_BOUND && score <= alpha)
        return score;
    }
  }

  MoveList legal_moves;
  state.generateLegalMoves(legal_moves);
  sortMoves(legal_moves, depth, state, tt_best_move);

  if (legal_moves.empty()) {
    return Heuristics::evaluate(state, ai_player_id);
  }

  Move best_local_move;
  int alpha_orig = alpha;
  EntryType entry_type = EntryType::UPPER_BOUND;

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
        if (best_move) *best_move = move;
      }
      alpha = std::max(alpha, eval);
      if (beta <= alpha) {
        updateKillerMove(move, depth);
        updateHistoryScore(*move_info, depth);
        entry_type = EntryType::LOWER_BOUND;
        goto cutoff;
      }
    }
    if (max_eval > alpha_orig) entry_type = EntryType::EXACT;
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
        if (best_move) *best_move = move;
      }
      beta = std::min(beta, eval);
      if (beta <= alpha) {
        updateKillerMove(move, depth);
        updateHistoryScore(*move_info, depth);
        entry_type = EntryType::LOWER_BOUND;
        goto cutoff;
      }
    }
    if (min_eval > alpha_orig) entry_type = EntryType::EXACT;
    transposition_table.store(key, depth, min_eval, entry_type,
                              best_local_move);
    return min_eval;
  }
cutoff:
  transposition_table.store(key, depth, maximizing_player ? alpha : beta,
                            entry_type, best_local_move);
  return maximizing_player ? alpha : beta;
}

int AIPlayer::quiesce(GameState& state, int alpha, int beta,
                      bool maximizing_player, int depth_left) {
  if (time_is_up) return 0;
  if (std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now() - start_time)
          .count() > time_limit - 20) {
    time_is_up = true;
    return 0;
  }

  nodes_visited++;
  int stand_pat = Heuristics::evaluate(state, ai_player_id);

  if (maximizing_player) {
    if (stand_pat >= beta) return stand_pat;
    if (stand_pat > alpha) alpha = stand_pat;
  } else {
    if (stand_pat <= alpha) return stand_pat;
    if (stand_pat < beta) beta = stand_pat;
  }

  if (depth_left == 0 || state.isGameOver()) {
    return stand_pat;
  }

  MoveList moves;
  state.getBoard().generateCaptureMoves(state.getCurrentPlayer(), moves);
  sortMoves(moves, 0, state, Move());

  for (const auto& move : moves) {
    auto move_info = state.applyMove(move);
    if (!move_info) continue;
    int score = quiesce(state, alpha, beta, !maximizing_player, depth_left - 1);
    state.undoMove(*move_info);
    if (time_is_up) return 0;

    if (maximizing_player) {
      if (score > alpha) alpha = score;
    } else {
      if (score < beta) beta = score;
    }
    if (alpha >= beta) break;
  }
  return maximizing_player ? alpha : beta;
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
  if (ply >= MAX_PLY || move.id == -1) return;
  if (killer_moves[ply][0].id != move.id) {
    killer_moves[ply][1] = killer_moves[ply][0];
    killer_moves[ply][0] = move;
  }
}

int AIPlayer::getMoveScore(const Move& move, int ply,
                           const GameState& state) const {
  if (state.getBoard().isCapture(move, state.getCurrentPlayer()))
    return 9000000;
  if (ply < MAX_PLY) {
    if (killer_moves[ply][0].id == move.id) return 8000000;
    if (killer_moves[ply][1].id == move.id) return 7000000;
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

void AIPlayer::sortMoves(MoveList& moves, int ply, const GameState& state,
                         const Move& tt_best_move) {
  std::vector<ScoredMove> scored_moves;
  scored_moves.reserve(moves.size());
  for (const auto& move : moves) {
    int score = 0;
    if (tt_best_move.id != -1 && move.id == tt_best_move.id) {
      score = 10000000;
    } else {
      score = getMoveScore(move, ply, state);
    }
    scored_moves.push_back({move, score});
  }
  std::sort(scored_moves.begin(), scored_moves.end(),
            std::greater<ScoredMove>());
  for (size_t i = 0; i < moves.size(); ++i) {
    moves[i] = scored_moves[i].move;
  }
}

}  // namespace SquadroAI
