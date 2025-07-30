#include "GameStateHasher.h"

#include <random>

namespace SquadroAI {

// Initializing static members
std::array<std::array<std::array<uint64_t, NUM_COLS>, NUM_ROWS>, NUM_PIECES>
    GameStateHasher::piece_hashes;
std::array<uint64_t, 2> GameStateHasher::player_turn_hashes;
std::array<std::array<uint64_t, 2>, NUM_PIECES>
    GameStateHasher::piece_status_hashes;
bool GameStateHasher::is_initialized = false;

void GameStateHasher::initialize() {
  if (is_initialized) return;

  // Using a Mersenne Twister with a fixed seed for reproducibility
  std::mt19937_64 rng(0x12345678);  // Fixed seed
  std::uniform_int_distribution<uint64_t> dist;

  // Initialize piece position hashes
  for (int piece = 0; piece < NUM_PIECES; ++piece) {
    for (int row = 0; row < NUM_ROWS; ++row) {
      for (int col = 0; col < NUM_COLS; ++col) {
        piece_hashes[piece][row][col] = dist(rng);
      }
    }
  }

  // Initialize player turn hashes
  for (int i = 0; i < 2; ++i) {
    player_turn_hashes[i] = dist(rng);
  }

  // Initialize piece status hashes
  for (int piece = 0; piece < NUM_PIECES; ++piece) {
    for (int status = 0; status < 2; ++status) {
      piece_status_hashes[piece][status] = dist(rng);
    }
  }

  is_initialized = true;
}

uint64_t GameStateHasher::computeHash(const Board& board,
                                    PlayerID currentPlayer) {
  if (!is_initialized) {
    initialize();  // Auto-initialize if needed
  }

  uint64_t hash = 0;

  // Hash the position and status of each piece
  const auto& pieces = board.getAllPieces();
  for (int piece_id = 0; piece_id < NUM_PIECES; ++piece_id) {
    const auto& piece = pieces[piece_id];

    // Only hash pieces that are on the board
    if (piece.row >= 0 && piece.row < NUM_ROWS && piece.col >= 0 &&
        piece.col < NUM_COLS) {
      hash ^= piece_hashes[piece_id][piece.row][piece.col];

      // Hash the piece's status/direction
      int status_idx = (piece.status == PieceStatus::ON_BOARD_BACKWARD) ? 1 : 0;
      hash ^= piece_status_hashes[piece_id][status_idx];
    }
  }

  // Hash the current player's turn
  int player_idx = (currentPlayer == PlayerID::PLAYER_2) ? 1 : 0;
  hash ^= player_turn_hashes[player_idx];

  return hash;
}

}  // namespace SquadroAI
