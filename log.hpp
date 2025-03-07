#pragma once

#include <iostream>

namespace Log {
class Logger {
private:
  std::ostream &os;

public:
  Logger(std::ostream &os) : os(os) {}

  template <typename T> void log(T message) { os << message << std::endl; }

  template <typename T, typename... Args> void log(T message, Args... args) {
    os << message << " ";
    log(args...);
  }
};
} // namespace Log