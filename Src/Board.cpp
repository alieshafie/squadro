#include "Board.h"

#include <algorithm>
#include <iostream>

namespace SquadroAI {

Board::Board() { initializeBoard(); }

void Board::initializeBoard() {
  grid.fill(EMPTY_CELL);

  for (int id = 0; id < NUM_PIECES; ++id) {
    pieces[id].id = id;
    pieces[id].owner = (id < 5) ? PlayerID::PLAYER_1 : PlayerID::PLAYER_2;
    pieces[id].status = PieceStatus::ON_BOARD_FORWARD;
    pieces[id].forward_power = FWD_POWERS[id];
    pieces[id].backward_power = BCK_POWERS[id];

    if (pieces[id].owner == PlayerID::PLAYER_1) {
      pieces[id].row = id + 1;
      pieces[id].col = 0;
    } else {
      pieces[id].row = 0;
      pieces[id].col = (id - 5) + 1;
    }
    get_cell_ref(pieces[id].row, pieces[id].col) = id;
  }
}

std::optional<Board::AppliedMoveInfo> Board::applyMove(
    const Move &move, PlayerID current_player) {
  if (!isMoveValid(move, current_player)) {
    return std::nullopt;
  }

  AppliedMoveInfo move_info;
  const int id = move.getid(current_player);
  Piece &mover = pieces[id];

  // ذخیره وضعیت برای undo
  move_info.move = move;
  move_info.original_mover_status = mover.status;

  get_cell_ref(mover.row, mover.col) = EMPTY_CELL;

  const bool is_forward = (mover.status == PieceStatus::ON_BOARD_FORWARD);
  const int power = is_forward ? mover.forward_power : mover.backward_power;
  const int dr =
      (current_player == PlayerID::PLAYER_2) ? (is_forward ? 1 : -1) : 0;
  const int dc =
      (current_player == PlayerID::PLAYER_1) ? (is_forward ? 1 : -1) : 0;

  int current_r = mover.row;
  int current_c = mover.col;

  for (int i = 0; i < power; ++i) {
    int next_r = current_r + dr;
    int next_c = current_c + dc;

    if (get_cell_ref(next_r, next_c) != EMPTY_CELL) {
      const int opponent_id = get_cell_ref(next_r, next_c);
      move_info.captured_piece =
          pieces[opponent_id];  // کپی از وضعیت فعلی مهره حریف
      move_info.captured_piece_valid = true;

      Piece &opponent_piece = pieces[opponent_id];
      opponent_piece.status =
          PieceStatus::ON_BOARD_FORWARD;  // همیشه به وضعیت اولیه
                                          // برمی‌گردد
      if (opponent_piece.owner == PlayerID::PLAYER_1) {
        opponent_piece.row = opponent_piece.id + 1;
        opponent_piece.col = 0;
      } else {
        opponent_piece.row = 0;
        opponent_piece.col = (opponent_piece.id - 5) + 1;
      }
      get_cell_ref(opponent_piece.row, opponent_piece.col) = opponent_id;
    }

    current_r = next_r;
    current_c = next_c;
  }

  mover.row = current_r;
  mover.col = current_c;

  bool reached_end =
      (current_player == PlayerID::PLAYER_1)
          ? (is_forward ? (mover.col == NUM_COLS - 1) : (mover.col == 0))
          : (is_forward ? (mover.row == NUM_ROWS - 1) : (mover.row == 0));

  if (reached_end) {
    mover.status =
        is_forward ? PieceStatus::ON_BOARD_BACKWARD : PieceStatus::FINISHED;
  }

  if (mover.status != PieceStatus::FINISHED) {
    get_cell_ref(mover.row, mover.col) = id;
  }

  return move_info;
}

void Board::undoMove(const AppliedMoveInfo &move_info,
                     PlayerID original_player) {
  const int id = move_info.move.getid(original_player);
  Piece &mover = pieces[id];

  // 1. بازگرداندن مهره‌ای که حرکت کرده
  // اگر مهره در حرکت قبلی تمام نشده بود،
  // خانه‌اش را خالی کن
  if (mover.status != PieceStatus::FINISHED) {
    get_cell_ref(mover.row, mover.col) = EMPTY_CELL;
  }

  mover.status = move_info.original_mover_status;
  // مکان اولیه مهره از روی ID و وضعیتش قابل محاسبه است
  if (mover.owner == PlayerID::PLAYER_1) {
    mover.col =
        (mover.status == PieceStatus::ON_BOARD_FORWARD) ? 0 : NUM_COLS - 1;
    mover.row = id + 1;
  } else {
    mover.row =
        (mover.status == PieceStatus::ON_BOARD_FORWARD) ? 0 : NUM_ROWS - 1;
    mover.col = (id - 5) + 1;
  }

  get_cell_ref(mover.row, mover.col) = id;

  // 2. بازگرداندن مهره گرفته شده (اگر وجود داشته باشد)
  if (move_info.captured_piece_valid) {
    const Piece &captured_info = move_info.captured_piece;
    // خانه‌ای که مهره گرفته شده الان به آن
    // برگشته را خالی کن
    get_cell_ref(pieces[captured_info.id].row, pieces[captured_info.id].col) =
        EMPTY_CELL;
    // وضعیت کامل مهره را بازگردان
    pieces[captured_info.id] = captured_info;
    // آن را در جای اصلی‌اش روی تخته قرار بده
    get_cell_ref(captured_info.row, captured_info.col) = captured_info.id;
  }
}

std::vector<Move> Board::generateLegalMoves(PlayerID player) const {
  std::vector<Move> legal_moves;
  legal_moves.reserve(5);

  int start_id = (player == PlayerID::PLAYER_1) ? 0 : 5;
  for (int i = 0; i < 5; ++i) {
    if (!pieces[start_id + i].isFinished()) {
      legal_moves.emplace_back(i);
    }
  }
  return legal_moves;
}

bool Board::isMoveValid(const Move &move, PlayerID player) const {
  if (move == NULL_MOVE) return false;

  const int piece_id = move.getid(player);
  const Piece &piece = pieces[piece_id];

  return !piece.isFinished() && piece.owner == player;
}

void Board::printBoard() const {
  // ... (این بخش بدون تغییر باقی می‌ماند، فقط
  // باید از pieces داخلی استفاده کند)
  for (int i = NUM_ROWS - 1; i >= 0; --i) {
    for (int j = 0; j < NUM_COLS; ++j) {
      std::cout << " | ";
      const int piece_id = get_cell_ref(i, j);
      if (piece_id != EMPTY_CELL) {
        std::cout << pieces[piece_id].to_string();
      } else {
        std::cout << "    ";
      }
    }
    std::cout << " |" << std::endl;
  }
}

const Piece &Board::getPiece(int piece_id) const { return pieces[piece_id]; }

Cell Board::getCell(int row, int col) const {
  if (row < 0 || row >= NUM_ROWS || col < 0 || col >= NUM_COLS)
    return EMPTY_CELL;
  return get_cell_ref(row, col);
}
}  // namespace SquadroAI
