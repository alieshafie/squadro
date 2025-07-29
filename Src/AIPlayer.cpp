#include "AIPlayer.h"
#include <iostream>

SquadroAI::AIPlayer::AIPlayer(PlayerID player_id, size_t tt_size_mb = 64)
    : my_player_id{ player_id }
    , transposition_table(tt_size_mb)
    , nodes_searched_total(0)
{
}

SquadroAI::Move SquadroAI::AIPlayer::findBestMove(const GameState& initial_state, std::chrono::milliseconds time_limit) {
    // using namespace std::chrono;

    // auto start_time = steady_clock::now();
    // nodes_searched_total = 0;

    Move best_move(1);
    // bool move_found = false;
    // int depth = 1;

    // while (true) {
    //     auto now = steady_clock::now();
    //     if (now - start_time >= time_limit) break;

    //     MinimaxResult result = minimaxAlphaBeta(
    //         initial_state, depth,
    //         -1000000, 1000000,
    //         true,
    //         start_time,
    //         time_limit,
    //         0
    //     );

    //     if (result.move_found) {
    //         best_move = result.best_move;
    //         move_found = true;
    //     }
    //     else {
    //         break; // زمان کافی برای کامل کردن این عمق وجود نداشت
    //     }

    //     depth++;
    // }

    // std::cout << "[AIPlayer] Nodes searched: " << nodes_searched_total << ", Depth reached: " << (depth - 1) << std::endl;

    // return best_move;

    return best_move;
}

// SquadroAI::AIPlayer::MinimaxResult SquadroAI::AIPlayer::minimaxAlphaBeta(GameState current_state, int depth, int alpha, int beta,
//     bool maximizing_player,
//     std::chrono::steady_clock::time_point start_time,
//     std::chrono::milliseconds time_limit,
//     int current_ply_from_root)
// {
//     nodes_searched_total++;

//     // بررسی زمان
//     if (std::chrono::steady_clock::now() - start_time >= time_limit) {
//         return { 0, NULL_MOVE, false };
//     }

//     // شرط پایان
//     if (depth == 0 || current_state.isGameOver()) {
//         int eval = evaluateState(current_state, my_player_id);
//         return { eval, NULL_MOVE, false };
//     }

//     std::vector<Move> legal_moves = current_state.generateLegalMoves();
//     if (legal_moves.empty()) {
//         int eval = evaluateState(current_state, my_player_id);
//         return { eval, NULL_MOVE, false };
//     }

//     Move best_move = legal_moves[0];
//     int best_score = maximizing_player ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();

//     for (const Move& move : legal_moves) {
//         GameState child = current_state.createChildState(move);
//         child.switchPlayer();

//         MinimaxResult result = minimaxAlphaBeta(child, depth - 1, alpha, beta,
//             !maximizing_player, start_time, time_limit, current_ply_from_root + 1);

//         if (maximizing_player) {
//             if (result.score > best_score) {
//                 best_score = result.score;
//                 best_move = move;
//             }
//             alpha = std::max(alpha, result.score);
//         }
//         else {
//             if (result.score < best_score) {
//                 best_score = result.score;
//                 best_move = move;
//             }
//             beta = std::min(beta, result.score);
//         }

//         if (beta <= alpha) break; // alpha-beta pruning
//     }

//     return { best_score, best_move, true };
// }

// void SquadroAI::AIPlayer::orderMoves(std::vector<Move>& moves, const GameState& state, int depth, const std::optional<TTEntry>& tt_entry) {}
