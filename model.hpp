#pragma once

#include <iostream>
#include <sqlite3.h>
#include <string>
#include <vector>
#include <unordered_map>

#define DB_UPDATE_THREAD_INTERVAL 10 /* seconds */

namespace Model {
class WastePickup {
private:
  int id;
  std::string wasteType;
  std::string pickupLocation;
  std::string pickupDateTime; // Format: "YYYY-MM-DD HH:MM:SS"
  std::string status;
  std::string userName;

public:
  WastePickup(std::string type, std::string location, std::string dateTime,
              std::string user, std::string stat = "Pending")
      : wasteType(type), pickupLocation(location), pickupDateTime(dateTime),
        userName(user), status(stat) {}

  int getId() const { return id; }
  std::string getWasteType() const { return wasteType; }
  std::string getLocation() const { return pickupLocation; }
  std::string getDateTime() const { return pickupDateTime; }
  std::string getUser() const { return userName; }
  std::string getStatus() const { return status; }

  static void displayRequest(const WastePickup& req);

  // Database methods
  static void initializeDatabase(const std::string& dbPath = "");
  static void closeDatabase();

  static bool addRequest(const WastePickup& request);
  static std::vector<WastePickup> getRequests(const std::string& filter = "");
  static WastePickup getRequestById(int id);
  static bool updateRequest(int id, const std::string& newDateTime,
                            const std::string& newStatus);
  static bool deleteRequest(int id);

  static std::unordered_map<int, std::string> getPendingRequests();

  static void *dbUpdateThread(void *arg);
};

} // namespace Model