#pragma once

#include "Constants.h"

namespace SquadroAI
{

    enum class PieceStatus
    {
        ON_BOARD_FORWARD,
        ON_BOARD_BACKWARD,
        FINISHED,
        NOT_STARTED
    };

    class Piece
    {
    public:
        PlayerID owner;
        int id;                 // 0-9
        int player_piece_index; // 0-4

        int row;
        int col;
        PieceStatus status;
        int forward_power;
        int backward_power;

        Piece(PlayerID owner = PlayerID::NONE, int id = -1, int player_piece_index = -1,
              int row = 0, int col = 0) : owner(owner), id(id), player_piece_index(player_piece_index),
                                          row(row), col(col),
                                          forward_power(forward_power), backward_power(backward_power),
                                          status(PieceStatus::NOT_STARTED)
        {
            if (owner == PlayerID::PLAYER_1)
            {
                forward_power = PLAYER_1_FWD_POWERS[player_piece_index];
                backward_power = PLAYER_1_BCK_POWERS[player_piece_index];
            }
            else if (owner == PlayerID::PLAYER_2)
            {
                forward_power = PLAYER_2_FWD_POWERS[player_piece_index];
                backward_power = PLAYER_2_BCK_POWERS[player_piece_index];
            }
        }

        int getCurrentMovePower() const { return (status == PieceStatus::ON_BOARD_FORWARD) ? forward_power : backward_power; };
        bool isFinished() const { return status == PieceStatus::FINISHED; };
    };

} // namespace SquadroAI