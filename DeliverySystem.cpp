// Module: Graph Representation and Road Connections
#include "DeliverySystem.h"

// Constructor
DeliverySystem::DeliverySystem()
{
    places = {
        "Dehradun",
        "Haridwar",
        "Rishikesh",
        "Roorkee",
        "Mussoorie",
        "Saharanpur",
        "Meerut",
        "Delhi",
        "Doiwala",
        "Vikasnagar"
    };

    // Predefined road distances (sample values in km)
    addRoad(0, 1, 55);
    addRoad(0, 2, 35);
    addRoad(0, 4, 34);
    addRoad(0, 8, 22);
    addRoad(0, 9, 40);
    addRoad(1, 2, 25);
    addRoad(1, 3, 30);
    addRoad(1, 5, 65);
    addRoad(2, 8, 20);
    addRoad(3, 5, 40);
    addRoad(3, 6, 90);
    addRoad(5, 6, 70);
    addRoad(6, 7, 70);
    addRoad(5, 7, 180);
    addRoad(4, 9, 55);
}

// Add road between two locations
void DeliverySystem::addRoad(int a, int b, int distance)
{
    graph[a].push_back({b, distance});
    graph[b].push_back({a, distance});
}

// Display all locations
void DeliverySystem::showLocations()
{
    cout << "\n===== AVAILABLE LOCATIONS =====\n";

    for (int i = 0; i < places.size(); i++)
    {
        cout << i << " - " << places[i] << endl;
    }
}

// Display all locations
void DeliverySystem::showLocations()
{
    cout << "\n===== AVAILABLE LOCATIONS =====\n";

    for (int i = 0; i < places.size(); i++)
    {
        cout << i << " - " << places[i] << endl;
    }
}

int DeliverySystem::shortestDistance(int source, int destination)
{
    vector<int> dist(10, INT_MAX);

    priority_queue<
        pair<int, int>,
        vector<pair<int, int>>,
        greater<pair<int, int>>
    > pq;

    dist[source] = 0;
    pq.push({0, source});

    while (!pq.empty())
    {
        int currentDistance = pq.top().first;
        int current = pq.top().second;
        pq.pop();

        if (currentDistance > dist[current])
            continue;

        for (auto road : graph[current])
        {
            int next = road.first;
            int weight = road.second;

            if (dist[current] + weight < dist[next])
            {
                dist[next] = dist[current] + weight;
                pq.push({dist[next], next});
            }
        }
    }

    return dist[destination];
}
// Optimize delivery route
void DeliverySystem::optimizeRoute()
{
    int warehouse, n;

    showLocations();

    cout << "\nEnter warehouse location ID: ";
    cin >> warehouse;

    if (warehouse < 0 || warehouse >= 10)
    {
        cout << "Invalid warehouse ID!\n";
        return;
    }

    cout << "Enter number of delivery locations: ";
    cin >> n;

    if (n < 1 || n > 9)
    {
        cout << "Invalid number of deliveries!\n";
        return;
    }

    vector<bool> selected(10, false);
    selected[warehouse] = true;

    while (!deliveryQueue.empty())
        deliveryQueue.pop();

    cout << "Enter delivery location IDs:\n";

    for (int i = 0; i < n; i++)
    {
        int location;
        cin >> location;

        if (location < 0 || location >= 10 || selected[location])
        {
            cout << "Invalid or duplicate location!\n";

            while (!deliveryQueue.empty())
                deliveryQueue.pop();

            return;
        }

        selected[location] = true;
        deliveryQueue.push(location);
    }

    int current = warehouse;
    int totalDistance = 0;

    cout << "\nDelivery Route:\n";
    cout << places[current];

    while (!deliveryQueue.empty())
    {
        int nearest = -1;
        int minDistance = INT_MAX;

        queue<int> remaining;

        while (!deliveryQueue.empty())
        {
            int location = deliveryQueue.front();
            deliveryQueue.pop();

            int distance = shortestDistance(current, location);

            if (distance < minDistance)
            {
                minDistance = distance;
                nearest = location;
            }

            remaining.push(location);
        }

        bool removed = false;

        while (!remaining.empty())
        {
            int location = remaining.front();
            remaining.pop();

            if (location == nearest && !removed)
                removed = true;
            else
                deliveryQueue.push(location);
        }

        if (nearest == -1 || minDistance == INT_MAX)
        {
            cout << "\nNo route available!\n";
            return;
        }

        totalDistance += minDistance;
        current = nearest;

        cout << " -> " << places[current];
    }

    cout << "\nTotal Distance: " << totalDistance << " km\n";
    cout << "All deliveries completed!\n";
}
