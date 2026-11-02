#include "DeliverySystem.h"

int main()
{
    DeliverySystem system;

    int choice;

    do
    {
        cout << "\n================================\n";
        cout << "   DELIVERY ROUTE OPTIMIZER\n";
        cout << "================================\n";

        cout << "1. Show Available Locations\n";
        cout << "2. Optimize Delivery Route\n";
        cout << "3. Exit\n";

        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice)
        {
            case 1:
                system.showLocations();
                break;

            case 2:
                system.optimizeRoute();
                break;

            case 3:
                cout << "Thank you for using Delivery Route Optimizer!\n";
                break;

            default:
                cout << "Invalid choice! Try again.\n";
        }

    } while (choice != 3);

    return 0;
}