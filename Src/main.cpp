#include <iostream>
#include <string>
#include <stdexcept> // For std::runtime_error, std::invalid_argument, std::out_of_range
#include <vector>    // For command-line arguments
#include <thread>    // For std::thread
#include <condition_variable>
#include <mutex>
#include <chrono> // For std::chrono::seconds, std::chrono::milliseconds

// Include all project headers. Ensure these files have their content
// wrapped in 'namespace SquadroAI { ... }'
#include "Constants.h"
#include "Move.h"
#include "Piece.h"
#include "Board.h"
#include "GameState.h"
#include "GameStateHasher.h"    // Included for completeness, AIPlayer might use it internally
#include "TranspositionTable.h" // Included for completeness, AIPlayer uses it
#include "Heuristics.h"         // Included for completeness, AIPlayer uses it
#include "AIPlayer.h"
#include "NetworkManager.h"

// Bring the SquadroAI namespace into the current scope for easier access to its members.
using namespace SquadroAI;

// --- Global variables for network thread synchronization ---
std::mutex network_mutex;
std::condition_variable cv_opponent_move;
bool g_opponent_has_moved = false;
int g_last_opponent_pawn_move_idx = -1; // Index of the opponent's pawn (0-4)
// --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---

// Callback function invoked by the NetworkManager's thread when an opponent's move is received.
void opponent_move_callback(int opponent_pawn_index)
{
    std::lock_guard<std::mutex> lock(network_mutex);
    g_last_opponent_pawn_move_idx = opponent_pawn_index;
    g_opponent_has_moved = true;
    std::cout << "[Callback] Opponent moved their pawn with index: " << opponent_pawn_index << std::endl;
    cv_opponent_move.notify_one(); // Notify the main game loop thread.
}

int main(int argc, char *argv[])
{
    std::cout << "Squadro AI Agent (C++) starting..." << std::endl;

    // Initialize Zobrist keys if your GameStateHasher requires a static initialization.
    // Example: GameStateHasher::initializeGlobalZobristTable();
    // If the constructor of GameStateHasher handles this, this line is not needed.

    if (argc < 7)
    { // argv[0] is the program name, so 6 more arguments are needed.
        std::cerr << "Usage: " << argv[0]
                  << " <my_player_num (1 or 2)> <gui_ip> "
                  << "<p1_sends_to_gui_port> <p1_listens_for_reply_port> "
                  << "<p2_sends_to_gui_port> <p2_listens_for_reply_port>" << std::endl;
        std::cerr << "Example: " << argv[0] << " 1 127.0.0.1 8081 9081 8082 9082" << std::endl;
        return 1;
    }

    int my_player_num_arg;
    std::string gui_ip_arg;
    int p1_send_to_gui_port_arg, p1_listen_reply_port_arg;
    int p2_send_to_gui_port_arg, p2_listen_reply_port_arg;

    try
    {
        my_player_num_arg = std::stoi(argv[1]);
        gui_ip_arg = argv[2]; // std::string can be constructed from char*
        p1_send_to_gui_port_arg = std::stoi(argv[3]);
        p1_listen_reply_port_arg = std::stoi(argv[4]);
        p2_send_to_gui_port_arg = std::stoi(argv[5]);
        p2_listen_reply_port_arg = std::stoi(argv[6]);
    }
    catch (const std::invalid_argument &ia)
    {
        std::cerr << "Error: Invalid argument provided. " << ia.what()
                  << " Please ensure player number and port numbers are integers." << std::endl;
        return 1;
    }
    catch (const std::out_of_range &oor)
    {
        std::cerr << "Error: Out of range argument. " << oor.what()
                  << " A port number might be too large or too small." << std::endl;
        return 1;
    }

    if (my_player_num_arg != 1 && my_player_num_arg != 2)
    {
        std::cerr << "Error: Player number must be 1 or 2. Received: " << my_player_num_arg << std::endl;
        return 1;
    }

    PlayerID my_ai_player_id = (my_player_num_arg == 1) ? PlayerID::PLAYER_1 : PlayerID::PLAYER_2;
    PlayerID opponent_id = (my_player_num_arg == 1) ? PlayerID::PLAYER_2 : PlayerID::PLAYER_1;

    std::cout << "Registered as Player " << my_player_num_arg
              << " (Internal PlayerID: " << static_cast<int>(my_ai_player_id) << ")" << std::endl;
    std::cout << "GUI IP: " << gui_ip_arg << std::endl;

    // Determine which ports this instance should use based on its player number.
    int my_send_to_gui_port = (my_player_num_arg == 1) ? p1_send_to_gui_port_arg : p2_send_to_gui_port_arg;
    int my_listen_for_reply_port = (my_player_num_arg == 1) ? p1_listen_reply_port_arg : p2_listen_reply_port_arg;

    std::cout << "This agent (Player " << my_player_num_arg << ") will send its moves to GUI on port: "
              << my_send_to_gui_port << std::endl;
    std::cout << "This agent (Player " << my_player_num_arg << ") will listen for opponent's moves from GUI on port: "
              << my_listen_for_reply_port << std::endl;

    try
    {
        GameState current_game_state;           // Initializes to the starting state of the game.
        current_game_state.initializeNewGame(); // Ensure board and pieces are set up.

        AIPlayer ai_player(my_ai_player_id); // Default TT size (e.g., 64MB).
        NetworkManager network_manager(gui_ip_arg, my_send_to_gui_port,
                                       "0.0.0.0", my_listen_for_reply_port); // Listen on all available interfaces.

        network_manager.startListeningForOpponentMoves(opponent_move_callback);
        std::cout << "NetworkManager started. Listening for opponent moves on a separate thread..." << std::endl;

        // Set initial turn according to Squadro rules (Player 1 usually starts).
        // GameState constructor should set this, but we can ensure it here.
        current_game_state.setCurrentPlayer(PlayerID::PLAYER_1);
        if (my_player_num_arg == 1)
        {
            std::cout << "I am Player 1. My turn to make the first move." << std::endl;
        }
        else
        { // Player 2
            std::cout << "I am Player 2. Waiting for Player 1's first move." << std::endl;
        }

        // Main game loop
        while (true)
        {
            if (current_game_state.isGameOver())
            {
                std::cout << "Game is over (locally determined)." << std::endl;
                break;
            }
            current_game_state.printState(); // For debugging: print current board state.

            if (current_game_state.getCurrentPlayer() == my_ai_player_id)
            {
                std::cout << "My turn (Player " << my_player_num_arg << "). Thinking..." << std::endl;

                // Time limit for decision making (e.g., 29 seconds to be safe within 30s).
                std::chrono::milliseconds time_for_move(29000);
                Move best_move = ai_player.findBestMove(current_game_state, time_for_move);

                if (best_move.piece_index != NULL_MOVE.piece_index)
                {
                    std::cout << "AI chose to move its pawn with player-relative index: "
                              << best_move.piece_index << ". (" << best_move.to_string() << ")" << std::endl;

                    // The piece_index from AIPlayer should be the 0-4 index for the current player.
                    // GUI also expects this 0-4 index.
                    if (network_manager.sendMoveToGui(best_move.piece_index))
                    {
                        std::cout << "Move successfully sent to GUI." << std::endl;
                        // Apply the move to our local game state *after* successfully sending it.
                        if (!current_game_state.applyMove(best_move))
                        {
                            std::cerr << "CRITICAL ERROR: AI-generated move (" << best_move.to_string()
                                      << ") was sent to GUI but deemed invalid by local GameState.applyMove. "
                                      << "This indicates a bug in move generation or validation logic. State desync likely." << std::endl;
                            network_manager.stopListening();
                            return 1; // Critical error, exit.
                        }
                        std::cout << "Local game state updated." << std::endl;
                    }
                    else
                    {
                        std::cerr << "FATAL ERROR: Failed to send move to GUI. Terminating." << std::endl;
                        network_manager.stopListening();
                        return 1; // Critical error, exit.
                    }
                }
                else
                {
                    std::cerr << "CRITICAL ERROR: AIPlayer::findBestMove returned NULL_MOVE. "
                              << "AI could not find any valid move. This implies a bug, an unhandled stalemate, or a lost game state where no moves are possible. Terminating."
                              << std::endl;
                    network_manager.stopListening();
                    return 1; // Critical error, exit.
                }
            }
            else
            { // Opponent's turn
                std::cout << "Opponent's turn (Player "
                          << (current_game_state.getCurrentPlayer() == PlayerID::PLAYER_1 ? "1" : "2")
                          << "). Waiting for move from GUI..." << std::endl;

                std::unique_lock<std::mutex> lock(network_mutex);
                // Wait for the network callback to signal an opponent's move.
                // Add a generous timeout in case something goes wrong with GUI or network.
                if (!cv_opponent_move.wait_for(lock, std::chrono::seconds(65), []
                                               { return g_opponent_has_moved; }))
                {
                    std::cerr << "TIMEOUT: No move received from opponent within the timeout period. "
                              << "Assuming opponent disconnected or GUI issue. Terminating." << std::endl;
                    // In a real competition, this might be handled differently (e.g., claim win by timeout).
                    network_manager.stopListening();
                    return 1; // Exit.
                }

                // g_opponent_has_moved is now true.
                Move opponent_move = {g_last_opponent_pawn_move_idx}; // piece_index relative to the opponent.
                std::cout << "Opponent's move received from GUI: Pawn with player-relative index "
                          << opponent_move.piece_index << ". (" << opponent_move.to_string() << ")" << std::endl;

                // Apply opponent's move to our local game state.
                // GameState::applyMove should correctly handle which player's piece_index this is.
                if (!current_game_state.applyMove(opponent_move))
                {
                    std::cerr << "CRITICAL ERROR: Opponent's move (" << opponent_move.to_string()
                              << "), received from GUI, is considered invalid by local GameState.applyMove. "
                              << "State desynchronization with GUI is highly likely. Terminating." << std::endl;
                    // This could mean GUI sent a bad move, or our state/validation logic is flawed.
                    network_manager.stopListening();
                    return 1; // Critical error, exit.
                }
                std::cout << "Opponent's move successfully applied to local game state." << std::endl;
                g_opponent_has_moved = false; // Reset flag for the next opponent move.
                lock.unlock();                // Release lock as soon as possible.
            }
        } // End of main game loop.

        std::cout << "Game Over! (Main loop exited because isGameOver() is true)." << std::endl;
        current_game_state.printState();                  // Print final state.
        PlayerID winner = current_game_state.getWinner(); // This function should determine the winner.

        if (winner == my_ai_player_id)
        {
            std::cout << "VICTORY! I (Player " << my_player_num_arg << ") won!" << std::endl;
        }
        else if (winner == opponent_id)
        {
            std::cout << "DEFEAT. Opponent (Player "
                      << (opponent_id == PlayerID::PLAYER_1 ? "1" : "2")
                      << ") won." << std::endl;
        }
        else if (winner == PlayerID::DRAW)
        {
            std::cout << "The game is a DRAW." << std::endl;
        }
        else
        { // PlayerID::NONE
            std::cout << "Game ended, but the winner is PlayerID::NONE. "
                      << "(This might occur if the game exited prematurely or if the win condition wasn't met by either player, e.g., a loop limit)." << std::endl;
        }

        network_manager.stopListening(); // Ensure network thread is properly shut down.
    }
    catch (const std::exception &e)
    {
        std::cerr << "FATAL Unhandled std::exception in main: " << e.what() << std::endl;
        // Consider trying to stop network manager if it was initialized.
        return 1;
    }
    catch (...)
    {
        std::cerr << "FATAL Unknown unhandled exception in main." << std::endl;
        return 1;
    }

    std::cout << "Squadro AI Agent (C++) shutting down gracefully." << std::endl;
    return 0;
}