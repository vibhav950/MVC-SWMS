#include "model.hpp"
#include "log.hpp"

#include <pthread.h>
#include <unistd.h>
#include <limits>

sqlite3 *db;
pthread_mutex_t dbMutex = PTHREAD_MUTEX_INITIALIZER;

extern Log::Logger logger;

using namespace Model;

void WastePickup::initializeDatabase(const std::string& dbPath) {
  const char *path = dbPath.empty() ? "waste_pickup.db" : dbPath.c_str();

  pthread_mutex_lock(&dbMutex);

  if (sqlite3_open(path, &db)) {
    logger.log("Error: can't open database\n");
    pthread_mutex_unlock(&dbMutex);
    return;
  }

  std::string sql = "CREATE TABLE IF NOT EXISTS WastePickup ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "wasteType TEXT,"
                    "pickupLocation TEXT,"
                    "pickupDateTime TEXT,"
                    "status TEXT,"
                    "userName TEXT);";

  char *errMsg;
  if (sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg)) {
    logger.log("crror Can't create table: ", errMsg, "\n");
    sqlite3_free(errMsg);
  }

  pthread_mutex_unlock(&dbMutex);
  return;
}

void WastePickup::closeDatabase() {
  pthread_mutex_lock(&dbMutex);
  sqlite3_close(db);
  pthread_mutex_unlock(&dbMutex);
}

// todo: remove this expensive copy
bool WastePickup::addRequest(const WastePickup& request) {
  bool ret = true;

  std::string sql = "INSERT INTO WastePickup (wasteType, pickupLocation, "
                    "pickupDateTime, status, userName) VALUES ('" +
                    request.getWasteType() + "', '" + request.getLocation() +
                    "', '" + request.getDateTime() + "', '" +
                    request.getStatus() + "', '" + request.getUser() + "');";
  char *errMsg;

  pthread_mutex_lock(&dbMutex);
  if (sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
    logger.log("Error: can't insert data into table: ", errMsg, "\n");
    sqlite3_free(errMsg);
    ret = false;
    goto cleanup;
  }

cleanup:
  pthread_mutex_unlock(&dbMutex);
  return ret;
}

bool WastePickup::updateRequest(int id, const std::string& newDateTime,
                                const std::string& newStatus) {
  bool ret = true;

  std::string sql = "UPDATE WastePickup SET pickupDateTime = '" + newDateTime +
                    "', status = '" + newStatus +
                    "' WHERE id = " + std::to_string(id) + ";";
  char *errMsg;

  pthread_mutex_lock(&dbMutex);
  if (sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
    logger.log("Error: can't update data in table: ", errMsg, "\n");
    sqlite3_free(errMsg);
    ret = false;
    goto cleanup;
  }

cleanup:
  pthread_mutex_unlock(&dbMutex);
  return ret;
}

bool WastePickup::deleteRequest(int id) {
  bool ret = true;

  std::string sql =
      "DELETE FROM WastePickup WHERE id = " + std::to_string(id) + ";";
  char *errMsg;

  pthread_mutex_lock(&dbMutex);
  if (sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
    logger.log("Error: can't delete data from table: ", errMsg, "\n");
    sqlite3_free(errMsg);
    ret = false;
    goto cleanup;
  }

cleanup:
  pthread_mutex_unlock(&dbMutex);
  return ret;
}

std::vector<WastePickup> WastePickup::getRequests(const std::string& filter) {
  std::vector<WastePickup> requests;
  std::string sql = "SELECT * FROM WastePickup " +
                    (filter.empty() ? "" : "WHERE " + filter) + ";";

  pthread_mutex_lock(&dbMutex);

  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      WastePickup req(
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)),
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2)),
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3)),
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5)),
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4)));

      // Set the ID from the database
      req.id = sqlite3_column_int(stmt, 0);

      requests.push_back(req);
    }
    sqlite3_finalize(stmt);
  }

  // return empty object if there were no objects matched
  if (requests.empty()) {
    logger.log("No records matched\n");
    WastePickup req("", "", "", "", "");
    req.id = std::numeric_limits<int>::max();
    requests.push_back(std::move(req));
  }

  pthread_mutex_unlock(&dbMutex);
  return requests;
}

WastePickup WastePickup::getRequestById(int id) {
  std::string sql =
      "SELECT * FROM WastePickup WHERE id = " + std::to_string(id) + ";";

  sqlite3_stmt *stmt;

  pthread_mutex_lock(&dbMutex);

  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      WastePickup req(
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)),
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2)),
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3)),
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5)),
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4)));

      // Set the ID from the database
      req.id = sqlite3_column_int(stmt, 0);

      sqlite3_finalize(stmt);
      pthread_mutex_unlock(&dbMutex);
      return req;
    }
    sqlite3_finalize(stmt);
  }

  pthread_mutex_unlock(&dbMutex);

  // return empty object if no record was found
  WastePickup wp = WastePickup("", "", "", "");
  wp.id = std::numeric_limits<int>::max();
  return std::move(wp);
}

void WastePickup::displayRequest(const WastePickup& req) {
  std::cout << "ID: " << std::to_string(req.getId()) << std::endl;
  std::cout << "Type: " << req.getWasteType() << std::endl;
  std::cout << "Location: " << req.getLocation() << std::endl;
  std::cout << "Date and Time: " << req.getDateTime() << std::endl;
  std::cout << "User: " << req.getUser() << std::endl;
  std::cout << "Status: " << req.getStatus() << std::endl;
  std::cout << std::endl;
}

std::unordered_map<int, std::string> WastePickup::getPendingRequests() {
  std::unordered_map<int, std::string> requests;
  std::string sql =
      "SELECT id, pickupDateTime FROM WastePickup WHERE status = 'Pending';";

  pthread_mutex_lock(&dbMutex);

  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      requests[sqlite3_column_int(stmt, 0)] =
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
    }
    sqlite3_finalize(stmt);
  }

  pthread_mutex_unlock(&dbMutex);
  return std::move(requests);
}

// thread procedure to periodically update the statuses of pending requests
void *WastePickup::dbUpdateThread(void *arg) {
  while (true) {
    std::cout << "Updating pending requests...\n";

    std::string sql =
        "UPDATE WastePickup SET status = 'Completed' WHERE "
        "status = 'Pending' AND datetime(pickupDateTime) < datetime('now', 'localtime');";
    char *errMsg;

    pthread_mutex_lock(&dbMutex);
    if (sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
      logger.log("Error: can't update data in table: ", errMsg, "\n");
      sqlite3_free(errMsg);
    }
    pthread_mutex_unlock(&dbMutex);

    sleep(DB_UPDATE_THREAD_INTERVAL);
  }
  return NULL;
}
