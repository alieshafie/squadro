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

const TTEntry* TranspositionTable::probe(uint64_t key) const {
  probes++;
  size_t index = key % table_size;
  const TTEntry& entry = table[index];

  // If the zobrist key matches, the entry is for the correct position.
  if (entry.zobrist_key == key) {
    hits++;
    return &entry;
  }

  // The key does not match (hash collision) or the entry is empty.
  return nullptr;
}

}  // namespace SquadroAI
