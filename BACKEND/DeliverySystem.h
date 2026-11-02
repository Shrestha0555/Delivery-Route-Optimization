#ifndef DELIVERY_SYSTEM_H
#define DELIVERY_SYSTEM_H

#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <climits>

using namespace std;

class DeliverySystem
{
private:
vector<string> places;
vector<pair<int, int>> graph[10];
queue<int> deliveryQueue;

public:
DeliverySystem();


void addRoad(int a, int b, int distance);

void showLocations();

int shortestDistance(int source, int destination);

void optimizeRoute();

// New function for website integration
bool calculateRoute(
    int warehouse,
    const vector<int>& deliveries,
    vector<int>& route,
    int& totalDistance
);

const vector<string>& getLocations() const;

};



#endif
