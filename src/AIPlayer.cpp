#include "AIPlayer.h"

#include <algorithm>  // برای std::sort
#include <climits>
#include <iostream>

#include "GameStateHasher.h"
#include "Heuristics.h"

namespace SquadroAI {

AIPlayer::AIPlayer(PlayerID ai_id, size_t tt_size_mb)
    : ai_player_id(ai_id), transposition_table(tt_size_mb) {
  // اطمینان از مقداردهی اولیه جدول هش Zobrist
  GameStateHasher::initialize();

  // Clear history and killer move tables
  history_table.fill({});
  killer_moves.fill({});
}

Move AIPlayer::findBestMove(const GameState& initial_state, int time_limit_ms) {
  start_time = std::chrono::steady_clock::now();
  time_limit = time_limit_ms;
  time_is_up = false;
  nodes_visited = 0;

  Move best_move_overall(-1);  // Initialize to invalid move
  int max_depth = 1;
  int completed_depth = 0;
  std::cout << "AI Player " << (ai_player_id == PlayerID::PLAYER_1 ? "1" : "2")
            << " thinking..." << std::endl;

  // Get initial legal moves - if only one move is possible, return it
  // immediately
  auto legal_moves = initial_state.generateLegalMoves();
  if (legal_moves.size() == 1) {
    return legal_moves[0];
  }

  // Iterative Deepening (IDDFS) Loop
  while (true) {
    transposition_table.clear();  // Clear TT for each new depth

    std::cout << "Searching depth: " << max_depth << std::endl;

    GameState root_state = initial_state;  // یک کپی برای جستجو
    Move root_best_move;                   // Track best move at root
    int score = alphaBeta(root_state, max_depth, LOSS_SCORE - 1, WIN_SCORE + 1,
                          true, &root_best_move);

    if (time_is_up) {
      auto end_time = std::chrono::steady_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                          end_time - start_time)
                          .count();
      double nodes_per_second = (nodes_visited * 1000.0) / duration;

      std::cout << "Time up at " << duration << "ms. Depth " << completed_depth
                << " completed. Nodes: " << nodes_visited << " ("
                << static_cast<int>(nodes_per_second) << " N/s)" << std::endl;
      break;
    }

    // Update best move only if we got a valid move at this depth
    if (root_best_move.id >= 0) {
      best_move_this_iteration = root_best_move;
    }

    // Update best move if search completed
    completed_depth = max_depth;
    best_move_overall = best_move_this_iteration;

    auto current_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        current_time - start_time)
                        .count();
    double nodes_per_second = (nodes_visited * 1000.0) / duration;

    std::cout << "Depth " << max_depth << " complete. Score: " << score
              << ". Move: piece " << best_move_overall.id
              << ". Nodes: " << nodes_visited << " ("
              << static_cast<int>(nodes_per_second) << " N/s)"
              << ". TT Hits: " << transposition_table.hits << std::endl;

    // اگر بازی با این حرکت تمام می‌شود، نیازی
    // به جستجوی عمیق‌تر نیست
    if (abs(score) >= WIN_SCORE - max_depth) {
      std::cout << "Found a winning/losing move. Halting search." << std::endl;
      break;
    }

    max_depth++;
  }

  // Verify final move is valid
  if (best_move_overall.id >= 0) {
    return best_move_overall;
  }

  // If no valid move was found or saved, return first legal move
  std::cout << "WARNING: Using fallback move selection" << std::endl;
  if (!legal_moves.empty()) {
    return legal_moves[0];
  }

  throw std::logic_error("No legal moves available!");
}

int AIPlayer::alphaBeta(GameState& state, int depth, int alpha, int beta,
                        bool maximizing_player, Move* best_move) {
  // --- 1. بررسی شرایط توقف ---
  auto current_time = std::chrono::steady_clock::now();
  if (std::chrono::duration_cast<std::chrono::milliseconds>(current_time -
                                                            start_time)
          .count() > time_limit - 50) {  // 50ms margin
    time_is_up = true;
    return 0;  // مقدار بازگشتی مهم نیست، چون از آن استفاده
               // نمی‌شود.
  }

  if (state.isGameOver()) {
    nodes_visited++;
    return Heuristics::evaluate(state, ai_player_id);
  }

  if (depth == 0) {
    // At leaf nodes, enter quiescence search to handle tactical positions
    return quiesce(state, alpha, beta, 0);
  }

  uint64_t key =
      GameStateHasher::computeHash(state.getBoard(), state.getCurrentPlayer());
  Move tt_best_move;
  int tt_score;

  // --- 2. جستجو در جدول انتقال (Transposition Table) ---
  if (transposition_table.probe(key, depth, tt_score, tt_best_move)) {
    if (tt_score != INT_MIN) {  // Valid score found
      if (tt_score >= beta && !maximizing_player)
        return tt_score;  // Beta cutoff
      if (tt_score <= alpha && maximizing_player)
        return tt_score;  // Alpha cutoff
    }
  }

  // NOTE: Null move pruning removed as it's not suitable for Squadro
  // In Squadro, players must move, and position evaluation is tied to piece
  // progress

  // --- 3. تولید و مرتب‌سازی حرکات ---
  auto legal_moves = state.generateLegalMoves();

  // Sort moves based on multiple heuristics
  sortMoves(legal_moves, depth, state);

  Move best_move_found;

  // --- 4. حلقه اصلی آلفا-بتا ---
  if (maximizing_player) {
    int max_eval = LOSS_SCORE - 1;
    Move best_local_move;  // Track best move at this node

    for (const auto& move : legal_moves) {
      GameState next_state = state.createNextState(move);
      int eval = alphaBeta(next_state, depth - 1, alpha, beta, false, nullptr);

      if (time_is_up) return 0;  // خروج سریع

      if (eval > max_eval) {
        max_eval = eval;
        best_local_move = move;
        // At root, update the best move pointer if provided
        if (best_move != nullptr) {
          *best_move = move;
        }
      }
      alpha = std::max(alpha, eval);
      if (beta <= alpha) {
        // Beta cutoff - this is a good move, save it as a killer
        updateKillerMove(move, depth);
        updateHistoryScore(move, depth);
        break;  // Beta cutoff
      }
    }

    // Store the result with correct bound type
    EntryType entry_type;
    if (max_eval <= alpha)
      entry_type = EntryType::UPPER_BOUND;
    else if (max_eval >= beta)
      entry_type = EntryType::LOWER_BOUND;
    else
      entry_type = EntryType::EXACT;

    // Store the best move we found at this node
    transposition_table.store(key, depth, max_eval, entry_type,
                              best_local_move);
    return max_eval;
  } else {  // Minimizing Player
    int min_eval = WIN_SCORE + 1;
    Move best_local_move;  // Track best move at this node

    for (const auto& move : legal_moves) {
      GameState next_state = state.createNextState(move);
      int eval = alphaBeta(next_state, depth - 1, alpha, beta, true, nullptr);

      if (time_is_up) return 0;  // خروج سریع

      if (eval < min_eval) {
        min_eval = eval;
        best_local_move = move;
        // At root, update the best move pointer if provided
        if (best_move != nullptr) {
          *best_move = move;
        }
      }
      beta = std::min(beta, eval);
      if (beta <= alpha) {
        // Alpha cutoff - this is a good move, save it as a killer
        updateKillerMove(move, depth);
        updateHistoryScore(move, depth);
        break;  // Alpha cutoff
      }
    }

    // Store the result with correct bound type
    EntryType entry_type;
    if (min_eval <= alpha)
      entry_type = EntryType::UPPER_BOUND;
    else if (min_eval >= beta)
      entry_type = EntryType::LOWER_BOUND;
    else
      entry_type = EntryType::EXACT;

    // Store the best move we found at this node
    transposition_table.store(key, depth, min_eval, entry_type,
                              best_local_move);
    return min_eval;
  }

  // If no legal moves and not game over, return evaluation of current position
  if (legal_moves.empty() && !state.isGameOver()) {
    return Heuristics::evaluate(state, ai_player_id);
  }

  // Return the eval based on maximizing_player
  return maximizing_player ? LOSS_SCORE - 1 : WIN_SCORE + 1;
}

void AIPlayer::updateHistoryScore(const Move& move, int depth) {
  int piece_idx = move.id;
  if (piece_idx >= 0 && piece_idx < MAX_PIECE_IDX) {
    // Map board position to 1D index
    int pos_idx = move.id * NUM_COLS;
    history_table[piece_idx][pos_idx] += depth * depth;
  }
}

void AIPlayer::updateKillerMove(const Move& move, int ply) {
  if (ply >= MAX_PLY) return;
  if (move.id == -1) return;  // Don't store null moves

  // Only store if it's different from existing killers
  if (killer_moves[ply][0].id != move.id) {
    killer_moves[ply][1] = killer_moves[ply][0];  // Shift existing killer
    killer_moves[ply][0] = move;                  // Store new killer
  }
}

int AIPlayer::getMoveScore(const Move& move, int ply,
                           const GameState& state) const {
  constexpr int HASH_MOVE_BONUS = 10000000;
  constexpr int KILLER_FIRST_BONUS = 9000000;
  constexpr int KILLER_SECOND_BONUS = 8000000;

  int score = 0;

  // Hash move bonus handled in sortMoves

  // Killer moves get next priority
  if (ply < MAX_PLY) {
    if (killer_moves[ply][0].id == move.id) {
      return KILLER_FIRST_BONUS;
    }
    if (killer_moves[ply][1].id == move.id) {
      return KILLER_SECOND_BONUS;
    }
  }

  // History heuristic provides final ordering
  int piece_idx = move.id;
  if (piece_idx >= 0 && piece_idx < MAX_PIECE_IDX) {
    int pos_idx = move.id * NUM_COLS;
    score += history_table[piece_idx][pos_idx];
  }

  return score;
}

void AIPlayer::sortMoves(std::vector<Move>& moves, int ply,
                         const GameState& state) const {
  const Move* tt_move = nullptr;  // Will be set if TT entry exists

  // Try to get TT move
  uint64_t key =
      GameStateHasher::computeHash(state.getBoard(), state.getCurrentPlayer());
  Move tt_best_move;
  int dummy_score;
  if (transposition_table.probe(key, ply, dummy_score, tt_best_move)) {
    tt_move = &tt_best_move;
  }

  // Sort moves
  std::sort(moves.begin(), moves.end(),
            [this, ply, &state, tt_move](const Move& a, const Move& b) {
              // TT move always comes first
              if (tt_move && tt_move->id == a.id) return true;
              if (tt_move && tt_move->id == b.id) return false;

              return getMoveScore(a, ply, state) > getMoveScore(b, ply, state);
            });
}

int AIPlayer::quiesce(GameState& state, int alpha, int beta, int depth_left) {
  nodes_visited++;

  // Check for game over first
  if (state.isGameOver()) {
    return Heuristics::evaluate(state, ai_player_id);
  }

  // Basic static evaluation
  int stand_pat = Heuristics::evaluate(state, ai_player_id);

  // Return immediately if we're too deep
  if (depth_left <= -4) {  // Limit quiescence depth
    return stand_pat;
  }

  // Beta cutoff
  if (stand_pat >= beta) {
    return beta;
  }

  // Update alpha if standing pat is better
  if (stand_pat > alpha) {
    alpha = stand_pat;
  }

  // Generate legal moves (TODO: Add generateCapturingMoves())
  auto moves = state.generateLegalMoves();
  if (moves.empty()) {
    return stand_pat;  // No moves available
  }

  sortMoves(moves, depth_left, state);

  for (const auto& move : moves) {
    // Create next state and validate
    GameState next_state = state.createNextState(move);
    if (time_is_up) return alpha;  // Check for timeout

    int score = -quiesce(next_state, -beta, -alpha, depth_left - 1);

    if (score >= beta) {
      return beta;
    }
    if (score > alpha) {
      alpha = score;
    }
  }

  return alpha;
}

}  // namespace SquadroAI
