#pragma once

#include <chrono>
#include "Constants.h"
#include "GameState.h"
#include "Move.h"
#include "TranspositionTable.h"
#include "Heuristics.h"

namespace SquadroAI
{

    class AIPlayer
    {
    public:
        AIPlayer(PlayerID player_id, size_t tt_size_mb = 64); // tt_size_mb: اندازه جدول انتقال به مگابایت

        // پیدا کردن بهترین حرکت برای وضعیت فعلی با محدودیت زمانی
        Move findBestMove(const GameState& initial_state, std::chrono::milliseconds time_limit);

    private:
        PlayerID my_player_id;
        TranspositionTable transposition_table;
        long long nodes_searched_total; // برای آمار

        struct MinimaxResult
        {
            int score;
            Move best_move;
            bool move_found;
        };

        // الگوریتم Minimax با هرس آلفا-بتا
        MinimaxResult minimaxAlphaBeta(GameState current_state, int depth, int alpha, int beta, bool maximizing_player,
            std::chrono::steady_clock::time_point start_time, std::chrono::milliseconds time_limit,
            int current_ply_from_root);

        // مرتب‌سازی حرکات برای بهبود کارایی هرس آلفا-بتا
        void orderMoves(std::vector<Move>& moves, const GameState& state, int depth, const std::optional<TTEntry>& tt_entry);
        int evaluateState(const GameState& state, PlayerID my_id) {
            int my_pieces = state.getCompletedPieceCount(my_id);
            PlayerID opponent_id = (my_id == PlayerID::PLAYER_1) ? PlayerID::PLAYER_2 : PlayerID::PLAYER_1;
            int opp_pieces = state.getCompletedPieceCount(opponent_id);

            return (my_pieces - opp_pieces) * 100;
        }
    };

} // namespace SquadroAI