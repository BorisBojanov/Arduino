#pragma once

#include <Arduino.h>
#include <WebSocketsServer.h>

namespace WebSocket
{

  /// Initialize the websocket instance
  void initialize();

  /// Handle websocket
  void handle();

  /// Broadcast the current animation status to all active websocket connections
  void broadcastStatus();

} // namespace Websocket
