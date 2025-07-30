#include <chrono>
#include <iostream>
#include <vector>

#include "AIPlayer.h"
#include "GameState.h"

using namespace SquadroAI;

struct GameResult {
    PlayerID winner;
    int total_moves;
    int total_nodes;
    double avg_depth;
    double avg_nodes_per_second;
};

GameResult playSelfPlay(int time_limit_ms) {
    GameState state;
    AIPlayer player1(PlayerID::PLAYER_1);
    AIPlayer player2(PlayerID::PLAYER_2);
    
    int total_moves = 0;
    int64_t total_nodes = 0;
    int total_depth = 0;
    double total_nps = 0.0;
    
    std::cout << "\nStarting new game with " << time_limit_ms << "ms per move" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    while (!state.isGameOver()) {
        AIPlayer& current_player = (state.getCurrentPlayer() == PlayerID::PLAYER_1) ? player1 : player2;
        
        auto start = std::chrono::steady_clock::now();
        Move best_move = current_player.findBestMove(state, time_limit_ms);
        auto end = std::chrono::steady_clock::now();
        
        state = state.createNextState(best_move);
        total_moves++;
        
        // Print board state
        state.getBoard().printBoard();
        std::cout << "----------------------------------------" << std::endl;
    }
    
    PlayerID winner = state.getWinner();
    std::cout << "Game Over! Winner: Player " 
              << (winner == PlayerID::PLAYER_1 ? "1" : "2") << std::endl;
    std::cout << "Total moves: " << total_moves << std::endl;
    
    return GameResult{
        winner,
        total_moves,
        0,  // Will be filled by the stats from AIPlayer
        0.0,
        0.0
    };
}

int main() {
    const int NUM_GAMES = 3;
    const int TIME_PER_MOVE_MS = 10000;  // 10 seconds
    
    std::vector<GameResult> results;
    
    for (int i = 0; i < NUM_GAMES; i++) {
        std::cout << "\n=== Starting Game " << (i + 1) << " ===" << std::endl;
        results.push_back(playSelfPlay(TIME_PER_MOVE_MS));
    }
    
    // Print summary
    int player1_wins = 0;
    int player2_wins = 0;
    int total_moves = 0;
    
    for (const auto& result : results) {
        if (result.winner == PlayerID::PLAYER_1) player1_wins++;
        if (result.winner == PlayerID::PLAYER_2) player2_wins++;
        total_moves += result.total_moves;
    }
    
    std::cout << "\n=== Test Summary ===" << std::endl;
    std::cout << "Games played: " << NUM_GAMES << std::endl;
    std::cout << "Player 1 wins: " << player1_wins << std::endl;
    std::cout << "Player 2 wins: " << player2_wins << std::endl;
    std::cout << "Average moves per game: " << (total_moves / NUM_GAMES) << std::endl;
    
    return 0;
}
