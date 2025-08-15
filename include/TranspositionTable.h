#pragma once

#include <cstdint>
#include <vector>

#include "Move.h"

namespace SquadroAI {

// نوع اطلاعات ذخیره شده در جدول:
// EXACT: امتیاز دقیقا برابر با value است.
// LOWER_BOUND: امتیاز حداقل برابر با value است (یک حرکت باعث هرس بتا شد).
// UPPER_BOUND: امتیاز حداکثر برابر با value است (همه حرکات بررسی شدند و از آلفا
// بهتر نشدند).
enum class EntryType { EMPTY, EXACT, LOWER_BOUND, UPPER_BOUND };

struct TTEntry {
  uint64_t zobrist_key = 0;  // برای تشخیص برخورد (collision)
  int depth = -1;
  int score = 0;
  EntryType type = EntryType::EMPTY;
  Move best_move;  // بهترین حرکتی که از این وضعیت پیدا شده
};

class TranspositionTable {
 public:
  // سازنده: اندازه جدول را بر حسب مگابایت
  // می‌گیرد
  explicit TranspositionTable(size_t size_mb);

  // ذخیره یک ورودی جدید در جدول
  void store(uint64_t key, int depth, int score, EntryType type,
             const Move& best_move);

  // Probes the table for an entry.
  // Returns a pointer to the entry if found and valid, otherwise nullptr.
  // This is more efficient than returning by value and avoids bool return.
  const TTEntry* probe(uint64_t key) const;

  // پاک کردن جدول (مثلا برای شروع یک جستجوی جدید از ریشه)
  void clear();

  // تعداد برخوردها و بازدیدها برای آمار
  mutable long long hits = 0;
  mutable long long probes = 0;

 private:
  std::vector<TTEntry> table;
  size_t table_size;
};

}  // namespace SquadroAI
