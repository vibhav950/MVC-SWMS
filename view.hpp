#pragma once

namespace View {

class CView {
public:
  CView() {}
  void displayMenu();
  void createPickupRequest();
  void viewPickupRequestById();
  void viewAllPickupRequests();
  void updatePickupRequest();
  void deletePickupRequest();
};
} // namespace View
