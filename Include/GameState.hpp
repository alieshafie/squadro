#pragma once
#include "Piece.hpp"
#include "Zobrist.hpp"
#include "utils.hpp"
#include "vector"

struct Move
{
    uint8_t pieceId;
};

class GameState
{
public:
    static constexpr int N = Piece::N;
    static constexpr int COLUMN_LEN = 12; // number of cells per column
    std::array<Piece, N> myPieces;
    std::array<Piece, N> opPieces;
    int turn;      // 0=Player1, 1=Player2
    uint64_t hash; // Zobrist hash of current state

    GameState();
    void applyMove(const Move &m);
    std::vector<Move> legalMoves() const;
    bool isTerminal() const;
    uint64_t zobristHash() const;
};