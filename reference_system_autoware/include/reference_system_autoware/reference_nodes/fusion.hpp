// Copyright 2021 Apex.AI, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <chrono>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "reference_system_autoware/node_settings.hpp"
#include "reference_system_autoware/number_cruncher.hpp"
#include "reference_system_autoware/sample_management.hpp"
#include "reference_system_autoware/types.hpp"

namespace reference_nodes {
class Fusion : public rclcpp::Node {
 public:
  Fusion(const FusionSettings& settings)
      : Node(settings.node_name),
        number_crunch_time_(settings.number_crunch_time) {
    subscription_[0] = this->create_subscription<message_t>(
        settings.input_0, 10,
        [this](const message_t::SharedPtr msg) { input_callback(0U, msg); });

    subscription_[1] = this->create_subscription<message_t>(
        settings.input_1, 10,
        [this](const message_t::SharedPtr msg) { input_callback(1U, msg); });
    publisher_ = this->create_publisher<message_t>(settings.output_topic, 10);
  }

 private:
  void input_callback(const uint64_t input_number,
                      const message_t::SharedPtr input_message) {
    message_cache_[input_number] = input_message;

    // only process and publish when we can perform an actual fusion, this means
    // we have received a sample from each subscription
    if (!message_cache_[0] || !message_cache_[1]) {
      return;
    }

    auto number_cruncher_result = number_cruncher(number_crunch_time_);

    auto output_message = publisher_->borrow_loaned_message();
    fuse_samples(this->get_name(), output_message.get(), message_cache_[0],
                 message_cache_[1]);
    output_message.get().data[0] = number_cruncher_result.empty();
    publisher_->publish(std::move(output_message));

    message_cache_[0].reset();
    message_cache_[1].reset();
  }

 private:
  message_t::SharedPtr message_cache_[2];
  publisher_t publisher_;
  subscription_t subscription_[2];

  std::chrono::nanoseconds number_crunch_time_;
};
}  // namespace reference_nodes