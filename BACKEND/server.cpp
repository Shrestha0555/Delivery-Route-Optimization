#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <winsock2.h>

#include "DeliverySystem.h"

using namespace std;

// Send complete HTTP response
void sendResponse(SOCKET client, int status,
                  const string& contentType,
                  const string& body)
{
    string statusText = (status == 200) ? "OK" : "Bad Request";

    string response =
        "HTTP/1.1 " + to_string(status) + " " + statusText + "\r\n"
        "Content-Type: " + contentType + "\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Content-Length: " + to_string(body.size()) + "\r\n"
        "Connection: close\r\n\r\n" + body;

    send(client, response.c_str(), response.size(), 0);
}

// Read integer from JSON
bool getNumber(const string& json, const string& key, int& value)
{
    size_t pos = json.find("\"" + key + "\"");

    if (pos == string::npos)
        return false;

    pos = json.find(':', pos);

    if (pos == string::npos)
        return false;

    pos++;

    while (pos < json.size() && json[pos] == ' ')
        pos++;

    try
    {
        size_t used;
        value = stoi(json.substr(pos), &used);
        return used > 0;
    }
    catch (...)
    {
        return false;
    }
}

// Read delivery IDs from JSON array
bool getDeliveries(const string& json, vector<int>& deliveries)
{
    size_t pos = json.find("\"deliveries\"");

    if (pos == string::npos)
        return false;

    pos = json.find('[', pos);

    if (pos == string::npos)
        return false;

    size_t end = json.find(']', pos);

    if (end == string::npos)
        return false;

    string values = json.substr(pos + 1, end - pos - 1);

    stringstream ss(values);

    int number;

    while (ss >> number)
    {
        deliveries.push_back(number);

        ss >> ws;

        if (ss.peek() == ',')
            ss.get();
    }

    return !deliveries.empty();
}

// Main server
int main()
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "Winsock initialization failed!";
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == INVALID_SOCKET)
    {
        cout << "Socket creation failed!";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddress,
             sizeof(serverAddress)) == SOCKET_ERROR)
    {
        cout << "Port binding failed!";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    listen(serverSocket, 5);

    cout << "RouteFlow API running at http://localhost:8080"
         << endl;

    DeliverySystem system;

    while (true)
    {
        SOCKET client = accept(serverSocket, NULL, NULL);

        if (client == INVALID_SOCKET)
            continue;

        string request;
        char buffer[4096];

        size_t headerEnd = string::npos;
        int contentLength = 0;

        // Read HTTP request
        while (true)
        {
            int received = recv(client, buffer, sizeof(buffer), 0);

            if (received <= 0)
                break;

            request.append(buffer, received);

            headerEnd = request.find("\r\n\r\n");

            if (headerEnd != string::npos)
            {
                size_t p = request.find("Content-Length:");

                if (p != string::npos && p < headerEnd)
                {
                    p += 15;

                    while (p < headerEnd && request[p] == ' ')
                        p++;

                    contentLength = atoi(request.c_str() + p);
                }

                if (request.size() >= headerEnd + 4 + contentLength)
                    break;
            }

            if (request.size() > 100000)
                break;
        }

        if (headerEnd == string::npos)
        {
            closesocket(client);
            continue;
        }

        string headers = request.substr(0, headerEnd);

        string body = request.substr(headerEnd + 4);

        stringstream firstLine(headers);

        string method, path;

        firstLine >> method >> path;

        // Browser CORS preflight
        if (method == "OPTIONS")
        {
            sendResponse(client, 200, "text/plain", "");
        }

        // GET locations
        else if (method == "GET" && path == "/api/locations")
        {
            const vector<string>& locations = system.getLocations();

            string json = "{\"locations\":[";

            for (int i = 0; i < locations.size(); i++)
            {
                if (i > 0)
                    json += ",";

                json += "{\"id\":" + to_string(i) +
                        ",\"name\":\"" + locations[i] + "\"}";
            }

            json += "]}";

            sendResponse(client, 200, "application/json", json);
        }

        // POST optimize route
        else if (method == "POST" && path == "/api/optimize")
        {
            int warehouse;

            vector<int> deliveries;
            vector<int> route;

            int totalDistance = 0;

            bool valid =
                getNumber(body, "warehouse", warehouse) &&
                getDeliveries(body, deliveries);

            bool success = false;

            if (valid)
            {
                success = system.calculateRoute(
                    warehouse,
                    deliveries,
                    route,
                    totalDistance
                );
            }

            if (!success)
            {
                sendResponse(
                    client,
                    400,
                    "application/json",
                    "{\"error\":\"Invalid delivery request\"}"
                );
            }
            else
            {
                const vector<string>& locations =
                    system.getLocations();

                string json = "{\"route\":[";

                for (int i = 0; i < route.size(); i++)
                {
                    if (i > 0)
                        json += ",";

                    json += "\"" + locations[route[i]] + "\"";
                }

                json += "],\"totalDistance\":" +
                        to_string(totalDistance) + "}";

                sendResponse(client, 200, "application/json", json);
            }
        }

        else
        {
            sendResponse(
                client,
                400,
                "application/json",
                "{\"error\":\"Endpoint not found\"}"
            );
        }

        closesocket(client);
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}