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

std::optional<Board::AppliedMoveInfo> Board::applyMove(
    const Move& move, PlayerID current_player) {
  const int piece_id = move.id;

  if (piece_id < 0 || piece_id >= NUM_PIECES) {
    return std::nullopt;
  }

  Piece& mover = pieces[piece_id];

  if (mover.owner != current_player || mover.isFinished()) {
    return std::nullopt;
  }

  const bool is_forward = (mover.status == PieceStatus::ON_BOARD_FORWARD);
  const int power = mover.getCurrentMovePower();
  const int dr =
      (mover.owner == PlayerID::PLAYER_2) ? (is_forward ? 1 : -1) : 0;
  const int dc =
      (mover.owner == PlayerID::PLAYER_1) ? (is_forward ? 1 : -1) : 0;

  AppliedMoveInfo move_info;
  move_info.mover_id = static_cast<uint8_t>(piece_id);
  move_info.start_row = static_cast<int8_t>(mover.row);
  move_info.start_col = static_cast<int8_t>(mover.col);
  move_info.original_mover_status = static_cast<uint8_t>(mover.status);
  move_info.captured_count = 0;

  const int start_row = mover.row;
  const int start_col = mover.col;

  if (!isPositionValid(start_row, start_col)) {
    return std::nullopt;
  }

  cell_ref(start_row, start_col) = EMPTY_CELL;

  int current_r = start_row;
  int current_c = start_col;

  // Flag: وقتی capture رخ دهد حرکت پایان می‌یابد (طبق
  // خواستهٔ تو)
  bool capture_happened = false;

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
      // آغاز capture chain — پس از حل chain حرکت تمام
      // می‌شود
      capture_happened = true;

      while (isPositionValid(current_r, current_c) &&
             cell_ref(current_r, current_c) != EMPTY_CELL) {
        const int opponent_id = cell_ref(current_r, current_c);
        if (opponent_id < 0 || opponent_id >= NUM_PIECES) {
          // بازگردانی سریع و خطا
          mover.row = start_row;
          mover.col = start_col;
          cell_ref(start_row, start_col) = piece_id;
          return std::nullopt;
        }

        Piece& opponent_piece = pieces[opponent_id];
        if (opponent_piece.owner == mover.owner) {
          // can't capture own piece -> rollback
          mover.row = start_row;
          mover.col = start_col;
          cell_ref(start_row, start_col) = piece_id;
          return std::nullopt;
        }

        if (move_info.captured_count >= MAX_POSSIBLE_CAPTURES) {
          mover.row = start_row;
          mover.col = start_col;
          cell_ref(start_row, start_col) = piece_id;
          return std::nullopt;
        }

        auto& ci = move_info.captures[move_info.captured_count++];
        ci.id = static_cast<uint8_t>(opponent_piece.id);
        ci.prev_row = static_cast<int8_t>(opponent_piece.row);
        ci.prev_col = static_cast<int8_t>(opponent_piece.col);

        // پاک کردن خانهٔ فعلیِ حریف
        cell_ref(opponent_piece.row, opponent_piece.col) = EMPTY_CELL;

        // تعیین مکان ریست بر اساس وضعیت فعلیِ حریف (status را تغییر
        // نمی‌دهیم)
        int opponent_idx = opponent_id % PIECES_PER_PLAYER;
        int reset_row, reset_col;

        if (opponent_piece.status == PieceStatus::ON_BOARD_FORWARD) {
          // ریست به home (trough) مطابق state forward
          if (opponent_piece.owner == PlayerID::PLAYER_1) {
            reset_row = opponent_idx + 1;
            reset_col = 0;
          } else {
            reset_row = 0;
            reset_col = opponent_idx + 1;
          }
        } else {
          // status == ON_BOARD_BACKWARD (یا هر حالت بازگشتی): ریست به نقطهٔ
          // انتهایی مسیر
          if (opponent_piece.owner == PlayerID::PLAYER_1) {
            reset_row = opponent_idx + 1;
            reset_col = NUM_COLS - 1;
          } else {
            reset_row = NUM_ROWS - 1;
            reset_col = opponent_idx + 1;
          }
        }

        // بررسی در دسترس بودن مکان ریست
        if (cell_ref(reset_row, reset_col) != EMPTY_CELL &&
            cell_ref(reset_row, reset_col) != opponent_id) {
          // rollback همهٔ captures ذخیره‌شده تا کنون و
          // restore mover
          for (uint8_t k = 0; k < move_info.captured_count; ++k) {
            const auto& rci = move_info.captures[k];
            pieces[rci.id].row = rci.prev_row;
            pieces[rci.id].col = rci.prev_col;
            cell_ref(rci.prev_row, rci.prev_col) = rci.id;
          }
          mover.row = start_row;
          mover.col = start_col;
          mover.status =
              static_cast<PieceStatus>(move_info.original_mover_status);
          cell_ref(start_row, start_col) = piece_id;
          return std::nullopt;
        }

        // انتقال حریف به reset location — **status تغییر
        // نمی‌کند**
        opponent_piece.row = reset_row;
        opponent_piece.col = reset_col;
        cell_ref(reset_row, reset_col) = opponent_id;

        // mover باید به خانهٔ پس از موقعیتِ
        // گرفته‌شده برود
        int beyond_r = current_r + dr;
        int beyond_c = current_c + dc;

        if (!isPositionValid(beyond_r, beyond_c)) {
          clamp_to_edge(beyond_r, beyond_c, dr, dc);
          current_r = beyond_r;
          current_c = beyond_c;
          break;  // برخورد به لبه => خاتمهٔ chain
        } else {
          current_r = beyond_r;
          current_c = beyond_c;
          // ادامهٔ while برای بررسی chain بعدی
          continue;
        }
      }  // end while chain

      // حرکت بلافاصله بعد از حل chain تمام می‌شود
      break;
    }  // end if occupied
  }  // end for steps

  // Store destination coordinates before updating the mover piece
  move_info.dest_row = static_cast<int8_t>(current_r);
  move_info.dest_col = static_cast<int8_t>(current_c);

  // به‌روزرسانی موقعیت mover
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

  // قرار دادن mover در صفحه در صورت فعال بودن
  if (!mover.isFinished()) {
    if (cell_ref(mover.row, mover.col) != EMPTY_CELL) {
      // rollback captures and restore mover
      for (uint8_t k = 0; k < move_info.captured_count; ++k) {
        const auto& rci = move_info.captures[k];
        pieces[rci.id].row = rci.prev_row;
        pieces[rci.id].col = rci.prev_col;
        cell_ref(rci.prev_row, rci.prev_col) = rci.id;
      }
      mover.row = start_row;
      mover.col = start_col;
      mover.status = static_cast<PieceStatus>(move_info.original_mover_status);
      cell_ref(start_row, start_col) = piece_id;
      return std::nullopt;
    }
    cell_ref(mover.row, mover.col) = piece_id;
  } else {
    // finished -> no placement in grid
  }

  return move_info;
}

void Board::undoMove(const AppliedMoveInfo& move_info) {
  const int piece_id = static_cast<int>(move_info.mover_id);
  Piece& mover = pieces[piece_id];

  // پاک کردن جای فعلی mover در صورت وجود
  if (!mover.isFinished() && isPositionValid(mover.row, mover.col)) {
    if (cell_ref(mover.row, mover.col) == piece_id)
      cell_ref(mover.row, mover.col) = EMPTY_CELL;
  }

  // بازگرداندن وضعیت و مکان mover به حالت قبل
  mover.status = static_cast<PieceStatus>(move_info.original_mover_status);
  mover.row = move_info.start_row;
  mover.col = move_info.start_col;
  if (!mover.isFinished() && isPositionValid(mover.row, mover.col)) {
    cell_ref(mover.row, mover.col) = piece_id;
  }

  // بازگردانی captures به حالتِ prev_row/prev_col (status تغییر نکرده بود)
  // بازگردانی به ترتیب معکوس برای اطمینان از سالم بودن
  // سلول‌ها
  for (int k = static_cast<int>(move_info.captured_count) - 1; k >= 0; --k) {
    const auto& ci = move_info.captures[k];
    Piece& cap = pieces[ci.id];

    // پاک‌سازی مکان فعلی (که در apply به آن منتقل
    // شده بود)
    if (isPositionValid(cap.row, cap.col) &&
        cell_ref(cap.row, cap.col) == ci.id) {
      cell_ref(cap.row, cap.col) = EMPTY_CELL;
    }

    // بازگردانی مکان قبلی (status نیازی به بازنشانی ندارد)
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
    int global_id = m.id;
    if (global_id < 0 || global_id >= NUM_PIECES) continue;
    if (!pieces[global_id].isFinished()) {
      if (isMoveValid(m, player)) {
        moves.push_back(m);
      }
    }
  }
}

void Board::generateCaptureMoves(PlayerID player, MoveList& moves) const {
  moves.clear();
  for (int i = 1; i <= PIECES_PER_PLAYER; ++i) {
    Move m = Move::fromRelativeIndex(i, player);
    const int global_id = m.id;
    if (global_id < 0 || global_id >= NUM_PIECES) continue;

    const auto& piece = pieces[global_id];
    if (piece.isFinished()) continue;

    bool is_potential_capture = false;
    const bool is_forward = (piece.status == PieceStatus::ON_BOARD_FORWARD);
    const int power = piece.getCurrentMovePower();
    const int dr =
        (piece.owner == PlayerID::PLAYER_2) ? (is_forward ? 1 : -1) : 0;
    const int dc =
        (piece.owner == PlayerID::PLAYER_1) ? (is_forward ? 1 : -1) : 0;

    int r = piece.row;
    int c = piece.col;

    for (int step = 0; step < power; ++step) {
      r += dr;
      c += dc;
      if (!isPositionValid(r, c)) break;

      if (cell_ref(r, c) != EMPTY_CELL) {
        const int opponent_id = cell_ref(r, c);
        if (opponent_id >= 0 && opponent_id < NUM_PIECES &&
            pieces[opponent_id].owner != player) {
          is_potential_capture = true;
        }
        break;
      }
    }

    if (is_potential_capture) {
      if (isMoveValid(m, player)) {
        moves.push_back(m);
      }
    }
  }
}

bool Board::isMoveValid(const Move& move, PlayerID player) const {
  if (move.id < 0 || move.id >= NUM_PIECES) {
    return false;
  }

  const int piece_id = move.id;
  const Piece& piece = pieces[piece_id];

  if (piece.owner != player || piece.isFinished()) return false;

  const bool is_forward = (piece.status == PieceStatus::ON_BOARD_FORWARD);
  const int power = piece.getCurrentMovePower();
  const int dr =
      (piece.owner == PlayerID::PLAYER_2) ? (is_forward ? 1 : -1) : 0;
  const int dc =
      (piece.owner == PlayerID::PLAYER_1) ? (is_forward ? 1 : -1) : 0;

  int current_r = piece.row;
  int current_c = piece.col;

  if (!isPositionValid(current_r, current_c)) return false;

  for (int i = 0; i < power; ++i) {
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
      const int opponent_id = cell_ref(current_r, current_c);
      if (opponent_id < 0 || opponent_id >= NUM_PIECES) return false;

      const Piece& opponent = pieces[opponent_id];
      if (opponent.owner == player) return false;

      // تعیین ریست تراق مطابق status فعلیِ opponent (بدون تغییر status)
      int opponent_idx = opponent_id % PIECES_PER_PLAYER;
      int reset_row, reset_col;

      if (opponent.status == PieceStatus::ON_BOARD_FORWARD) {
        if (opponent.owner == PlayerID::PLAYER_1) {
          reset_row = opponent_idx + 1;
          reset_col = 0;
        } else {
          reset_row = 0;
          reset_col = opponent_idx + 1;
        }
      } else {
        if (opponent.owner == PlayerID::PLAYER_1) {
          reset_row = opponent_idx + 1;
          reset_col = NUM_COLS - 1;
        } else {
          reset_row = NUM_ROWS - 1;
          reset_col = opponent_idx + 1;
        }
      }

      if (cell_ref(reset_row, reset_col) != EMPTY_CELL &&
          cell_ref(reset_row, reset_col) != opponent_id) {
        return false;
      }

      // شبیه‌سازی chain: mover به خانهٔ بعد از opponent
      // می‌رود
      int beyond_r = current_r + dr;
      int beyond_c = current_c + dc;

      if (!isPositionValid(beyond_r, beyond_c)) {
        clamp_to_edge(beyond_r, beyond_c, dr, dc);
        current_r = beyond_r;
        current_c = beyond_c;
        break;
      } else {
        current_r = beyond_r;
        current_c = beyond_c;
        // بررسی زنجیرهٔ بعدی (تا زمانی که خانهٔ جدید
        // اشغال‌شده باشد)
        while (isPositionValid(current_r, current_c) &&
               cell_ref(current_r, current_c) != EMPTY_CELL) {
          const int op2 = cell_ref(current_r, current_c);
          if (op2 < 0 || op2 >= NUM_PIECES) return false;
          const Piece& opponent2 = pieces[op2];
          if (opponent2.owner == player) return false;

          int op2_idx = op2 % PIECES_PER_PLAYER;
          int rtr, ctr;
          if (opponent2.status == PieceStatus::ON_BOARD_FORWARD) {
            if (opponent2.owner == PlayerID::PLAYER_1) {
              rtr = op2_idx + 1;
              ctr = 0;
            } else {
              rtr = 0;
              ctr = op2_idx + 1;
            }
          } else {
            if (opponent2.owner == PlayerID::PLAYER_1) {
              rtr = op2_idx + 1;
              ctr = NUM_COLS - 1;
            } else {
              rtr = NUM_ROWS - 1;
              ctr = op2_idx + 1;
            }
          }

          if (cell_ref(rtr, ctr) != EMPTY_CELL && cell_ref(rtr, ctr) != op2)
            return false;

          int br = current_r + dr;
          int bc = current_c + dc;
          if (!isPositionValid(br, bc)) {
            clamp_to_edge(br, bc, dr, dc);
            current_r = br;
            current_c = bc;
            break;
          } else {
            current_r = br;
            current_c = bc;
            // ادامه chain
          }
        }
        // پس از chain شبیه‌سازی، حرکت خاتمه
        // می‌یابد
        break;
      }
    }
  }

  return true;
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
