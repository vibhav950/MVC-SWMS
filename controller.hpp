#pragma once

#include "crow_all.h"
#include "model.hpp"

namespace Controller {

class CController {
private:
  crow::SimpleApp app;

  void configure();

public:
  CController() { configure(); }

  void run(unsigned short __port) { app.port(__port).multithreaded().run(); }

  void stop() { app.stop(); }
};
} // namespace Controller