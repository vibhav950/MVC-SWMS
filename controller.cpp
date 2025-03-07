#include "controller.hpp"
#include "log.hpp"
#include "model.hpp"

#include <thread>

extern Log::Logger logger;

using namespace Controller;

void CController::configure() {
  using namespace Model;

  CROW_ROUTE(app, "/api/wastepickups")
      .methods(crow::HTTPMethod::Post)([](const crow::request& req) {
        logger.log("Recieved:\n", req.body);
        try {
          auto json = crow::json::load(req.body);
          if (!json)
            return crow::response(400);
          WastePickup pickup(json["wasteType"].s(), json["pickupLocation"].s(),
                             json["pickupDateTime"].s(), json["userName"].s());
          WastePickup::addRequest(pickup);
          return crow::response(201);
        } catch (const std::exception &e) {
          logger.log(e.what());
          return crow::response(400);
        }
      });

  CROW_ROUTE(app, "/api/wastepickups/<int>")
      .methods(crow::HTTPMethod::Get)([](int id) {
        logger.log("Recieved:\n", id);

        WastePickup pickup = WastePickup::getRequestById(id);
        crow::json::wvalue res;

        res["id"] = pickup.getId();
        res["wasteType"] = pickup.getWasteType();
        res["pickupLocation"] = pickup.getLocation();
        res["pickupDateTime"] = pickup.getDateTime();
        res["status"] = pickup.getStatus();
        res["userName"] = pickup.getUser();

        return crow::response(res);
      });

  CROW_ROUTE(app, "/api/wastepickups").methods(crow::HTTPMethod::Get)([]() {
    auto pickups = WastePickup::getRequests();
    crow::json::wvalue res;
    int i = 0;
    for (const auto &pickup : pickups) {
      res[i]["id"] = pickup.getId();
      res[i]["wasteType"] = pickup.getWasteType();
      res[i]["pickupLocation"] = pickup.getLocation();
      res[i]["pickupDateTime"] = pickup.getDateTime();
      res[i]["status"] = pickup.getStatus();
      res[i]["userName"] = pickup.getUser();
      i++;
    }
    return crow::response(res);
  });

  CROW_ROUTE(app, "/api/wastepickups/<int>")
      .methods(crow::HTTPMethod::Put)([](const crow::request &req, int id) {
        logger.log("Recieved:\n", req.body, id);
        try {
          auto json = crow::json::load(req.body);
          if (!json)
            return crow::response(400);
          bool success = WastePickup::updateRequest(
              id, json["pickupDateTime"].s(), json["status"].s());
          return success ? crow::response(200) : crow::response(400);
        } catch (const std::exception &e) {
          logger.log(e.what());
          return crow::response(400);
        }
      });

  CROW_ROUTE(app, "/api/wastepickups/<int>")
      .methods(crow::HTTPMethod::Delete)([](int id) {
        logger.log("Recieved:\n", id);
        bool success = WastePickup::deleteRequest(id);
        return success ? crow::response(200) : crow::response(400);
      });
}

Log::Logger logger(std::cout);

void cleanup() {
  Model::WastePickup::closeDatabase();
}

int main() {
  std::atexit(cleanup);

  // init the db
  Model::WastePickup::initializeDatabase();

  // spawn the update thread
  std::thread updateThread(Model::WastePickup::dbUpdateThread, nullptr);

  // start the server
  CController controller;
  controller.run(18080);

  return 0;
}
