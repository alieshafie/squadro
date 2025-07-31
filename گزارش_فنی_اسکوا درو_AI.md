# گزارش فنی جامع پروژه اسکوادرو AI

## بخش اول: هسته و مغز هوش مصنوعی

### 1. معماری داده‌ها و نمایش بازی

#### ساختار کلی

پروژه اسکوادرو AI با استفاده از چندین کلاس کلیدی برای نمایش وضعیت بازی طراحی شده است:

1. **Constants.h**: تعاریف ثابت‌های کلیدی مانند ابعاد تخته، تعداد مهره‌ها و امتیازات
2. **Piece.h**: نمایش مهره‌های بازی
3. **Move.h**: نمایش حرکات
4. **Board.h**: نمایش تخته بازی
5. **GameState.h**: نمایش وضعیت کلی بازی

#### تحلیل ساختار Piece.h

کلاس `Piece` یک ساختار سبک و کارآمد برای نمایش مهره‌های بازی است:

```cpp
class Piece {
 public:
  PlayerID owner;
  int id;  // شناسه سراسری 0 تا 9

  int row;
  int col;
  PieceStatus status;
  int forward_power;
  int backward_power;
  // ...
};
```

**دلیل انتخاب ساختار سبک**:

- کلاس `Piece` بیشتر شبیه یک `struct` ساده است که برای سرعت عالی است.
- تمام اعضای عمومی و بدون متد پیچیده، دسترسی سریع را فراهم می‌کند.

**استفاده از Bit-Fields**:
در کد فعلی از Bit-Fields استفاده نشده است، اما می‌توان از آنها برای کاهش مصرف حافظه و بهبود عملکرد کش استفاده کرد. به عنوان مثال:

```cpp
class Piece {
  unsigned int owner : 2;     // 2 بیت برای ذخیره PlayerID (0, 1, 2)
  unsigned int id : 4;        // 4 بیت برای شناسه 0-9
  unsigned int row : 3;       // 3 بیت برای سطر 0-6
  unsigned int col : 3;       // 3 بیت برای ستون 0-6
  unsigned int status : 2;    // 2 بیت برای وضعیت
  unsigned int forward_power : 2;  // 2 بیت برای قدرت حرکت به جلو
  unsigned int backward_power : 2; // 2 بیت برای قدرت حرکت به عقب
};
```

این رویکرد می‌تواند اندازه هر مهره را از 28 بایت به 2 بایت کاهش دهد و بهبود قابل توجهی در عملکرد کش ایجاد کند.

#### تحلیل ساختار Move.h

ساختار `Move` برای نمایش حرکات بازی طراحی شده است:

```cpp
struct Move {
  int piece_index;  //(0-4) نسبی
  // ...
};
```

این ساختار ساده و کارآمد:

- فقط یک عدد صحیح برای نمایش حرکت ذخیره می‌کند
- توابع کمکی برای تبدیل بین اندیس نسبی و سراسری دارد

#### تحلیل ساختار Board.h

کلاس `Board` مسئول مدیریت وضعیت فیزیکی تخته بازی است:

```cpp
class Board {
 private:
  // **بهینه‌سازی کلیدی شماره ۱: Data Locality**
  // به جای grid دو بعدی، از یک آرایه یک بعدی استفاده
  // می‌کنیم.
  std::array<Cell, NUM_ROWS * NUM_COLS> grid;

  // **بهینه‌سازی کلیدی شماره ۲: Single Source of Truth**
  std::array<Piece, NUM_PIECES> pieces;
  // ...
};
```

**دلیل استفاده از std::array به جای std::vector**:

- اندازه ثابت در زمان کامپایل
- تخصیص حافظه روی پشته (Stack) که سریع‌تر است
- دسترسی مستقیم به عناصر با عملکرد بهتر

**دلیل استفاده از آرایه یک بعدی به جای دو بعدی**:

- بهبود locality داده‌ها
- کل تخته در یک بلوک حافظه پیوسته قرار می‌گیرد
- دسترسی سریع‌تر به عناصر

#### تحلیل ساختار GameState.h

کلاس `GameState` وضعیت کلی بازی را مدیریت می‌کند:

```cpp
class GameState {
 private:
  Board board;             // وضعیت فیزیکی تخته (سریع و فشرده)
  PlayerID currentPlayer;  // بازیکنی که نوبت اوست
  PlayerID winner;         // برای ذخیره برنده بازی
  int turnCount;           // تعداد نوبت‌های گذشته
  // ...
};
```

**طراحی سبک و قابل کپی**:

- کلاس سبک با اعضای اولیه
- قابل کپی برای استفاده در الگوریتم جستجو
- تمام داده‌های مورد نیاز برای نمایش وضعیت بازی را نگه می‌دارد

### 2. موتور جستجو و منطق تصمیم‌گیری

#### الگوریتم آلفا-بتا

پیاده‌سازی الگوریتم آلفا-بتا در فایل `AIPlayer.cpp` انجام شده است:

```cpp
int AIPlayer::alphaBeta(GameState& state, int depth, int alpha, int beta,
                        bool maximizing_player, Move* best_move) {
  // --- 1. بررسی شرایط توقف ---
  // ...

  // --- 2. جستجو در جدول انتقال ---
  // ...

  // --- 3. تولید و مرتب‌سازی حرکات ---
  // ...

  // --- 4. حلقه اصلی آلفا-بتا ---
  // ...
}
```

**ساختار تابع بازگشتی**:

- پارامترهای `alpha` و `beta` برای هرس کردن
- پارامتر `maximizing_player` برای تعیین نوع بازیکن
- پارامتر `best_move` برای ذخیره بهترین حرکت

**منطق کلیدی هرس کردن**:

```cpp
if (beta <= alpha) {
  // Beta cutoff - this is a good move, save it as a killer
  updateKillerMove(move, depth);
  updateHistoryScore(move, depth);
  break;  // Beta cutoff
}
```

#### جستجوی عمیق‌شونده تکراری (Iterative Deepening)

پیاده‌سازی در تابع `findBestMove`:

```cpp
Move AIPlayer::findBestMove(const GameState& initial_state, int time_limit_ms) {
  // ...
  // Iterative Deepening (IDDFS) Loop
  while (true) {
    transposition_table.clear();  // Clear TT for each new depth
    // ...
    int score = alphaBeta(root_state, max_depth, LOSS_SCORE - 1, WIN_SCORE + 1,
                          true, &root_best_move);
    // ...
    max_depth++;
  }
  // ...
}
```

**ویژگی‌ها**:

- حلقه‌ای که عمق را افزایش می‌دهد
- تایمر برای کنترل زمان ۱۰ ثانیه‌ای
- پاک کردن جدول انتقال در هر عمق جدید

#### جستجوی آرامش (Quiescence Search)

پیاده‌سازی در تابع `quiesce`:

```cpp
int AIPlayer::quiesce(GameState& state, int alpha, int beta, int depth_left) {
  // ...
  if (depth == 0) {
    // At leaf nodes, enter quiescence search to handle tactical positions
    return quiesce(state, alpha, beta, 0);
  }
  // ...
}
```

**ویژگی‌ها**:

- فراخوانی در انتهای جستجوی اصلی (وقتی depth == 0)
- جلوگیری از خطای ارزیابی در موقعیت‌های حساس
- محدودیت عمق برای جلوگیری از انفجار ترکیبی

## بخش دوم: بهینه‌سازی‌های پیشرفته و زیرساخت

### 3. تکنیک‌های افزایش سرعت جستجو

#### جدول جابجایی (Transposition Table) و Zobrist Hashing

**فایل GameStateHasher.cpp**:
پیاده‌سازی هش‌کردن وضعیت بازی با استفاده از Zobrist Hashing:

```cpp
uint64_t GameStateHasher::computeHash(const Board& board,
                                    PlayerID currentPlayer) {
  // ...
  uint64_t hash = 0;

  // Hash the position and status of each piece
  const auto& pieces = board.getAllPieces();
  for (int piece_id = 0; piece_id < NUM_PIECES; ++piece_id) {
    const auto& piece = pieces[piece_id];
    // ...
    hash ^= piece_hashes[piece_id][piece.row][piece.col];
    // ...
  }
  // ...
  return hash;
}
```

**نقش هش‌کردن**:

- تولید کلیدهای هش منحصر به فرد با XOR
- تشخیص وضعیت‌های تکراری در درخت جستجو

**فایل TranspositionTable.cpp**:
پیاده‌سازی جدول جابجایی:

```cpp
bool TranspositionTable::probe(uint64_t key, int depth, int& score,
                               Move& best_move) const {
  // ...
  if (entry.zobrist_key == key && entry.depth >= depth) {
    // ...
    return true;
  }
  // ...
  return false;
}
```

**روند کار**:

- چک کردن جدول قبل از جستجو (Cache Hit)
- ذخیره کردن نتیجه بعد از جستجو (Cache Miss)
- استراتژی جایگزینی مناسب بر اساس عمق

#### ترتیب حرکت پیشرفته (Advanced Move Ordering)

پیاده‌سازی در فایل `AIPlayer.cpp`:

```cpp
void AIPlayer::sortMoves(std::vector<Move>& moves, int ply,
                         const GameState& state) const {
  // ...
  std::sort(moves.begin(), moves.end(),
            [this, ply, &state, tt_move](const Move& a, const Move& b) {
              // TT move always comes first
              // ...
              return getMoveScore(a, ply, state) > getMoveScore(b, ply, state);
            });
}
```

**دلیل اهمیت مرتب‌سازی حرکات**:

- کارایی آلفا-بتا به ترتیب حرکات بسیار حساس است
- حرکات خوب‌تر را زودتر بررسی کند تا هرس بهتری انجام شود

**تکنیک‌های پیاده‌سازی شده**:

1. **استفاده از حرکت موجود در جدول هش**:

   - حرکتی که قبلاً بهترین بوده اول بررسی می‌شود

2. **حرکات کُشنده (Killer Moves)**:

   ```cpp
   void AIPlayer::updateKillerMove(const Move& move, int ply) {
     // ...
   }
   ```

3. **هیوریستیک تاریخچه (History Heuristic)**:
   ```cpp
   void AIPlayer::updateHistoryScore(const Move& move, int depth) {
     // ...
   }
   ```

### 4. بخش شبکه و ارتباطات

#### هدف و پروتکل

**هدف**: ارتباط با GUI و دریافت/ارسال حرکات
**پروتکل**: HTTP/JSON

#### نقش کتابخانه cpp-httplib

کتابخانه `cpp-httplib` برای پیاده‌سازی سرور و کلاینت HTTP استفاده شده است.

#### پیاده‌سازی دوگانه سرور و کلاینت

**فایل NetworkManager.cpp**:

```cpp
bool SquadroAI::NetworkManager::sendMoveToGui(int pawn_to_move_idx) {
  // ...
  auto res = m_gui_client->Post("/move", json, "application/json");
  // ...
}

void SquadroAI::NetworkManager::startListeningForOpponentMoves(
    std::function<void(int opponent_pawn_move_idx)> on_opponent_move_received) {
  // ...
  m_listen_server->Post("/move", [this, on_opponent_move_received](
                                     const httplib::Request& req,
                                     httplib::Response& res) {
    // ...
  });
  // ...
}
```

**ویژگی‌ها**:

- AI هم به عنوان سرور (برای دریافت حرکت حریف با `svr.Post(...)`) پیاده‌سازی شده است
- AI هم به عنوان کلاینت (برای ارسال حرکت خود با `cli.Post(...)`) پیاده‌سازی شده است
- استفاده از callback برای مدیریت حرکات دریافتی

### نتیجه‌گیری

پروژه اسکوادرو AI یک پیاده‌سازی کامل و بهینه‌شده از الگوریتم‌های هوش مصنوعی برای بازی اسکوادرو است. با استفاده از تکنیک‌های پیشرفته مانند جستجوی آلفا-بتا، جدول جابجایی، هش‌کردن Zobrist، و مرتب‌سازی حرکات، عملکرد بسیار خوبی در زمان محدود 10 ثانیه ایجاد کرده است. همچنین بخش شبکه به خوبی پیاده‌سازی شده تا ارتباط مناسب با GUI برقرار کند.
