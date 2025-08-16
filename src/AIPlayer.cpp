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
  if (initial_state.isGameOver()) {
    return Move();  // Return a default, invalid move if the game is already
                    // over.
  }
  start_time = std::chrono::steady_clock::now();
  time_limit = time_limit_ms;
  time_is_up = false;
  nodes_visited = 0;
  transposition_table.hits = 0;

  Move best_move_overall(-1);
  int max_depth = 1;

  // This move will hold the best move found in the current iteration.
  Move best_move_this_iteration(-1);

  while (true) {
    GameState root_state = initial_state;
    // Pass the same move object to be updated by the search
    int score = alphaBeta(root_state, max_depth, LOSS_SCORE - 1, WIN_SCORE + 1,
                          true, &best_move_this_iteration);

    // After the search, check if time is up.
    // Crucially, we check *after* the search returns, so
    // best_move_this_iteration might have been updated.
    if (time_is_up) {
      std::cout << "Time is up." << std::endl;
      // If the interrupted search found a better move, use it.
      // Otherwise, we'll stick with the best move from the last fully completed
      // depth.
      if (best_move_this_iteration.id != -1) {
        best_move_overall = best_move_this_iteration;
      }
      break;
    }

    // If the search completed without timeout, we trust its result.
    if (best_move_this_iteration.id != -1) {
      best_move_overall = best_move_this_iteration;
      std::cout << "Depth " << max_depth
                << " finished. Best move: " << best_move_overall.to_string()
                << " Score: " << score << std::endl;
    } else {
      std::cout << "Depth " << max_depth
                << " finished. No move found. Score: " << score << std::endl;
    }

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

  // Initialize best_local_move with the first legal move. This ensures that
  // even if all moves fail high/low, we have at least one move to return.
  Move best_local_move = legal_moves[0];
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
        // Always update the root's best move pointer as soon as we find a
        // better move.
        if (best_move) {
          *best_move = best_local_move;
        }
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
        // Always update the root's best move pointer as soon as we find a
        // better move.
        if (best_move) {
          *best_move = best_local_move;
        }
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

void AIPlayer::sortMoves(MoveList& moves, int ply, const GameState& state,
                         const Move& tt_best_move) {
  // OPTIMIZATION: Use a stack-allocated std::array to avoid heap allocation.
  std::array<ScoredMove, PIECES_PER_PLAYER> scored_moves;

  const Board& board = state.getBoard();  // Get a reference to the board
  PlayerID current_player = state.getCurrentPlayer();
  size_t num_moves = moves.size();

  for (size_t i = 0; i < num_moves; ++i) {
    const auto& move = moves[i];
    int score = 0;
    if (tt_best_move.id != -1 && move.id == tt_best_move.id) {
      score = 10000000;  // PV move has highest priority
    } else {
      // Get move properties once to avoid redundant calculations
      Board::MoveProperties props =
          board.getMoveProperties(move, current_player);
      if (props.is_capture) {
        score = 9000000;  // Capture bonus
      } else {
        if (ply < MAX_PLY) {
          if (killer_moves[ply][0].id == move.id)
            score = 8000000;
          else if (killer_moves[ply][1].id == move.id)
            score = 7000000;
          else {
            // History score for quiet moves
            int piece_idx = move.id;
            if (piece_idx >= 0 && piece_idx < MAX_PIECE_IDX) {
              const auto& piece = board.getPiece(piece_idx);
              int pos_idx = piece.row * NUM_COLS + piece.col;
              if (pos_idx >= 0 && pos_idx < NUM_ROWS * NUM_COLS) {
                score = history_table[piece_idx][pos_idx];
              }
            }
          }
        }
      }
    }
    scored_moves[i] = {move, score};
  }

  // Sort only the part of the array that contains valid moves
  std::sort(scored_moves.begin(), scored_moves.begin() + num_moves,
            std::greater<ScoredMove>());

  for (size_t i = 0; i < num_moves; ++i) {
    moves[i] = scored_moves[i].move;
  }
}

}  // namespace SquadroAI
