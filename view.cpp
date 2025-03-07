#include "view.hpp"
#include "log.hpp"

#include <chrono>
#include <cpr/cpr.h>
#include <ctime>
#include <format>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <limits>
#include <string>

using namespace View;

void CView::displayMenu() {
  std::cout << "1. Create pickup request\n"
            << "2. View pickup request by ID\n"
            << "3. View all pickup requests\n"
            << "4. Update pickup request\n"
            << "5. Delete pickup request\n"
            << "6. Display help\n"
            << "7. Exit\n\n";
}

static void getDateTime(std::string& dateTime) {
  using namespace std::chrono;
  std::string dt;

  std::getline(std::cin, dt);
  std::tm tm = {};
  tm.tm_sec = 0;

  std::istringstream ss(dt);
  ss >> std::get_time(&tm, "%Y-%m-%d %H:%M");

  if (ss.fail()) {
    std::cout << "\033[31m[ERROR]\033[0m Invalid datetime format!\n";
    return;
  }

  auto timePt = system_clock::from_time_t(std::mktime(&tm));
  std::time_t t = system_clock::to_time_t(timePt);

  // finally convert to string
  std::ostringstream op;
  op << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");
  dateTime = op.str();
}

void CView::createPickupRequest() {
  std::string wasteType, location, dateTime, userName;

  std::cout << "Enter waste type (Plastic, Electronic, Organic, Hazardous): ";
  std::getline(std::cin, wasteType);

  std::cout << "Enter pickup location: ";
  std::getline(std::cin, location);

  std::cout << "Enter pickup date and time (YYYY-MM-DD HH:MM): ";
  getDateTime(dateTime);

  std::cout << "Enter user name: ";
  std::getline(std::cin, userName);

  std::string jsonPayload = "{\"wasteType\":\"" + wasteType +
                            "\", \"pickupLocation\":\"" + location +
                            "\", \"pickupDateTime\":\"" + dateTime +
                            "\", \"userName\":\"" + userName + "\"}";

  auto response = cpr::Post(cpr::Url{"http://localhost:18080/api/wastepickups"},
                            cpr::Header{{"Content-Type", "application/json"}},
                            cpr::Body{jsonPayload});

  if (response.status_code = 201)
    std::cout << "\033[32mPickup request created successfully\033[0m\n";
  else
    std::cout << "\033[31mFailed to create pickup request\033[0m\n";
}

void CView::viewPickupRequestById() {
  int id;
  std::cout << "Enter pickup request ID: ";
  std::cin >> id;
  std::cin.ignore();

  auto response = cpr::Get(cpr::Url{"http://localhost:18080/api/wastepickups/" +
                                    std::to_string(id)});

  if (response.status_code == 200) {
    std::cout << "\033[32mPickup request found\033[0m\n";

    nlohmann::json jsonObj = nlohmann::json::parse(response.text);

    std::cout << jsonObj.dump(4) << std::endl;
  } else {
    std::cout << "\033[31mPickup request not found\033[0m\n";
  }
}

void CView::viewAllPickupRequests() {
  auto response = cpr::Get(cpr::Url{"http://localhost:18080/api/wastepickups"});

  if (response.status_code == 200) {
    std::cout << "\033[32mPickup requests found\033[0m\n";

    nlohmann::json jsonObj = nlohmann::json::parse(response.text);

    std::cout << jsonObj.dump(4) << std::endl;
  } else {
    std::cout << "\033[31mPickup requests not found\033[0m\n";
  }
}

void CView::updatePickupRequest() {
  int id;

  std::cout << "Enter pickup request ID: ";
  std::string dateTime, status;
  std::cin >> id;
  std::cin.ignore();
  std::cout << "Enter new date and time (YYYY-MM-DD HH:MM): ";
  getDateTime(dateTime);
  std::cout << "Enter new status: ";
  std::getline(std::cin, status);

  auto response = cpr::Put(
      cpr::Url{"http://localhost:18080/api/wastepickups/" + std::to_string(id)},
      cpr::Body{"{\"pickupDateTime\": \"" + dateTime + "\", \"status\": \"" +
                status + "\"}"},
      cpr::Header{{"Content-Type", "application/json"}});

  if (response.status_code == 200) {
    std::cout << "\033[32mPickup request updated successfully.\033[0m\n";
  } else {
    std::cout << "\033[31mFailed to update pickup request. Status code: "
              << response.status_code << "\033[0m\n";
  }
}

void CView::deletePickupRequest() {
  int id;
  std::cout << "Enter Pickup Request ID to delete: ";
  std::cin >> id;

  auto response = cpr::Delete(cpr::Url{
      "http://localhost:18080/api/wastepickups/" + std::to_string(id)});

  if (response.status_code == 200) {
    std::cout << "\033[32mPickup request deleted successfully.\033[0m\n";
  } else {
    std::cout << "\033[31mFailed to delete pickup request. Status code: "
              << response.status_code << "\033[0m\n";
  }
}
