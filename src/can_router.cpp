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

#include "libhal-canrouter/can_router.hpp"

#include <libhal-util/can.hpp>
#include <libhal-util/comparison.hpp>

namespace hal {
/**
 * @brief Construct a new can message router
 *
 * @param p_can - can peripheral to route messages for
 */
can_router::can_router(hal::can& p_can)
  : m_can(&p_can)
{
  m_can->on_receive(std::ref((*this)));
}

can_router& can_router::operator=(can_router&& p_other) noexcept
{
  m_handlers = std::move(p_other.m_handlers);
  m_can = p_other.m_can;
  m_can->on_receive(std::ref(*this));

  p_other.m_can = nullptr;
  return *this;
}

can_router::can_router(can_router&& p_other) noexcept
{
  *this = std::move(p_other);
}

can_router::~can_router()
{
  if (m_can) {
    // Assume that if this succeeded in the create factory function, that it
    // will work this time
    m_can->on_receive(noop);
  }
}

/**
 * @brief Get a reference to the can peripheral driver
 *
 * Used to send can messages through the same port that the can_router is
 * using.
 *
 * @return can& reference to the can peripheral driver
 */
hal::can& can_router::bus()
{
  return *m_can;
}

/**
 * @brief Add a message route without setting the callback
 *
 * The default callback will do nothing and will drop the message.
 *
 * @param p_id - Associated ID of messages to be stored.
 * @return auto - route item from the linked list that must be stored stored
 * in a variable
 */
static_list<can_router::route>::item can_router::add_message_callback(
  hal::can::id_t p_id)
{
  return m_handlers.push_back(route{
    .id = p_id,
  });
}

/**
 * @brief Set a callback for when messages with a specific ID is received
 *
 * @param p_id - Associated ID of messages to be stored.
 * @param p_handler - callback to be executed when a p_id message is received.
 * @return auto - route item from the linked list that must be stored stored
 * in a variable
 */
static_list<can_router::route>::item can_router::add_message_callback(
  hal::can::id_t p_id,
  message_handler p_handler)
{
  return m_handlers.push_back(route{
    .id = p_id,
    .handler = std::move(p_handler),
  });
}

/**
 * @brief Get the list of handlers
 *
 * Meant for testing purposes or when direct inspection of the map is useful
 * in userspace. Should not be used in by libraries.
 *
 * @return const auto& map of all of the can message handlers.
 */
const static_list<can_router::route>& can_router::handlers()
{
  return m_handlers;
}

/**
 * @brief Message routing interrupt service handler
 *
 * Searches the static list and finds the first ID associated with the message
 * and run's that route's callback.
 *
 * @param p_message - message received from the bus
 */
void can_router::operator()(const can::message_t& p_message)
{
  for (auto& list_handler : m_handlers) {
    if (p_message.id == list_handler.id) {
      list_handler.handler(p_message);
      return;
    }
  }
}
}  // namespace hal
