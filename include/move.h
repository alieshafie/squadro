#pragma once

#include <string>

namespace SquadroAI
{

    struct Move
    {
        int piece_index; // اندیس مهره (0 تا 4 برای هر بازیکن)
        // ممکن است اطلاعات بیشتری مانند مختصات شروع و پایان برای سادگی در برخی موارد اضافه شود،
        // اما معمولاً فقط اندیس مهره کافی است و قوانین بازی بقیه را تعیین می‌کنند.
        // int from_row, from_col;
        // int to_row, to_col;

        Move(int p_index = -1) : piece_index(p_index) {}

        bool operator==(const Move &other) const
        {
            return piece_index == other.piece_index;
        }

        // برای استفاده در std::map یا std::set اگر لازم باشد
        bool operator<(const Move &other) const
        {
            return piece_index < other.piece_index;
        }

        std::string to_string() const
        {
            if (piece_index == -1)
                return "NULL_MOVE";
            return "P" + std::to_string(piece_index);
        }
    };

    // یک حرکت "نامعتبر" یا "تهی"
    const Move NULL_MOVE = Move(-1);

} // namespace SquadroAI