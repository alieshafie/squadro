#include "Heuristics.h"

#include "GameState.h"  // Needs the full definition of GameState

namespace SquadroAI {

int Heuristics::evaluate(const GameState& state, PlayerID perspective_player) {
  if (state.isGameOver()) {
    PlayerID winner = state.getWinner();
    if (winner == perspective_player) return WIN_SCORE;
    if (winner == PlayerID::NONE || winner == PlayerID::DRAW) return DRAW_SCORE;
    return LOSS_SCORE;
  }

  int my_score = 0;
  int opponent_score = 0;
  const auto& pieces = state.getBoard().getAllPieces();

  for (const auto& piece : pieces) {
    if (piece.owner == PlayerID::NONE) continue;

    int piece_score = 0;
    if (piece.isFinished()) {
      piece_score += PIECE_COMPLETED_SCORE;
    } else {
      int progress = 0;
      if (piece.owner == PlayerID::PLAYER_1) {
        progress = (piece.status == PieceStatus::ON_BOARD_FORWARD)
                       ? piece.col
                       : (NUM_COLS - 1) + ((NUM_COLS - 1) - piece.col);
      } else {  // Player 2
        progress = (piece.status == PieceStatus::ON_BOARD_FORWARD)
                       ? piece.row
                       : (NUM_ROWS - 1) + ((NUM_ROWS - 1) - piece.row);
      }
      if (progress >= 0 && progress < 13) {
        piece_score += PIECE_PROGRESS_SCORE[progress];
      }
    }

    if (piece.owner == perspective_player) {
      my_score += piece_score;
    } else {
      opponent_score += piece_score;
    }
  }
  return my_score - opponent_score;
}

}  // namespace SquadroAI
