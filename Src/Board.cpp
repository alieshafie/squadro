#include "Board.h"

#include <iostream>
#include <stdexcept>

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
  if (!isMoveValid(move, current_player)) {
    return std::nullopt;
  }

  AppliedMoveInfo move_info;
  move_info.move = move;
  const int piece_id = move.getid(current_player);
  Piece& mover = pieces[piece_id];

  // 1. ذخیره اطلاعات برای Undo
  move_info.original_mover_status = mover.status;

  // 2. خالی کردن خانه فعلی مهره
  cell_ref(mover.row, mover.col) = EMPTY_CELL;

  // 3. محاسبه جهت و مقدار حرکت
  const bool is_forward = (mover.status == PieceStatus::ON_BOARD_FORWARD);
  const int power = mover.getCurrentMovePower();
  const int dr =
      (mover.owner == PlayerID::PLAYER_2) ? (is_forward ? 1 : -1) : 0;
  const int dc =
      (mover.owner == PlayerID::PLAYER_1) ? (is_forward ? 1 : -1) : 0;

  // 4. شبیه‌سازی حرکت و برخوردها
  int current_r = mover.row;
  int current_c = mover.col;

  for (int i = 0; i < power; ++i) {
    current_r += dr;
    current_c += dc;

    if (cell_ref(current_r, current_c) != EMPTY_CELL) {
      const int opponent_id = cell_ref(current_r, current_c);
      Piece& opponent_piece = pieces[opponent_id];

      // یک مهره در مسیر است، آن را به خانه اول برگردان
      move_info.captured_piece = opponent_piece;  // ذخیره کپی کامل برای undo
      move_info.piece_was_captured = true;

      opponent_piece.status = PieceStatus::ON_BOARD_FORWARD;
      int opponent_player_piece_idx = opponent_id % 5;
      if (opponent_piece.owner == PlayerID::PLAYER_1) {
        opponent_piece.row = opponent_player_piece_idx + 1;
        opponent_piece.col = 0;
      } else {
        opponent_piece.row = 0;
        opponent_piece.col = opponent_player_piece_idx + 1;
      }
      // اطمینان از اینکه خانه جدید حریف خالی است (نباید اتفاق بیفتد مگر در
      // قوانین خاص)
      if (cell_ref(opponent_piece.row, opponent_piece.col) != EMPTY_CELL) {
        // این حالت در بازی استاندارد نباید رخ دهد
        // اما برای اطمینان می‌توان یک خطا پرتاب
        // کرد یا لاگ گرفت
      }
      cell_ref(opponent_piece.row, opponent_piece.col) = opponent_id;
    }
  }

  // 5. به‌روزرسانی مکان نهایی مهره
  mover.row = current_r;
  mover.col = current_c;

  // 6. بررسی وضعیت جدید مهره (رسیدن به انتها، تمام شدن بازی)
  bool reached_end;
  if (mover.owner == PlayerID::PLAYER_1) {
    reached_end = is_forward ? (mover.col == NUM_COLS - 1) : (mover.col == 0);
  } else {  // Player 2
    reached_end = is_forward ? (mover.row == NUM_ROWS - 1) : (mover.row == 0);
  }

  if (reached_end) {
    mover.status =
        is_forward ? PieceStatus::ON_BOARD_BACKWARD : PieceStatus::FINISHED;
  }

  // 7. قرار دادن مهره در خانه جدیدش (اگر تمام نشده باشد)
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

  // محاسبه مکان اولیه از روی ID و وضعیت قبلی
  int player_piece_index = piece_id % 5;
  if (mover.owner == PlayerID::PLAYER_1) {
    mover.col =
        (mover.status == PieceStatus::ON_BOARD_FORWARD) ? 0 : NUM_COLS - 1;
    mover.row = player_piece_index + 1;
  } else {
    mover.row =
        (mover.status == PieceStatus::ON_BOARD_FORWARD) ? 0 : NUM_ROWS - 1;
    mover.col = player_piece_index + 1;
  }
  cell_ref(mover.row, mover.col) = piece_id;

  // 2. بازگرداندن مهره گرفته شده (در صورت وجود)
  if (move_info.piece_was_captured) {
    const Piece& captured_info = move_info.captured_piece;
    // خانه‌ای که مهره گرفته‌شده به آن برگشته
    // بود را خالی می‌کنیم
    cell_ref(pieces[captured_info.id].row, pieces[captured_info.id].col) =
        EMPTY_CELL;

    // وضعیت کامل مهره را از کپی ذخیره شده بازگردانی
    // می‌کنیم
    pieces[captured_info.id] = captured_info;

    // مهره را به جای اصلی‌اش روی تخته
    // برمی‌گردانیم
    cell_ref(captured_info.row, captured_info.col) = captured_info.id;
  }
}

std::vector<Move> Board::generateLegalMoves(PlayerID player) const {
  std::vector<Move> legal_moves;
  legal_moves.reserve(5);  // بهینه‌سازی: از re-allocation
                           // جلوگیری می‌کند

  int start_id = (player == PlayerID::PLAYER_1) ? 0 : 5;
  for (int i = 0; i < 5; ++i) {
    if (!pieces[start_id + i].isFinished()) {
      legal_moves.emplace_back(i);  // i همان player_piece_index است
    }
  }
  return legal_moves;
}

bool Board::isMoveValid(const Move& move, PlayerID player) const {
  if (move.piece_index < 0 || move.piece_index > 4) return false;
  const int piece_id = move.getid(player);
  return pieces[piece_id].owner == player && !pieces[piece_id].isFinished();
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
  // ... (پیاده‌سازی این تابع برای نمایش
  // گرافیکی در کنسول)
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
