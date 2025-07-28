#pragma once

#include <string>
#include <stdexcept>

namespace SquadroAI
{

    struct Move
    {
        int piece_index; //(0-4) نسبی

        Move(int piece_index = -1)
        {
            if (piece_index < -1 || piece_index > 4)
            {
                throw std::invalid_argument("Invalid piece index");
                this->piece_index = -1;
            }
            this->piece_index = piece_index;
        }
        int getid(PlayerID player) const
        {
            int id = piece_index;
            if (player == PlayerID::PLAYER_2)
            {
                id = 5 + piece_index;
            }
            return id;
        }
        bool operator==(const Move& other) const { return piece_index == other.piece_index; }

        // برای استفاده در std::map یا std::set اگر لازم باشد
        bool operator<(const Move& other) const { return piece_index < other.piece_index; }

        std::string to_string() const
        {
            if (piece_index == -1)
                return "NULL_MOVE";
            return "P" + std::to_string(piece_index);
        }
        // یک حرکت "نامعتبر" یا "تهی"
    };
    const Move NULL_MOVE = Move(-1);


} // namespace SquadroAI