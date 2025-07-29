#pragma once

#include <string>
#include <functional>
#include <thread>
#include <memory> // برای std::unique_ptr
#include <httplib.h>
#include "Constants.h"
#include "Move.h" // اگر لازم باشد اطلاعات بیشتری از Move ارسال شود

namespace SquadroAI
{

    class NetworkManager
    {
    public:
        NetworkManager(const std::string& gui_ip, int my_player_gui_port,
            const std::string& my_listen_ip, int my_listen_reply_port);
        ~NetworkManager();

        // برای ارسال حرکت به GUI
        // ورودی: شماره مهره‌ای که باید حرکت داده شود (0-4)
        bool sendMoveToGui(int pawn_to_move_idx);

        // برای شروع سرور جهت دریافت حرکت حریف از GUI
        // callback زمانی فراخوانی می‌شود که حرکتی از حریف دریافت شود
        // ورودی callback: شماره مهره حرکت داده شده توسط حریف (0-4)
        void startListeningForOpponentMoves(std::function<void(int opponent_pawn_move_idx)> on_opponent_move_received);

        void stopListening();

    private:
        std::string m_gui_ip;
        int m_my_player_gui_port; // پورتی که GUI برای حرکات *این* بازیکن گوش می‌دهد (مثلاً player1_port)

        std::string m_my_listen_ip; // IP این عامل که GUI حرکات حریف را به آن ارسال می‌کند
        int m_my_listen_reply_port; // پورتی که این عامل برای دریافت حرکات حریف گوش می‌دهد (مثلاً player1_reply_port)

        std::unique_ptr<httplib::Client> m_gui_client;
        std::unique_ptr<httplib::Server> m_listen_server;
        std::thread m_server_thread;
        bool m_is_listening = false;

        // توابع داخلی برای پردازش درخواست‌ها
        void handleGuiPost(const httplib::Request& req, httplib::Response& res,
            std::function<void(int)> on_opponent_move_received_callback);
    };

} // namespace SquadroAI