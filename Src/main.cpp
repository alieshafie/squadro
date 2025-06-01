#include <iostream>
#include <string>
#include <stdexcept> // برای std::runtime_error
#include <vector>    // برای argc, argv
#include <thread>    // برای std::this_thread::sleep_for (اگر لازم شود)
#include <condition_variable>
#include <mutex>
#include <chrono>

#include "Constants.h"       // <--- اصلاح شد
#include "GameState.h"       // <--- اصلاح شد
#include "AIPlayer.h"        // <--- اصلاح شد
#include "NetworkManager.h"  // <--- اصلاح شد
#include "Move.h"            // <--- اصلاح شد (برای NULL_MOVE)
#include "GameStateHasher.h" // برای Zobrist::init() در نسخه شما، اگرچه Zobrist در هدر جدا بود

// --- متغیرهای سراسری برای همگام‌سازی با نخ شبکه ---
std::mutex network_mutex;
std::condition_variable cv_opponent_move;
bool g_opponent_has_moved = false;
int g_last_opponent_pawn_move = -1;
// --- --- --- --- --- --- --- --- --- --- --- --- ---

void opponent_move_callback(int opponent_pawn_move)
{
    std::lock_guard<std::mutex> lock(network_mutex);
    g_last_opponent_pawn_move = opponent_pawn_move;
    g_opponent_has_moved = true;
    std::cout << "[Callback] Opponent moved pawn: " << opponent_pawn_move << std::endl;
    cv_opponent_move.notify_one();
}

int main(int argc, char *argv[])
{
    std::cout << "Squadro AI Agent starting..." << std::endl;

    // مقداردهی اولیه Zobrist (اگر از فایل Zobrist.hpp که قبلا داشتید استفاده می‌کنید)
    // GameStateHasher::init_zobrist_keys(); // یا هر تابعی که برای این کار دارید
    // اگر از GameStateHasher من استفاده می‌کنید، سازنده آن این کار را انجام می‌دهد یا تابع init جدا دارد

    if (argc < 7)
    { // argv[0] is program name, then 6 arguments
        std::cerr << "Usage: " << argv[0] << " <my_player_num (1 or 2)> <gui_ip> <p1_send_port> <p1_reply_port> <p2_send_port> <p2_reply_port>" << std::endl;
        std::cerr << "Example: " << argv[0] << " 1 127.0.0.1 8081 9081 8082 9082" << std::endl;
        return 1;
    }

    int my_player_num;
    std::string gui_ip;
    int p1_send_port, p1_reply_port, p2_send_port, p2_reply_port;

    try
    {
        my_player_num = std::stoi(argv[1]);
        gui_ip = argv[2]; // std::string می‌تونه char* رو مستقیم بگیره
        p1_send_port = std::stoi(argv[3]);
        p1_reply_port = std::stoi(argv[4]);
        p2_send_port = std::stoi(argv[5]);
        p2_reply_port = std::stoi(argv[6]);
    }
    catch (const std::invalid_argument &ia)
    {
        std::cerr << "Invalid argument: " << ia.what() << " - Ensure ports and player number are integers." << std::endl;
        return 1;
    }
    catch (const std::out_of_range &oor)
    {
        std::cerr << "Out of Range error: " << oor.what() << " - Port number likely too large." << std::endl;
        return 1;
    }

    PlayerID my_ai_player_id = (my_player_num == 1) ? PlayerID::PLAYER_1 : PlayerID::PLAYER_2;
    PlayerID opponent_id = (my_player_num == 1) ? PlayerID::PLAYER_2 : PlayerID::PLAYER_1;

    std::cout << "I am Player " << my_player_num << " (" << static_cast<int>(my_ai_player_id) << ")" << std::endl;
    std::cout << "GUI IP: " << gui_ip << std::endl;
    std::cout << "P1 Send: " << p1_send_port << ", P1 Reply: " << p1_reply_port << std::endl;
    std::cout << "P2 Send: " << p2_send_port << ", P2 Reply: " << p2_reply_port << std::endl;

    try
    {
        GameState current_game_state;
        AIPlayer ai_player(my_ai_player_id, 64);
        NetworkManager network_manager(gui_ip, my_player_num, p1_send_port, p1_reply_port, p2_send_port, p2_reply_port);

        network_manager.start_listening_for_opponent_moves(opponent_move_callback);
        std::cout << "Listening for opponent moves..." << std::endl;

        // تعیین نوبت اولیه بر اساس شماره بازیکن
        // فرض: بازیکن ۱ همیشه شروع می‌کند. GUI باید این را مدیریت کند، اما ما هم یک پیش‌فرض می‌گذاریم.
        if (my_player_num == 1)
        {
            current_game_state.current_player_to_move = PlayerID::PLAYER_1;
            std::cout << "Player 1, I start." << std::endl;
        }
        else
        {                                                                   // Player 2
            current_game_state.current_player_to_move = PlayerID::PLAYER_1; // منتظر حرکت بازیکن ۱
            std::cout << "Player 2, waiting for Player 1 to start." << std::endl;
        }

        while (!current_game_state.is_game_over())
        {
            current_game_state.print_state();

            if (current_game_state.current_player_to_move == my_ai_player_id)
            {
                std::cout << "My turn (Player " << my_player_num << ")... Thinking..." << std::endl;
                // زمان تصمیم‌گیری حدود ۲۸-۲۹ ثانیه برای ایجاد حاشیه امن
                Move best_move = ai_player.find_best_move(current_game_state, std::chrono::seconds(29));

                if (best_move.piece_index != NULL_MOVE.piece_index)
                {
                    std::cout << "AI chose to move pawn index: " << best_move.piece_index << std::endl;
                    if (network_manager.send_move_to_gui(best_move.piece_index))
                    {
                        // فقط پس از ارسال موفق، حرکت را در وضعیت داخلی اعمال کن
                        if (!current_game_state.apply_move(best_move))
                        {
                            std::cerr << "CRITICAL: AI generated a move that was invalid locally after sending! " << best_move.to_string() << std::endl;
                            // این نباید اتفاق بیفتد اگر generate_legal_moves و apply_move هماهنگ باشند
                        }
                        std::cout << "Move sent successfully." << std::endl;
                    }
                    else
                    {
                        std::cerr << "Failed to send move to GUI. Exiting." << std::endl;
                        break;
                    }
                }
                else
                {
                    std::cerr << "AI could not find a valid move. This might be a bug or a lost game state. Exiting." << std::endl;
                    break;
                }
            }
            else
            { // Opponent's turn
                std::cout << "Opponent's turn (Player "
                          << (current_game_state.current_player_to_move == PlayerID::PLAYER_1 ? "1" : "2")
                          << ")... Waiting for move via network." << std::endl;

                std::unique_lock<std::mutex> lock(network_mutex);
                // منتظر بمان تا حریف حرکت کند یا زمان تمام شود (مثلاً ۶۰ ثانیه برای اطمینان)
                if (!cv_opponent_move.wait_for(lock, std::chrono::seconds(60), []
                                               { return g_opponent_has_moved; }))
                {
                    std::cerr << "Timeout waiting for opponent's move. Assuming opponent disconnected or issue with GUI. Exiting." << std::endl;
                    // اینجا می‌تونیم یک حرکت "پاس" یا اعلام برد به خاطر عدم فعالیت حریف رو در نظر بگیریم، اگر قوانین اجازه بده
                    break;
                }

                // g_opponent_has_moved باید true باشد
                Move opponent_move = {g_last_opponent_pawn_move}; // piece_index از ۰ تا ۴ است
                std::cout << "Opponent move received: Pawn index " << opponent_move.piece_index << std::endl;

                if (!current_game_state.apply_move(opponent_move))
                {
                    std::cerr << "CRITICAL: Opponent's move " << opponent_move.to_string() << " is invalid for the current local state. "
                              << "State desynchronization likely. Exiting." << std::endl;
                    // اینجا بهتر است برنامه خاتمه یابد چون وضعیت ما با GUI دیگر همگام نیست
                    break;
                }
                std::cout << "Opponent's move applied to local state." << std::endl;
                g_opponent_has_moved = false; // ریست کردن پرچم برای حرکت بعدی حریف
                lock.unlock();                // آزاد کردن قفل در اسرع وقت
            }
        }

        std::cout << "Game Over!" << std::endl;
        current_game_state.print_state(); // نمایش وضعیت نهایی
        PlayerID winner = current_game_state.get_winner();
        if (winner == my_ai_player_id)
        {
            std::cout << "I WON! :D" << std::endl;
        }
        else if (winner == opponent_id)
        {
            std::cout << "Opponent won. :(" << std::endl;
        }
        else if (winner == PlayerID::DRAW)
        {
            std::cout << "It's a DRAW!" << std::endl;
        }
        else
        { // PlayerID::NONE
            std::cout << "Game ended, but winner unclear (or game exited prematurely)." << std::endl;
        }

        network_manager.stop_listening();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Unhandled Exception: " << e.what() << std::endl;
        // اطمینان از توقف نخ شبکه اگر در حال اجرا بود
        // if (network_manager_is_initialized_and_listening) network_manager.stop_listening();
        return 1;
    }
    catch (...)
    {
        std::cerr << "Unknown unhandled exception." << std::endl;
        // if (network_manager_is_initialized_and_listening) network_manager.stop_listening();
        return 1;
    }

    std::cout << "Squadro AI Agent shutting down." << std::endl;
    return 0;
}