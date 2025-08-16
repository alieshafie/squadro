#include <chrono>
#include <iostream>
#include <vector>

#include "AIPlayer.h"
#include "GameState.h"

using namespace SquadroAI;

struct GameResult {
  PlayerID winner;
  int total_moves;
  long long total_nodes_p1;
  long long total_nodes_p2;
  long long total_tt_hits_p1;
  long long total_tt_hits_p2;
  double total_time_p1;
  double total_time_p2;
  double avg_nodes_per_second_p1;
  double avg_nodes_per_second_p2;
  double avg_time_per_move_p1;
  double avg_time_per_move_p2;
};

GameResult playSelfPlay(int time_limit_ms) {
  GameState state;
  AIPlayer player1(PlayerID::PLAYER_1, 64);  // 64MB TT
  AIPlayer player2(PlayerID::PLAYER_2, 64);  // 64MB TT

  int total_moves = 0;
  int moves_p1 = 0, moves_p2 = 0;
  double total_time_p1 = 0.0, total_time_p2 = 0.0;

  std::cout << "\nStarting new game with " << time_limit_ms << "ms per move"
            << std::endl;
  std::cout << "----------------------------------------" << std::endl;

  // Reset statistics at the start of each game
  player1.resetStatistics();
  player2.resetStatistics();
  long long cumulative_nodes_p1 = 0;
  long long cumulative_nodes_p2 = 0;
  long long cumulative_tt_hits_p1 = 0;
  long long cumulative_tt_hits_p2 = 0;

  while (!state.isGameOver()) {
    PlayerID current_player_id = state.getCurrentPlayer();
    AIPlayer& current_player =
        (current_player_id == PlayerID::PLAYER_1) ? player1 : player2;

    auto start = std::chrono::steady_clock::now();
    Move best_move = current_player.findBestMove(state, time_limit_ms);
    auto end = std::chrono::steady_clock::now();

    if (best_move.id == -1) {
      std::cout << "AI returned no valid move (game likely over)." << std::endl;
      break;  // Exit the loop if no move is found
    }

    // Calculate and track move time
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    double time_ms = static_cast<double>(duration.count());

    if (current_player_id == PlayerID::PLAYER_1) {
      total_time_p1 += time_ms;
      moves_p1++;
      // getNodesVisited() returns the count for the last findBestMove call.
      cumulative_nodes_p1 += player1.getNodesVisited();
      // getTTHits() is cumulative for the lifetime of the object.
      cumulative_tt_hits_p1 = player1.getTTHits();
    } else {
      total_time_p2 += time_ms;
      moves_p2++;
      cumulative_nodes_p2 += player2.getNodesVisited();
      cumulative_tt_hits_p2 = player2.getTTHits();
    }

    std::cout << "Player "
              << (current_player_id == PlayerID::PLAYER_1 ? "1" : "2")
              << " move time: " << duration.count() << "ms" << std::endl;

    state.applyMove(best_move);
    total_moves++;

    // Print board state (commented out for performance benchmarking)
    // state.getBoard().printBoard();
    // std::cout << "----------------------------------------" << std::endl;
  }

  PlayerID winner = state.getWinner();
  std::cout << "Game Over! Winner: Player "
            << (winner == PlayerID::PLAYER_1 ? "1" : "2") << std::endl;
  std::cout << "Total moves: " << total_moves << std::endl;

  // Collect final statistics from our cumulative counters
  long long& nodes_p1 = cumulative_nodes_p1;
  long long& nodes_p2 = cumulative_nodes_p2;
  long long& tt_hits_p1 = cumulative_tt_hits_p1;
  long long& tt_hits_p2 = cumulative_tt_hits_p2;

  // Calculate averages
  double avg_nps_p1 =
      (total_time_p1 > 0)
          ? (static_cast<double>(nodes_p1) * 1000.0) / total_time_p1
          : 0.0;
  double avg_nps_p2 =
      (total_time_p2 > 0)
          ? (static_cast<double>(nodes_p2) * 1000.0) / total_time_p2
          : 0.0;
  double avg_time_p1 = (moves_p1 > 0) ? total_time_p1 / moves_p1 : 0.0;
  double avg_time_p2 = (moves_p2 > 0) ? total_time_p2 / moves_p2 : 0.0;

  // Display game statistics
  std::cout << "\n=== Game Statistics ===" << std::endl;
  std::cout << "Player 1: " << moves_p1 << " moves, " << nodes_p1 << " nodes, "
            << tt_hits_p1 << " TT hits" << std::endl;
  std::cout << "  Avg time: " << static_cast<int>(avg_time_p1) << "ms, "
            << "Avg NPS: " << static_cast<int>(avg_nps_p1) << std::endl;
  std::cout << "Player 2: " << moves_p2 << " moves, " << nodes_p2 << " nodes, "
            << tt_hits_p2 << " TT hits" << std::endl;
  std::cout << "  Avg time: " << static_cast<int>(avg_time_p2) << "ms, "
            << "Avg NPS: " << static_cast<int>(avg_nps_p2) << std::endl;

  return GameResult{winner,     total_moves, nodes_p1,      nodes_p2,
                    tt_hits_p1, tt_hits_p2,  total_time_p1, total_time_p2,
                    avg_nps_p1, avg_nps_p2,  avg_time_p1,   avg_time_p2};
}

int main() {
  const int NUM_GAMES = 1;
  const int TIME_PER_MOVE_MS = 1000;  // 1 second

  std::vector<GameResult> results;
  try {
    for (int i = 0; i < NUM_GAMES; i++) {
      std::cout << "\n=== Starting Game " << (i + 1) << " ===" << std::endl;
      results.push_back(playSelfPlay(TIME_PER_MOVE_MS));
    }
  } catch (const std::exception& e) {
    std::cerr << "An unhandled exception occurred: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "An unknown unhandled exception occurred." << std::endl;
    return 1;
  }

  // Calculate comprehensive summary statistics
  int player1_wins = 0;
  int player2_wins = 0;
  int total_moves = 0;
  long long total_nodes_p1 = 0, total_nodes_p2 = 0;
  long long total_tt_hits_p1 = 0, total_tt_hits_p2 = 0;
  double total_time_all_p1 = 0.0, total_time_all_p2 = 0.0;
  double total_nps_p1 = 0.0, total_nps_p2 = 0.0;
  double total_avg_time_p1 = 0.0, total_avg_time_p2 = 0.0;

  for (const auto& result : results) {
    if (result.winner == PlayerID::PLAYER_1) player1_wins++;
    if (result.winner == PlayerID::PLAYER_2) player2_wins++;
    total_moves += result.total_moves;

    total_nodes_p1 += result.total_nodes_p1;
    total_nodes_p2 += result.total_nodes_p2;
    // The result already contains the cumulative total, so just assign from the
    // last game.
    if (!results.empty()) {
      total_tt_hits_p1 = results.back().total_tt_hits_p1;
      total_tt_hits_p2 = results.back().total_tt_hits_p2;
    }
    total_time_all_p1 += result.total_time_p1;
    total_time_all_p2 += result.total_time_p2;
    total_nps_p1 += result.avg_nodes_per_second_p1;
    total_nps_p2 += result.avg_nodes_per_second_p2;
    total_avg_time_p1 += result.avg_time_per_move_p1;
    total_avg_time_p2 += result.avg_time_per_move_p2;
  }

  std::cout << "\n=== Complete Test Summary ===" << std::endl;
  std::cout << "Games played: " << NUM_GAMES << std::endl;
  std::cout << "Player 1 wins: " << player1_wins << " ("
            << (player1_wins * 100 / NUM_GAMES) << "%)" << std::endl;
  std::cout << "Player 2 wins: " << player2_wins << " ("
            << (player2_wins * 100 / NUM_GAMES) << "%)" << std::endl;
  std::cout << "Average moves per game: " << (total_moves / NUM_GAMES)
            << std::endl;

  std::cout << "\n=== Performance Summary ===" << std::endl;
  std::cout << "Player 1 totals:" << std::endl;
  std::cout << "  Total nodes: " << total_nodes_p1 << std::endl;
  std::cout << "  Total TT hits: " << total_tt_hits_p1 << std::endl;
  std::cout << "  Total time: " << static_cast<int>(total_time_all_p1) << "ms"
            << std::endl;
  std::cout << "  Avg NPS: " << static_cast<int>(total_nps_p1 / NUM_GAMES)
            << std::endl;
  std::cout << "  Avg time per move: "
            << static_cast<int>(total_avg_time_p1 / NUM_GAMES) << "ms"
            << std::endl;

  std::cout << "Player 2 totals:" << std::endl;
  std::cout << "  Total nodes: " << total_nodes_p2 << std::endl;
  std::cout << "  Total TT hits: " << total_tt_hits_p2 << std::endl;
  std::cout << "  Total time: " << static_cast<int>(total_time_all_p2) << "ms"
            << std::endl;
  std::cout << "  Avg NPS: " << static_cast<int>(total_nps_p2 / NUM_GAMES)
            << std::endl;
  std::cout << "  Avg time per move: "
            << static_cast<int>(total_avg_time_p2 / NUM_GAMES) << "ms"
            << std::endl;

  // Calculate TT hit rates
  double tt_hit_rate_p1 = (total_nodes_p1 > 0)
                              ? (static_cast<double>(total_tt_hits_p1) /
                                 static_cast<double>(total_nodes_p1) * 100.0)
                              : 0.0;
  double tt_hit_rate_p2 = (total_nodes_p2 > 0)
                              ? (static_cast<double>(total_tt_hits_p2) /
                                 static_cast<double>(total_nodes_p2) * 100.0)
                              : 0.0;

  std::cout << "\n=== Efficiency Metrics ===" << std::endl;
  std::cout << "Player 1 TT hit rate: " << static_cast<int>(tt_hit_rate_p1)
            << "%" << std::endl;
  std::cout << "Player 2 TT hit rate: " << static_cast<int>(tt_hit_rate_p2)
            << "%" << std::endl;

  return 0;
}
