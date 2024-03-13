// Copyright 2024 Khalil Estell
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <libhal-util/static_list.hpp>
#include <libhal/can.hpp>

namespace hal {
/**
 * @brief Route CAN messages received on the can bus to callbacks based on ID.
 *
 */
class can_router
{
public:
  static constexpr auto noop =
    []([[maybe_unused]] const can::message_t& p_message) {};

  using message_handler = hal::callback<hal::can::handler>;

  struct route
  {
    hal::can::id_t id = 0;
    message_handler handler = noop;
  };

  using route_item = static_list<route>::item;

  /**
   * @brief Construct a new can message router
   *
   * @param p_can - can peripheral to route messages for
   */
  explicit can_router(hal::can& p_can);

  can_router() = delete;
  can_router(can_router& p_other) = delete;
  can_router& operator=(can_router& p_other) = delete;
  can_router& operator=(can_router&& p_other) noexcept;
  can_router(can_router&& p_other) noexcept;
  ~can_router();

  /**
   * @brief Get a reference to the can peripheral driver
   *
   * Used to send can messages through the same port that the can_router is
   * using.
   *
   * @return can& reference to the can peripheral driver
   */
  [[nodiscard]] hal::can& bus();

  /**
   * @brief Add a message route without setting the callback
   *
   * The default callback will do nothing and will drop the message.
   *
   * @param p_id - Associated ID of messages to be stored.
   * @return auto - route item from the linked list that must be stored stored
   * in a variable
   */
  [[nodiscard]] static_list<route>::item add_message_callback(
    hal::can::id_t p_id);

  /**
   * @brief Set a callback for when messages with a specific ID is received
   *
   * @param p_id - Associated ID of messages to be stored.
   * @param p_handler - callback to be executed when a p_id message is received.
   * @return auto - route item from the linked list that must be stored stored
   * in a variable
   */
  [[nodiscard]] static_list<route>::item add_message_callback(
    hal::can::id_t p_id,
    message_handler p_handler);

  /**
   * @brief Get the list of handlers
   *
   * Meant for testing purposes or when direct inspection of the map is useful
   * in userspace. Should not be used in by libraries.
   *
   * @return const auto& map of all of the can message handlers.
   */
  [[nodiscard]] const static_list<route>& handlers();

  /**
   * @brief Message routing interrupt service handler
   *
   * Searches the static list and finds the first ID associated with the message
   * and run's that route's callback.
   *
   * @param p_message - message received from the bus
   */
  void operator()(const can::message_t& p_message);

private:
  static_list<route> m_handlers{};
  hal::can* m_can = nullptr;
};
}  // namespace hal
