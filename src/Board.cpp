// === File: src/Board.cpp ===
#include "Board.h"

#include <cstring>  // memcpy if needed
#include <iostream>
#include <stdexcept>

namespace SquadroAI {

Board::Board() { initializeBoard(); }

void Board::initializeBoard() {
  grid.fill(EMPTY_CELL);

  for (int id = 0; id < NUM_PIECES; ++id) {
    Piece& p = pieces[id];
    p.id = id;
    p.owner =
        (id < PIECES_PER_PLAYER) ? PlayerID::PLAYER_1 : PlayerID::PLAYER_2;
    p.status = PieceStatus::ON_BOARD_FORWARD;

    int player_piece_index = id % PIECES_PER_PLAYER;
    p.forward_power = FWD_POWERS[id];
    p.backward_power = BCK_POWERS[id];

    if (p.owner == PlayerID::PLAYER_1) {
      p.row = player_piece_index + 1;
      p.col = 0;
    } else {
      p.row = 0;
      p.col = player_piece_index + 1;
    }
    cell_ref(p.row, p.col) = p.id;
  }
}

static inline void clamp_to_edge(int& r, int& c, int dr, int dc) {
  if (dr > 0)
    r = NUM_ROWS - 1;
  else if (dr < 0)
    r = 0;
  if (dc > 0)
    c = NUM_COLS - 1;
  else if (dc < 0)
    c = 0;
}

// This new helper function contains the consolidated move logic.
// It is const and does not modify the board state. It returns the calculated
// outcome.
Board::MoveResult Board::calculateMoveResult(const Piece& mover) const {
  MoveResult result;

  const bool is_forward = (mover.status == PieceStatus::ON_BOARD_FORWARD);
  const int power = mover.getCurrentMovePower();
  const int dr =
      (mover.owner == PlayerID::PLAYER_2) ? (is_forward ? 1 : -1) : 0;
  const int dc =
      (mover.owner == PlayerID::PLAYER_1) ? (is_forward ? 1 : -1) : 0;

  // OPTIMIZATION: This function is now allocation-free. It simulates the move
  // by calculating positions and using a small, stack-allocated array to track
  // pieces that would be captured and moved during this simulation.
  int current_r = mover.row;
  int current_c = mover.col;

  // Simulated positions of pieces during the move validation.
  std::array<int8_t, NUM_PIECES> sim_rows, sim_cols;
  for (int i = 0; i < NUM_PIECES; ++i) {
    sim_rows[i] = pieces[i].row;
    sim_cols[i] = pieces[i].col;
  }

  // A helper lambda to check for collisions in our simulated reality.
  auto is_cell_occupied_sim = [&](int r, int c) {
    for (int i = 0; i < NUM_PIECES; ++i) {
      // Skip the mover itself and any captured (moved) pieces.
      if (i == mover.id || pieces[i].isFinished()) continue;
      bool was_captured = false;
      for (uint8_t k = 0; k < result.captured_count; ++k) {
        if (result.captures[k].id == i) {
          was_captured = true;
          break;
        }
      }
      if (was_captured) continue;

      if (sim_rows[i] == r && sim_cols[i] == c) {
        return i;  // Return ID of piece at the location
      }
    }
    return EMPTY_CELL;
  };

  for (int step = 0; step < power; ++step) {
    int next_r = current_r + dr;
    int next_c = current_c + dc;

    if (!isPositionValid(next_r, next_c)) {
      clamp_to_edge(next_r, next_c, dr, dc);
      current_r = next_r;
      current_c = next_c;
      break;
    }

    current_r = next_r;
    current_c = next_c;

    int occupant_id = is_cell_occupied_sim(current_r, current_c);
    if (occupant_id != EMPTY_CELL) {
      // Collision chain logic
      while (isPositionValid(current_r, current_c) &&
             (occupant_id = is_cell_occupied_sim(current_r, current_c)) !=
                 EMPTY_CELL) {
        if (pieces[occupant_id].owner == mover.owner) {
          return result;  // Invalid move (ran into own piece)
        }

        if (result.captured_count >= MAX_POSSIBLE_CAPTURES) {
          return result;  // Too many captures, invalid
        }

        const auto& opponent_piece = pieces[occupant_id];
        auto& ci = result.captures[result.captured_count++];
        ci.id = static_cast<uint8_t>(opponent_piece.id);
        ci.prev_row = static_cast<int8_t>(opponent_piece.row);
        ci.prev_col = static_cast<int8_t>(opponent_piece.col);

        // Simulate the opponent's reset
        int opponent_idx = opponent_piece.id % PIECES_PER_PLAYER;
        int reset_row, reset_col;
        if (opponent_piece.status == PieceStatus::ON_BOARD_FORWARD) {
          if (opponent_piece.owner == PlayerID::PLAYER_1) {
            reset_row = opponent_idx + 1;
            reset_col = 0;
          } else {
            reset_row = 0;
            reset_col = opponent_idx + 1;
          }
        } else {
          if (opponent_piece.owner == PlayerID::PLAYER_1) {
            reset_row = opponent_idx + 1;
            reset_col = NUM_COLS - 1;
          } else {
            reset_row = NUM_ROWS - 1;
            reset_col = opponent_idx + 1;
          }
        }
        sim_rows[occupant_id] = reset_row;
        sim_cols[occupant_id] = reset_col;

        // Check if the reset location is blocked by another piece (not the one
        // being reset)
        int blocker_id = is_cell_occupied_sim(reset_row, reset_col);
        if (blocker_id != EMPTY_CELL && blocker_id != occupant_id) {
          return result;  // Reset location is blocked
        }

        int beyond_r = current_r + dr;
        int beyond_c = current_c + dc;
        if (!isPositionValid(beyond_r, beyond_c)) {
          clamp_to_edge(beyond_r, beyond_c, dr, dc);
          current_r = beyond_r;
          current_c = beyond_c;
          goto move_simulation_end;
        } else {
          current_r = beyond_r;
          current_c = beyond_c;
        }
      }
      break;
    }
  }

move_simulation_end:
  result.dest_row = static_cast<int8_t>(current_r);
  result.dest_col = static_cast<int8_t>(current_c);

  bool reached_end;
  if (mover.owner == PlayerID::PLAYER_1) {
    reached_end =
        is_forward ? (result.dest_col == NUM_COLS - 1) : (result.dest_col == 0);
  } else {
    reached_end =
        is_forward ? (result.dest_row == NUM_ROWS - 1) : (result.dest_row == 0);
  }

  result.final_status = mover.status;
  if (reached_end) {
    result.final_status =
        is_forward ? PieceStatus::ON_BOARD_BACKWARD : PieceStatus::FINISHED;
  }

  if (result.final_status != PieceStatus::FINISHED) {
    // Use the simulation checker to see if the final destination is blocked
    if (is_cell_occupied_sim(result.dest_row, result.dest_col) != EMPTY_CELL) {
      return result;  // Final destination is blocked
    }
  }

  result.is_valid = true;
  return result;
}

std::optional<Board::AppliedMoveInfo> Board::applyMove(
    const Move& move, PlayerID current_player) {
  if (move.id < 0 || move.id >= NUM_PIECES) return std::nullopt;

  Piece& mover = pieces[move.id];
  if (mover.owner != current_player || mover.isFinished()) return std::nullopt;

  AppliedMoveInfo move_info;
  move_info.mover_id = mover.id;
  move_info.start_row = mover.row;
  move_info.start_col = mover.col;
  move_info.original_mover_status = static_cast<uint8_t>(mover.status);

  const bool is_forward = (mover.status == PieceStatus::ON_BOARD_FORWARD);
  const int power = mover.getCurrentMovePower();
  const int dr =
      (mover.owner == PlayerID::PLAYER_2) ? (is_forward ? 1 : -1) : 0;
  const int dc =
      (mover.owner == PlayerID::PLAYER_1) ? (is_forward ? 1 : -1) : 0;

  int current_r = mover.row;
  int current_c = mover.col;
  cell_ref(current_r, current_c) = EMPTY_CELL;

  for (int step = 0; step < power; ++step) {
    int next_r = current_r + dr;
    int next_c = current_c + dc;

    if (!isPositionValid(next_r, next_c)) {
      clamp_to_edge(next_r, next_c, dr, dc);
      current_r = next_r;
      current_c = next_c;
      break;
    }
    current_r = next_r;
    current_c = next_c;

    if (cell_ref(current_r, current_c) != EMPTY_CELL) {
      while (isPositionValid(current_r, current_c) &&
             cell_ref(current_r, current_c) != EMPTY_CELL) {
        const int opponent_id = cell_ref(current_r, current_c);
        if (opponent_id < 0 || opponent_id >= NUM_PIECES ||
            pieces[opponent_id].owner == mover.owner)
          return std::nullopt;
        if (move_info.captured_count >= MAX_POSSIBLE_CAPTURES)
          return std::nullopt;

        Piece& opponent = pieces[opponent_id];
        auto& ci = move_info.captures[move_info.captured_count++];
        ci.id = opponent.id;
        ci.prev_row = opponent.row;
        ci.prev_col = opponent.col;

        cell_ref(opponent.row, opponent.col) =
            EMPTY_CELL;  // Vacate opponent's old cell

        int opponent_idx = opponent.id % PIECES_PER_PLAYER;
        if (opponent.status == PieceStatus::ON_BOARD_FORWARD) {
          if (opponent.owner == PlayerID::PLAYER_1) {
            opponent.row = opponent_idx + 1;
            opponent.col = 0;
          } else {
            opponent.row = 0;
            opponent.col = opponent_idx + 1;
          }
        } else {
          if (opponent.owner == PlayerID::PLAYER_1) {
            opponent.row = opponent_idx + 1;
            opponent.col = NUM_COLS - 1;
          } else {
            opponent.row = NUM_ROWS - 1;
            opponent.col = opponent_idx + 1;
          }
        }

        if (cell_ref(opponent.row, opponent.col) != EMPTY_CELL)
          return std::nullopt;  // Reset location blocked
        cell_ref(opponent.row, opponent.col) = opponent.id;

        int beyond_r = current_r + dr;
        int beyond_c = current_c + dc;
        if (!isPositionValid(beyond_r, beyond_c)) {
          clamp_to_edge(beyond_r, beyond_c, dr, dc);
          current_r = beyond_r;
          current_c = beyond_c;
          goto move_simulation_end_apply;
        } else {
          current_r = beyond_r;
          current_c = beyond_c;
        }
      }
      break;
    }
  }

move_simulation_end_apply:;
  move_info.dest_row = current_r;
  move_info.dest_col = current_c;
  mover.row = current_r;
  mover.col = current_c;

  bool reached_end;
  if (mover.owner == PlayerID::PLAYER_1) {
    reached_end = is_forward ? (mover.col == NUM_COLS - 1) : (mover.col == 0);
  } else {
    reached_end = is_forward ? (mover.row == NUM_ROWS - 1) : (mover.row == 0);
  }

  if (reached_end) {
    mover.status =
        is_forward ? PieceStatus::ON_BOARD_BACKWARD : PieceStatus::FINISHED;
  }

  if (!mover.isFinished()) {
    if (cell_ref(mover.row, mover.col) != EMPTY_CELL) return std::nullopt;
    cell_ref(mover.row, mover.col) = mover.id;
  }

  return move_info;
}

void Board::undoMove(const AppliedMoveInfo& move_info) {
  const int piece_id = static_cast<int>(move_info.mover_id);
  Piece& mover = pieces[piece_id];

  // Clear mover's current position from grid
  if (!mover.isFinished() && isPositionValid(mover.row, mover.col)) {
    if (cell_ref(mover.row, mover.col) == piece_id)
      cell_ref(mover.row, mover.col) = EMPTY_CELL;
  }

  // Restore mover's state
  mover.status = static_cast<PieceStatus>(move_info.original_mover_status);
  mover.row = move_info.start_row;
  mover.col = move_info.start_col;
  if (isPositionValid(mover.row, mover.col)) {
    cell_ref(mover.row, mover.col) = piece_id;
  }

  // Restore captured pieces in reverse order
  for (int k = static_cast<int>(move_info.captured_count) - 1; k >= 0; --k) {
    const auto& ci = move_info.captures[k];
    Piece& cap = pieces[ci.id];

    // Clear the reset position
    if (isPositionValid(cap.row, cap.col) &&
        cell_ref(cap.row, cap.col) == ci.id) {
      cell_ref(cap.row, cap.col) = EMPTY_CELL;
    }

    // Restore original position and state
    cap.row = ci.prev_row;
    cap.col = ci.prev_col;

    if (isPositionValid(cap.row, cap.col)) {
      cell_ref(cap.row, cap.col) = ci.id;
    }
  }
}

void Board::generateLegalMoves(PlayerID player, MoveList& moves) const {
  moves.clear();
  for (int i = 1; i <= PIECES_PER_PLAYER; ++i) {
    Move m = Move::fromRelativeIndex(i, player);
    if (isMoveValid(m, player)) {
      moves.push_back(m);
    }
  }
}

void Board::generateCaptureMoves(PlayerID player, MoveList& moves) const {
  moves.clear();
  for (int i = 1; i <= PIECES_PER_PLAYER; ++i) {
    Move m = Move::fromRelativeIndex(i, player);
    // Call the unified properties function once.
    MoveProperties props = getMoveProperties(m, player);
    if (props.is_capture) {  // is_capture implies is_valid
      moves.push_back(m);
    }
  }
}

Board::MoveProperties Board::getMoveProperties(const Move& move,
                                               PlayerID player) const {
  if (move.id < 0 || move.id >= NUM_PIECES) {
    return {};  // Default constructed: is_valid=false, is_capture=false
  }
  const int piece_id = move.id;
  const Piece& piece = pieces[piece_id];

  if (piece.owner != player || piece.isFinished()) {
    return {};
  }

  MoveResult result = calculateMoveResult(piece);
  return {result.is_valid, result.captured_count > 0};
}

bool Board::isMoveValid(const Move& move, PlayerID player) const {
  return getMoveProperties(move, player).is_valid;
}

bool Board::isCapture(const Move& move, PlayerID player) const {
  return getMoveProperties(move, player).is_capture;
}

const Piece& Board::getPiece(int piece_id) const { return pieces[piece_id]; }

const std::array<Piece, NUM_PIECES>& Board::getAllPieces() const {
  return pieces;
}

Cell Board::getCell(int row, int col) const {
  if (row < 0 || row >= NUM_ROWS || col < 0 || col >= NUM_COLS)
    return EMPTY_CELL;
  return cell_ref(row, col);
}

void Board::printBoard() const {
  for (int i = NUM_ROWS - 1; i >= 0; --i) {
    for (int j = 0; j < NUM_COLS; ++j) {
      std::cout << "|";
      const int piece_id = cell_ref(i, j);
      if (piece_id != EMPTY_CELL) {
        std::cout << " " << pieces[piece_id].to_string() << " ";
      } else {
        std::cout << "     ";
      }
    }
    std::cout << "|" << std::endl;
  }
}

}  // namespace SquadroAI
