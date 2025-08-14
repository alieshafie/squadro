#include "TranspositionTable.h"

namespace SquadroAI {

TranspositionTable::TranspositionTable(size_t size_mb) {
  table_size = (size_mb * 1024 * 1024) / sizeof(TTEntry);
  table.resize(table_size);
  clear();
}

void TranspositionTable::clear() {
  // به جای ساختن دوباره، فقط ورودی‌ها را پاک می‌کنیم
  // که سریع‌تر است.
  std::fill(table.begin(), table.end(), TTEntry());
  hits = 0;
  probes = 0;
}

void TranspositionTable::store(uint64_t key, int depth, int score,
                               EntryType type, const Move& best_move) {
  size_t index = key % table_size;

  // استراتژی جایگزینی: همیشه جایگزین کن، مگر اینکه ورودی موجود از عمق بیشتری
  // باشد. این یک استراتژی ساده و مؤثر است.
  if (table[index].type == EntryType::EMPTY || depth >= table[index].depth) {
    table[index].zobrist_key = key;
    table[index].depth = depth;
    table[index].score = score;
    table[index].type = type;
    table[index].best_move = best_move;
  }
}

bool TranspositionTable::probe(uint64_t key, int depth, int& score,
                               Move& best_move) const {
  probes++;
  size_t index = key % table_size;
  const TTEntry& entry = table[index];

  if (entry.zobrist_key == key) {
    // The key matches, so this entry is for the current board state.
    // We can always use the best_move for move ordering.
    best_move = entry.best_move;

    // Now, check if the stored depth is sufficient for a score cutoff.
    if (entry.depth >= depth) {
      // TODO: Add bound checks (alpha/beta) here for more precise cutoffs
      score = entry.score;
      hits++;
      return true;  // We have a full hit, score can be used.
    }

    // Key matched, but depth was too shallow. The best_move is still useful
    // for ordering, but we can't use the score.
    return false;
  }

  // The key does not match (hash collision). Do not use any information from
  // this entry.
  return false;
}

}  // namespace SquadroAI
