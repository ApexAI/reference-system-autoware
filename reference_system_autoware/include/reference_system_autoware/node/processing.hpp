#pragma once

#include <chrono>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "reference_system_autoware/number_cruncher.hpp"
#include "reference_system_autoware/types.hpp"

namespace node {
struct ProcessingSettings {
  std::string node_name;
  std::string input_topic;
  std::string output_topic;
  std::chrono::nanoseconds number_crunch_time;
};

class Processing : public rclcpp::Node {
 public:
  Processing(const ProcessingSettings& settings)
      : Node(settings.node_name),
        number_crunch_time_(settings.number_crunch_time) {
    subscription_ = this->create_subscription<message_t>(
        settings.input_topic, 10,
        [this](const message_t::SharedPtr msg) { input_callback(msg); });
    publisher_ = this->create_publisher<message_t>(settings.output_topic, 10);
  }

 private:
  void input_callback(const message_t::SharedPtr input_message) const {
    number_cruncher(number_crunch_time_);

    auto output_message = publisher_->borrow_loaned_message();
    int64_t timestamp_in_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count();

    int64_t accumulated_latency_in_ns =
        input_message->data[0] + timestamp_in_ns - input_message->data[1];

    output_message.get().data[0] = accumulated_latency_in_ns;
    output_message.get().data[1] = timestamp_in_ns;
    publisher_->publish(std::move(output_message));
  }

 private:
  publisher_t publisher_;
  subscription_t subscription_;
  std::chrono::nanoseconds number_crunch_time_;
};
}  // namespace node
