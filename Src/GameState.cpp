#include "GameStateHasher.h"

namespace SquadroAI {

// مقداردهی اولیه متغیرهای استاتیک
std::array<std::array<std::array<uint64_t, NUM_COLS>, NUM_ROWS>, NUM_PIECES>
    GameStateHasher::piece_hashes;
std::array<uint64_t, 2> GameStateHasher::player_turn_hashes;
std::array<std::array<uint64_t, 2>, NUM_PIECES>
    GameStateHasher::piece_status_hashes;
bool GameStateHasher::is_initialized = false;

void GameStateHasher::initialize() {
  if (is_initialized) return;

  // از یک seed ثابت استفاده می‌کنیم تا هر بار اجرای برنامه، هش‌های یکسانی
  // تولید شود این برای دیباگ کردن حیاتی است.
  std::mt19937_64 rng(0xBEEFDEED);  // یک عدد جادویی برای seed
  std::uniform_int_distribution<uint64_t> dist;

  for (int p = 0; p < NUM_PIECES; ++p) {
    for (int r = 0; r < NUM_ROWS; ++r) {
      for (int c = 0; c < NUM_COLS; ++c) {
        piece_hashes[p][r][c] = dist(rng);
      }
    }
  }

  player_turn_hashes[0] = dist(rng);  // Player 1
  player_turn_hashes[1] = dist(rng);  // Player 2

  for (int p = 0; p < NUM_PIECES; ++p) {
    piece_status_hashes[p][0] = dist(rng);  // FORWARD
    piece_status_hashes[p][1] = dist(rng);  // BACKWARD
  }

  is_initialized = true;
}

uint64_t GameStateHasher::computeHash(const Board& board,
                                      PlayerID currentPlayer) {
  uint64_t hash = 0;

  const auto& pieces = board.getAllPieces();
  for (const auto& piece : pieces) {
    if (!piece.isFinished()) {
      // 1. هش مکان مهره
      hash ^= piece_hashes[piece.id][piece.row][piece.col];
      // 2. هش وضعیت مهره (رفت یا برگشت)
      int status_index =
          (piece.status == PieceStatus::ON_BOARD_FORWARD) ? 0 : 1;
      hash ^= piece_status_hashes[piece.id][status_index];
    }
  }

  // 3. هش نوبت بازیکن
  int player_index = (currentPlayer == PlayerID::PLAYER_1) ? 0 : 1;
  hash ^= player_turn_hashes[player_index];

  return hash;
}

}  // namespace SquadroAI
