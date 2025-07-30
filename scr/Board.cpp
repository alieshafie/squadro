#include "Board.h"

#include <iostream>
#include <stdexcept>
#include <vector>

namespace SquadroAI {

Board::Board() { initializeBoard(); }

void Board::initializeBoard() {
  grid.fill(EMPTY_CELL);

  for (int id = 0; id < NUM_PIECES; ++id) {
    Piece& p = pieces[id];
    p.id = id;
    p.owner = (id < 5) ? PlayerID::PLAYER_1 : PlayerID::PLAYER_2;
    p.status = PieceStatus::ON_BOARD_FORWARD;

    int player_piece_index = id % 5;
    p.forward_power = FWD_POWERS[player_piece_index];
    p.backward_power = BCK_POWERS[player_piece_index];

    if (p.owner == PlayerID::PLAYER_1) {
      p.row = player_piece_index + 1;
      p.col = 0;  // خانه شروع
    } else {      // Player 2
      p.row = 0;  // خانه شروع
      p.col = player_piece_index + 1;
    }
    cell_ref(p.row, p.col) = p.id;
  }
}

std::optional<Board::AppliedMoveInfo> Board::applyMove(
    const Move& move, PlayerID current_player) {
  const int piece_id = move.getid(current_player);

  // 1. Validate basic move parameters
  if (piece_id < 0 || piece_id >= NUM_PIECES) {
    std::cerr << "Invalid piece ID in applyMove: " << piece_id << std::endl;
    return std::nullopt;
  }

  Piece& mover = pieces[piece_id];

  // 2. Validate piece ownership and state
  if (mover.owner != current_player) {
    std::cerr << "Wrong piece owner in applyMove: expected "
              << (current_player == PlayerID::PLAYER_1 ? "P1" : "P2")
              << " but was "
              << (mover.owner == PlayerID::PLAYER_1 ? "P1" : "P2") << std::endl;
    return std::nullopt;
  }

  if (mover.isFinished()) {
    std::cerr << "Attempting to move finished piece " << piece_id
              << " in applyMove" << std::endl;
    return std::nullopt;
  }

  // 3. Calculate move parameters
  const bool is_forward = (mover.status == PieceStatus::ON_BOARD_FORWARD);
  const int power = mover.getCurrentMovePower();
  const int dr =
      (mover.owner == PlayerID::PLAYER_2) ? (is_forward ? 1 : -1) : 0;
  const int dc =
      (mover.owner == PlayerID::PLAYER_1) ? (is_forward ? 1 : -1) : 0;

  std::cerr << "Applying move: Piece " << piece_id << " at (" << mover.row
            << "," << mover.col << ") " << (is_forward ? "forward" : "backward")
            << " by " << power << " steps with dr=" << dr << " dc=" << dc
            << std::endl;

  // 4. Prepare move info
  AppliedMoveInfo move_info;
  move_info.move = move;
  move_info.original_mover_status = mover.status;
  move_info.piece_was_captured = false;

  // Store starting position
  const int start_row = mover.row;
  const int start_col = mover.col;

  // 5. Validate and simulate the complete move
  int current_r = mover.row;
  int current_c = mover.col;

  if (!isPositionValid(current_r, current_c)) {
    return std::nullopt;
  }

  // Clear starting position
  cell_ref(start_row, start_col) = EMPTY_CELL;

  // 6. Execute the move step by step
  for (int i = 0; i < power; ++i) {
    current_r += dr;
    current_c += dc;

    if (!isPositionValid(current_r, current_c)) {
      // Restore piece to starting position
      mover.row = start_row;
      mover.col = start_col;
      cell_ref(start_row, start_col) = piece_id;
      return std::nullopt;
    }

    if (cell_ref(current_r, current_c) != EMPTY_CELL) {
      const int opponent_id = cell_ref(current_r, current_c);

      if (opponent_id < 0 || opponent_id >= NUM_PIECES) {
        // Restore piece to starting position
        mover.row = start_row;
        mover.col = start_col;
        cell_ref(start_row, start_col) = piece_id;
        return std::nullopt;
      }

      Piece& opponent_piece = pieces[opponent_id];
      if (opponent_piece.owner == mover.owner) {
        // Restore piece to starting position
        mover.row = start_row;
        mover.col = start_col;
        cell_ref(start_row, start_col) = piece_id;
        return std::nullopt;
      }

      // Save capture info
      move_info.captured_piece = opponent_piece;
      move_info.piece_was_captured = true;

      // Clear opponent's current position
      cell_ref(opponent_piece.row, opponent_piece.col) = EMPTY_CELL;

      // Reset opponent piece
      opponent_piece.status = PieceStatus::ON_BOARD_FORWARD;
      int opponent_idx = opponent_id % 5;
      if (opponent_piece.owner == PlayerID::PLAYER_1) {
        opponent_piece.row = opponent_idx + 1;
        opponent_piece.col = 0;
      } else {
        opponent_piece.row = 0;
        opponent_piece.col = opponent_idx + 1;
      }

      // Make sure reset position is available
      if (cell_ref(opponent_piece.row, opponent_piece.col) != EMPTY_CELL) {
        // Restore both pieces
        opponent_piece = move_info.captured_piece;
        cell_ref(opponent_piece.row, opponent_piece.col) = opponent_id;
        mover.row = start_row;
        mover.col = start_col;
        cell_ref(start_row, start_col) = piece_id;
        return std::nullopt;
      }

      cell_ref(opponent_piece.row, opponent_piece.col) = opponent_id;
    }
  }

  // 7. Update mover's position and status
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

  // 8. Place mover in final position if not finished
  if (!mover.isFinished()) {
    cell_ref(mover.row, mover.col) = piece_id;
  }

  return move_info;
}

void Board::undoMove(const AppliedMoveInfo& move_info) {
  const PlayerID mover_owner = pieces[move_info.move.getid(PlayerID::PLAYER_1)]
                                   .owner;  // روشی برای فهمیدن بازیکن
  const int piece_id = move_info.move.getid(mover_owner);
  Piece& mover = pieces[piece_id];

  // 1. بازگرداندن مهره‌ای که حرکت کرده
  if (!mover.isFinished()) {
    cell_ref(mover.row, mover.col) = EMPTY_CELL;  // خالی کردن خانه فعلی
  }

  // بازگرداندن وضعیت قبلی مهره
  mover.status = move_info.original_mover_status;

  // Reset mover and captured piece positions
  int player_piece_index = piece_id % 5;

  // Calculate proper reset position based on original status
  // For moving piece
  if (mover.owner == PlayerID::PLAYER_1) {
    mover.col =
        (move_info.original_mover_status == PieceStatus::ON_BOARD_FORWARD)
            ? 0
            : NUM_COLS - 1;
    mover.row = player_piece_index + 1;
  } else {  // Player 2
    mover.row =
        (move_info.original_mover_status == PieceStatus::ON_BOARD_FORWARD)
            ? 0
            : NUM_ROWS - 1;
    mover.col = player_piece_index + 1;
  }

  // Clear old position and set new one
  cell_ref(mover.row, mover.col) = piece_id;

  // 2. Restore captured piece if there was one
  if (move_info.piece_was_captured) {
    const Piece& captured_info = move_info.captured_piece;

    // Remove from current position if needed
    int curr_row = pieces[captured_info.id].row;
    int curr_col = pieces[captured_info.id].col;
    if (isPositionValid(curr_row, curr_col)) {
      cell_ref(curr_row, curr_col) = EMPTY_CELL;
    }

    // Restore captured piece's state
    pieces[captured_info.id] = captured_info;

    // Place back on board
    if (isPositionValid(captured_info.row, captured_info.col)) {
      cell_ref(captured_info.row, captured_info.col) = captured_info.id;
    }
  }
}

std::vector<Move> Board::generateLegalMoves(PlayerID player) const {
  std::vector<Move> legal_moves;
  legal_moves.reserve(5);  // بهینه‌سازی: از re-allocation
                           // جلوگیری می‌کند

  int start_id = (player == PlayerID::PLAYER_1) ? 0 : 5;
  for (int i = 0; i < 5; ++i) {
    if (!pieces[start_id + i].isFinished()) {
      Move m(i);
      if (isMoveValid(m, player)) {
        legal_moves.emplace_back(i);  // i همان player_piece_index است
      }
    }
  }
  return legal_moves;
}

bool Board::isMoveValid(const Move& move, PlayerID player) const {
  // Check piece index is valid
  if (move.piece_index < 0 || move.piece_index > 4) {
    std::cerr << "Invalid piece index: " << move.piece_index << std::endl;
    return false;
  }

  // Get piece info
  const int piece_id = move.getid(player);
  if (piece_id < 0 || piece_id >= NUM_PIECES) {
    std::cerr << "Invalid piece id: " << piece_id << std::endl;
    return false;
  }

  const Piece& piece = pieces[piece_id];

  // Check basic validity
  if (piece.owner != player) {
    std::cerr << "Wrong piece owner: expected "
              << (player == PlayerID::PLAYER_1 ? "P1" : "P2") << " but was "
              << (piece.owner == PlayerID::PLAYER_1 ? "P1" : "P2") << std::endl;
    return false;
  }

  if (piece.isFinished()) {
    std::cerr << "Piece " << move.piece_index << " is finished" << std::endl;
    return false;
  }

  // Calculate move parameters
  const bool is_forward = (piece.status == PieceStatus::ON_BOARD_FORWARD);
  const int power = piece.getCurrentMovePower();
  const int dr =
      (piece.owner == PlayerID::PLAYER_2) ? (is_forward ? 1 : -1) : 0;
  const int dc =
      (piece.owner == PlayerID::PLAYER_1) ? (is_forward ? 1 : -1) : 0;

  // Print current and planned movement for debugging
  std::cerr << "Validating move: Piece " << move.piece_index << " at ("
            << piece.row << "," << piece.col << ") "
            << (is_forward ? "forward" : "backward") << " by " << power
            << " steps with dr=" << dr << " dc=" << dc << std::endl;

  // Check each step of the move
  int current_r = piece.row;
  int current_c = piece.col;

  // Validate starting position
  if (!isPositionValid(current_r, current_c)) {
    std::cerr << "Invalid starting position (" << current_r << "," << current_c
              << ")" << std::endl;
    return false;
  }

  // Simulate the move step by step
  for (int i = 0; i < power; ++i) {
    current_r += dr;
    current_c += dc;

    // Check position is valid
    if (!isPositionValid(current_r, current_c)) {
      std::cerr << "Step " << i + 1 << " position (" << current_r << ","
                << current_c << ") is invalid" << std::endl;
      return false;
    }

    // Check for captures
    if (cell_ref(current_r, current_c) != EMPTY_CELL) {
      const int opponent_id = cell_ref(current_r, current_c);

      // Validate opponent piece
      if (opponent_id < 0 || opponent_id >= NUM_PIECES) {
        std::cerr << "Invalid opponent piece id at (" << current_r << ","
                  << current_c << "): " << opponent_id << std::endl;
        return false;
      }

      const Piece& opponent = pieces[opponent_id];
      if (opponent.owner == player) {
        std::cerr << "Cannot capture own piece at (" << current_r << ","
                  << current_c << ")" << std::endl;
        return false;
      }

      // Check reset position for captured piece
      int opponent_idx = opponent_id % 5;
      int reset_row =
          opponent.owner == PlayerID::PLAYER_1 ? opponent_idx + 1 : 0;
      int reset_col =
          opponent.owner == PlayerID::PLAYER_1 ? 0 : opponent_idx + 1;

      // Make sure reset position is available
      if (cell_ref(reset_row, reset_col) != EMPTY_CELL &&
          cell_ref(reset_row, reset_col) != opponent_id) {
        std::cerr << "Reset position (" << reset_row << "," << reset_col
                  << ") is blocked for piece " << opponent_id << std::endl;
        return false;
      }
    }
  }

  return true;
}

// توابع Getter
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
