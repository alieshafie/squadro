#pragma once

#include <vector>
#include <optional>
#include <cstdint>
#include "Constants.h"
#include "Move.h"

namespace SquadroAI
{

    enum class TTEntryType
    {
        EXACT,       // امتیاز دقیق است
        LOWER_BOUND, // امتیاز حداقل مقدار ذخیره شده است (fail high)
        UPPER_BOUND  // امتیاز حداکثر مقدار ذخیره شده است (fail low)
    };

    struct TTEntry
    {
        uint64_t zobrist_key_check; // برای بررسی برخورد هش
        Move best_move;
        int score;
        int depth;
        TTEntryType type;
        bool is_valid = false;

        TTEntry() : zobrist_key_check(0), best_move(NULL_MOVE), score(0), depth(0), type(TTEntryType::EXACT) {}
    };

    class TranspositionTable
    {
    public:
        explicit TranspositionTable(size_t size_mb); // اندازه جدول به مگابایت

        // ذخیره یک ورودی در جدول
        void store(uint64_t zobrist_key, int depth, int score, TTEntryType type, const Move &best_move);

        // جستجو برای یک ورودی در جدول
        std::optional<TTEntry> probe(uint64_t zobrist_key) const;

        void clear(); // پاک کردن جدول

    private:
        std::vector<TTEntry> table;
        size_t num_entries;

        size_t getIndex(uint64_t zobrist_key) const;
    };

} // namespace SquadroAI