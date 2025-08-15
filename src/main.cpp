#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

// Include all project headers
#include "AIPlayer.h"
#include "Board.h"
#include "Constants.h"
#include "GameState.h"
#include "GameStateHasher.h"
#include "Move.h"
#include "NetworkManager.h"
#include "Piece.h"
#include "TranspositionTable.h"

using namespace SquadroAI;

// --- Global variables for network thread synchronization ---
std::mutex network_mutex;
std::condition_variable cv_opponent_move;
bool g_opponent_has_moved = false;
int g_last_opponent_pawn_move_idx = -1;  // Relative index (1-5)
// --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---

void opponent_move_callback(int opponent_pawn_index) {
  std::lock_guard<std::mutex> lock(network_mutex);
  g_last_opponent_pawn_move_idx = opponent_pawn_index;
  g_opponent_has_moved = true;
  std::cout << "[Callback] Opponent moved their pawn with relative index: "
            << opponent_pawn_index << std::endl;
  cv_opponent_move.notify_one();
}

int main(int argc, char *argv[]) {
  std::cout << "Squadro AI Agent (C++) starting..." << std::endl;

  // IMPORTANT: Initialize Zobrist keys once at the start of the program.
  GameStateHasher::initialize();
  std::cout << "Zobrist Hasher Initialized." << std::endl;

  if (argc < 7) {
    std::cerr << "Usage: " << argv[0]
              << " <my_player_num (1 or 2)> <gui_ip> <p1_send_port> "
                 "<p1_listen_port> <p2_send_port> <p2_listen_port>"
              << std::endl;
    return 1;
  }
  // (Argument parsing code remains the same as yours... it's already excellent)
  int my_player_num_arg;
  std::string gui_ip_arg;
  int p1_send_to_gui_port_arg, p1_listen_reply_port_arg;
  int p2_send_to_gui_port_arg, p2_listen_reply_port_arg;

  try {
    my_player_num_arg = std::stoi(argv[1]);
    gui_ip_arg = argv[2];
    p1_send_to_gui_port_arg = std::stoi(argv[3]);
    p1_listen_reply_port_arg = std::stoi(argv[4]);
    p2_send_to_gui_port_arg = std::stoi(argv[5]);
    p2_listen_reply_port_arg = std::stoi(argv[6]);
  } catch (const std::exception &e) {
    std::cerr << "Error parsing arguments: " << e.what() << std::endl;
    return 1;
  }
  if (my_player_num_arg != 1 && my_player_num_arg != 2) {
    std::cerr << "Error: Player number must be 1 or 2." << std::endl;
    return 1;
  }

  PlayerID my_ai_player_id =
      (my_player_num_arg == 1) ? PlayerID::PLAYER_1 : PlayerID::PLAYER_2;
  PlayerID opponent_id =
      (my_player_num_arg == 1) ? PlayerID::PLAYER_2 : PlayerID::PLAYER_1;

  int my_send_to_gui_port = (my_player_num_arg == 1) ? p1_send_to_gui_port_arg
                                                     : p2_send_to_gui_port_arg;
  int my_listen_for_reply_port = (my_player_num_arg == 1)
                                     ? p1_listen_reply_port_arg
                                     : p2_listen_reply_port_arg;

  std::cout << "Registered as Player " << my_player_num_arg << std::endl;

  try {
    GameState current_game_state;             // Initializes to starting state
    AIPlayer ai_player(my_ai_player_id, 64);  // 64MB TT
    NetworkManager network_manager(gui_ip_arg, my_send_to_gui_port, "0.0.0.0",
                                   my_listen_for_reply_port);

    network_manager.startListeningForOpponentMoves(opponent_move_callback);
    std::cout << "NetworkManager started. Listening for opponent moves..."
              << std::endl;

    // Main game loop
    while (true) {
      if (current_game_state.isGameOver()) {
        std::cout << "Game is over (locally determined)." << std::endl;
        break;
      }
      current_game_state.getBoard()
          .printBoard();  // Print current board state for debugging.

      if (current_game_state.getCurrentPlayer() == my_ai_player_id) {
        std::cout << "My turn (Player " << my_player_num_arg << "). Thinking..."
                  << std::endl;

        // FIXED: Using 10-second time limit with a 500ms safety margin.
        std::chrono::milliseconds time_for_move(9500);
        Move best_move = ai_player.findBestMove(
            current_game_state, static_cast<int>(time_for_move.count()));

        if (best_move.id != -1) {  // Compare with -1 directly since NULL_MOVE
                                   // is inside the function scope
          // FIXED: Convert global index to relative index for the GUI.
          int relative_idx_to_send =
              best_move.getRelativeIndex(my_ai_player_id);
          std::cout << "AI chose " << best_move.to_string()
                    << ". Sending relative index: " << relative_idx_to_send
                    << std::endl;

          if (network_manager.sendMoveToGui(relative_idx_to_send)) {
            std::cout << "Move successfully sent to GUI." << std::endl;
            // Apply the move to our local state
            current_game_state.applyMove(best_move);
            std::cout << "Local game state updated." << std::endl;
          } else {
            std::cerr << "FATAL ERROR: Failed to send move to GUI. Terminating."
                      << std::endl;
            network_manager.stopListening();
            return 1;
          }
        } else {
          std::cerr
              << "CRITICAL ERROR: AIPlayer returned NULL_MOVE. Terminating."
              << std::endl;
          network_manager.stopListening();
          return 1;
        }
      } else {  // Opponent's turn
        std::cout << "Opponent's turn. Waiting for move..." << std::endl;

        std::unique_lock<std::mutex> lock(network_mutex);
        // with timeout
        /* if (!cv_opponent_move.wait_for(lock, std::chrono::seconds(65),
                                       [] { return g_opponent_has_moved; })) {
          std::cerr << "TIMEOUT: No move received from opponent. Terminating."
                    << std::endl;
          network_manager.stopListening();
          return 1;
        } */

        cv_opponent_move.wait(lock, [] { return g_opponent_has_moved; });

        // FIXED: Create a correct Move object from the opponent's relative
        // index.
        Move opponent_move =
            Move::fromRelativeIndex(g_last_opponent_pawn_move_idx, opponent_id);
        std::cout << "Opponent's relative move "
                  << g_last_opponent_pawn_move_idx << " received. Applying "
                  << opponent_move.to_string() << std::endl;

        // Apply the opponent's move to our local state
        current_game_state.applyMove(opponent_move);
        std::cout << "Opponent's move applied to local state." << std::endl;
        g_opponent_has_moved = false;
        lock.unlock();
      }
    }

    std::cout << "Game Over! (Main loop exited)." << std::endl;
    current_game_state.getBoard().printBoard();  // Print final state
    PlayerID winner = current_game_state.getWinner();

    if (winner == my_ai_player_id) {
      std::cout << "VICTORY! I (Player " << my_player_num_arg << ") won!"
                << std::endl;
    } else if (winner == opponent_id) {
      std::cout << "DEFEAT. Opponent won." << std::endl;
    } else {
      std::cout << "The game is a DRAW or ended inconclusively." << std::endl;
    }

    network_manager.stopListening();
  } catch (const std::exception &e) {
    std::cerr << "FATAL Unhandled std::exception in main: " << e.what()
              << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "FATAL Unknown unhandled exception in main." << std::endl;
    return 1;
  }

  std::cout << "Squadro AI Agent (C++) shutting down gracefully." << std::endl;
  return 0;
}
