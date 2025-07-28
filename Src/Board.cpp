#include "Board.h"

#include <algorithm>  // برای std::fill
#include <iostream>

namespace SquadroAI {
Board::Board() {
  // مقداردهی اولیه کل تخته با مقدار نگهبان به صورت بهینه
  grid.fill(EMPTY_CELL);
  initializeBoard();
}

void Board::initializeBoard() {
  grid.fill(EMPTY_CELL);  // اطمینان از خالی بودن تخته قبل از چیدن
                          // مهره‌ها

  // قرار دادن مهره‌های بازیکن 1
  for (int i = 0; i < 5; ++i) {
    get_cell_ref(i + 1, 0) = i;  // مهره‌های بازیکن 1 (ID 0-4)
  }

  // قرار دادن مهره‌های بازیکن 2
  for (int j = 0; j < 5; ++j) {
    get_cell_ref(0, j + 1) =
        j + 5;  // مهره‌های بازیکن 2 (ID 5-9)
  }
}

std::optional<Board::AppliedMoveInfo> Board::applyMove(
    const Move &move, PlayerID current_player, std::vector<Piece> &pieces) {
  if (!isMoveValid(move, current_player, pieces)) {
    return std::nullopt;
  }

  AppliedMoveInfo move_info;
  const int id = move.getid(current_player);
  Piece &mover = pieces[id];

  // ذخیره وضعیت اولیه برای undo
  move_info.move = move;
  move_info.original_mover_status = mover.status;
  move_info.original_mover_owner = current_player;
  move_info.original_mover_row = mover.row;
  move_info.original_mover_col = mover.col;

  get_cell_ref(mover.row, mover.col) =
      EMPTY_CELL;  // پاک کردن مهره از مکان فعلی

  const bool is_forward = (mover.status == PieceStatus::ON_BOARD_FORWARD);
  const int power = is_forward ? mover.forward_power : mover.backward_power;
  const int dr =
      (current_player == PlayerID::PLAYER_2) ? (is_forward ? 1 : -1) : 0;
  const int dc =
      (current_player == PlayerID::PLAYER_1) ? (is_forward ? 1 : -1) : 0;

  for (int i = 0; i < power; ++i) {
    int next_row = mover.row + dr;
    int next_col = mover.col + dc;

    bool reached_end =
        (current_player == PlayerID::PLAYER_1)
            ? (is_forward ? (next_col == NUM_COLS - 1) : (next_col == 0))
            : (is_forward ? (next_row == NUM_ROWS - 1) : (next_row == 0));

    if (reached_end) {
      mover.row = next_row;
      mover.col = next_col;
      mover.status =
          is_forward ? PieceStatus::ON_BOARD_BACKWARD : PieceStatus::FINISHED;
      break;
    }

    if (get_cell_ref(next_row, next_col) != EMPTY_CELL) {
      const int opponent_id = get_cell_ref(next_row, next_col);
      Piece &opponent_piece = pieces[opponent_id];

      move_info.captured_piece = opponent_piece;
      move_info.captured_piece_valid = true;

      if (opponent_piece.owner == PlayerID::PLAYER_1) {
        opponent_piece.row =
            opponent_piece.status == PieceStatus::ON_BOARD_FORWARD
                ? opponent_piece.row
                : (NUM_ROWS - 1);
        opponent_piece.col = 0;
      } else {
        opponent_piece.col =
            opponent_piece.status == PieceStatus::ON_BOARD_FORWARD
                ? opponent_piece.col
                : (NUM_COLS - 1);
        opponent_piece.row = 0;
      }

      mover.row = next_row + dr;
      mover.col = next_col + dc;
      break;
    }

    mover.row = next_row;
    mover.col = next_col;
  }

  if (mover.status != PieceStatus::FINISHED) {
    get_cell_ref(mover.row, mover.col) = id;
  }

  return move_info;
}

void Board::undoMove(const AppliedMoveInfo &move_info,
                     std::vector<Piece> &pieces) {
  const Move &move = move_info.move;
  const int id = move.getid(move_info.original_mover_owner);
  Piece &mover = pieces[id];

  if (mover.status != PieceStatus::FINISHED) {
    get_cell_ref(mover.row, mover.col) = EMPTY_CELL;
  }

  mover.status = move_info.original_mover_status;
  mover.row = move_info.original_mover_row;
  mover.col = move_info.original_mover_col;
  get_cell_ref(mover.row, mover.col) = id;

  if (move_info.captured_piece_valid) {
    const Piece &captured_info = move_info.captured_piece;
    Piece &captured_piece = pieces[captured_info.id];
    captured_piece = captured_info;
    get_cell_ref(captured_piece.row, captured_piece.col) = captured_piece.id;
  }
}

std::vector<Move> Board::generateLegalMoves(
    PlayerID player, const std::vector<Piece> &pieces) const {
  std::vector<Move> legal_moves;
  legal_moves.reserve(5);

  int start_id = (player == PlayerID::PLAYER_1) ? 0 : 5;
  for (int i = 0; i < 5; ++i) {
    Move move(i);
    if (isMoveValid(move, player, pieces)) {
      legal_moves.push_back(move);
    }
  }
  return legal_moves;
}

bool Board::isMoveValid(const Move &move, PlayerID current_player,
                        const std::vector<Piece> &pieces) const {
  if (move == NULL_MOVE) return false;

  const int piece_id = move.getid(current_player);
  const Piece &mover = pieces[piece_id];

  if (mover.isFinished() || mover.owner != current_player) return false;

  // بهینه‌سازی: چک کردن سریع و امن با مقدار نگهبان.
  // این دستور بسیار
  // سریع‌تر از نسخه std::optional است و در مقابل کرش
  // کردن امن است.
  if (get_cell_ref(mover.row, mover.col) != mover.id) return false;

  return true;
}

void Board::printBoard(const std::vector<Piece> &pieces) const {
  for (int i = NUM_ROWS - 1; i >= 0; --i) {
    for (int j = 0; j < NUM_COLS; ++j) {
      std::cout << " | ";
      const int piece_id = get_cell_ref(i, j);
      if (piece_id != EMPTY_CELL) {
        const Piece &piece = pieces[piece_id];
        std::cout << piece.to_string() << " | ";
      } else {
        std::cout << "EMPTY | ";
      }
    }
    std::cout << std::endl;
  }
}

Cell Board::getCell(int row, int col) const {
  if (row < 0 || row >= NUM_ROWS || col < 0 || col >= NUM_COLS)
    return EMPTY_CELL;  // بازگرداندن مقدار امن

  return get_cell_ref(row, col);
}

void Board::setCell(int row, int col, Cell piece_id) {
  if (row < 0 || row >= NUM_ROWS || col < 0 || col >= NUM_COLS)
    return;  // انجام ندادن هیچ کاری در صورت خارج از محدوده بودن

  get_cell_ref(row, col) = piece_id;
}
}  // namespace SquadroAI
