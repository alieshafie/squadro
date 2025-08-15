#include "NetworkManager.h"

#include <iostream>
#include <string>

#include "Json.hpp"  // اگر json library داری

SquadroAI::NetworkManager::NetworkManager(const std::string& gui_ip,
                                          int my_player_gui_port,
                                          const std::string& my_listen_ip,
                                          int my_listen_reply_port)
    : m_gui_ip{gui_ip},
      m_my_player_gui_port{my_player_gui_port},
      m_my_listen_ip{my_listen_ip},
      m_my_listen_reply_port{my_listen_reply_port} {}

SquadroAI::NetworkManager::~NetworkManager() {
  stopListening();
  std::cout << "NetworkManager destroyed." << std::endl;
}

bool SquadroAI::NetworkManager::sendMoveToGui(int pawn_to_move_idx) {
  // Make GUI Client if donst made yet
  if (!m_gui_client) {
    m_gui_client =
        std::make_unique<httplib::Client>(m_gui_ip, m_my_player_gui_port);
  }

  // Make Json File
  std::string json = "{\"move\": \"" + std::to_string(pawn_to_move_idx) + "\"}";

  // send Json to GUI
  auto res = m_gui_client->Post("/", json, "application/json");

  // Checking
  if (res && res->status == 200) {
    std::cout << "Move Sucsessfully sent to GUI" << json << std::endl;
    return true;
  } else {
    std::cerr << "Move send Failed..." << std::endl;
    return false;
  }
}

void SquadroAI::NetworkManager::startListeningForOpponentMoves(
    std::function<void(int opponent_pawn_move_idx)> on_opponent_move_received) {
  using json = nlohmann::json;
  if (m_is_listening) {
    std::cerr << "Warning: Server is already running." << std::endl;
    return;
  }

  m_listen_server = std::make_unique<httplib::Server>();

  m_listen_server->Post("/", [this, on_opponent_move_received](
                                 const httplib::Request& req,
                                 httplib::Response& res) {
    try {
      auto j = json::parse(req.body);
      if (j.contains("move")) {
        int move_idx = -1;
        if (j["move"].is_string()) {
          move_idx = std::stoi(j["move"].get<std::string>());
        } else if (j["move"].is_number_integer()) {
          move_idx = j["move"].get<int>();
        } else {
          res.status = 400;
          res.set_content("Bad Request: 'move' must be string or integer",
                          "text/plain");
          return;
        }
        std::cout << "Received move: " << move_idx << std::endl;
        {
          std::lock_guard<std::mutex> lock(m_callback_mutex);
          on_opponent_move_received(move_idx);
        }
        res.status = 200;
        res.set_content("OK", "text/plain");
      } else {
        res.status = 400;
        res.set_content("Bad Request: missing 'move' field", "text/plain");
      }
    } catch (const std::exception& e) {
      res.status = 400;
      res.set_content(std::string("Bad Request: ") + e.what(), "text/plain");
    }
  });

  m_is_listening = true;

  m_server_thread = std::thread([this]() {
    std::cout << "Server started listening on " << m_my_listen_ip << ":"
              << m_my_listen_reply_port << std::endl;
    m_listen_server->listen(m_my_listen_ip.c_str(), m_my_listen_reply_port);
    std::cout << "Server stopped." << std::endl;
  });
}

void SquadroAI::NetworkManager::stopListening() {
  if (!m_is_listening) {
    std::cerr << "Server is not running." << std::endl;
    return;
  }

  std::cout << "Stopping server..." << std::endl;

  if (m_listen_server) {
    m_listen_server->stop();
  }

  if (m_server_thread.joinable()) {
    m_server_thread.join();
  }

  m_is_listening = false;

  std::cout << "Server stopped successfully." << std::endl;
}
