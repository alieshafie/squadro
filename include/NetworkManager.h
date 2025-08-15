#pragma once

#include <httplib.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "Constants.h"
#include "Move.h"

namespace SquadroAI {

enum class NetworkStatus { OK = 200, BAD_REQUEST = 400, SERVER_ERROR = 500 };

class NetworkManager {
 public:
  NetworkManager(const std::string& gui_ip, int my_player_gui_port,
                 const std::string& my_listen_ip, int my_listen_reply_port);
  ~NetworkManager();

  NetworkManager(const NetworkManager&) = delete;
  NetworkManager& operator=(const NetworkManager&) = delete;

  bool sendMoveToGui(int pawn_to_move_idx);
  void startListeningForOpponentMoves(
      std::function<void(int)> on_opponent_move_received);
  void stopListening();
  bool isListening() const { return m_is_listening; }

 private:
  static constexpr int TIMEOUT_SECONDS = 30;
  static constexpr int MAX_RETRY_COUNT = 3;

  std::string m_gui_ip;
  int m_my_player_gui_port;
  std::string m_my_listen_ip;
  int m_my_listen_reply_port;

  std::unique_ptr<httplib::Client> m_gui_client;
  std::unique_ptr<httplib::Server> m_listen_server;
  std::thread m_server_thread;
  std::atomic<bool> m_is_listening{false};
  std::atomic<bool> m_shutdown_requested{false};

  std::mutex m_server_mutex;
  mutable std::mutex
      m_callback_mutex;  // To protect the on_opponent_move_received callback
  std::condition_variable m_server_cv;

  void initializeServer();
  void setupServerRoutes(std::function<void(int)> callback);
  void serverLoop();
  void cleanupServer();

  bool validatePawnIndex(int index) const;
  void logNetworkError(const std::string& message, NetworkStatus status) const;

  bool retryOperation(std::function<bool()> operation,
                      int maxRetries = MAX_RETRY_COUNT);
};

}  // namespace SquadroAI
