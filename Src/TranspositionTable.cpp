#include "TranspositionTable.h"

SquadroAI::TranspositionTable::TranspositionTable(size_t size_mb)
    : num_entries{ size_mb }
{
}

void SquadroAI::TranspositionTable::store(uint64_t zobrist_key, int depth, int score, TTEntryType type, const Move& best_move);