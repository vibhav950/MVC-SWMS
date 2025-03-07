#include "log.hpp"
#include "view.hpp"

int main() {
  /* main loop for the UI */
  View::CView view;
  int choice;

  view.displayMenu();
  do {
    std::cout << "Enter choice: ";
    std::cin >> choice;
    std::cin.ignore();

    switch (choice) {
    case 1:
      view.createPickupRequest();
      break;
    case 2:
      view.viewPickupRequestById();
      break;
    case 3:
      view.viewAllPickupRequests();
      break;
    case 4:
      view.updatePickupRequest();
      break;
    case 5:
      view.deletePickupRequest();
      break;
    case 6:
      view.displayMenu();
      break;
    case 7:
      std::cout << "Exiting...\n";
      break;
    default:
      std::cout << "Invalid choice\n";
    }
    std::cout << '\n';
  } while (choice != 7);
}