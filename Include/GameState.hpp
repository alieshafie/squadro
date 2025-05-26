#pragma once
#include "Piece.hpp"
#include "Zobrist.hpp"
#include <array>
#include <vector>
#include <cstdint>

struct Move
{
    uint8_t pieceId;
};

class GameState
{
public:
    static constexpr int N = 5;
    std::array<Piece, N> myPieces;
    std::array<Piece, N> opPieces;
    int turn;

    GameState();
    void applyMove(const Move &m);
    std::vector<Move> legalMoves() const;
    bool isTerminal() const;
    uint64_t zobristHash() const;
};