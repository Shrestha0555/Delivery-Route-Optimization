// Initialize Interactive Map
let map = L.map("map").setView([30.3165, 78.0322], 8);

// Load OpenStreetMap
L.tileLayer("https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png", {
    attribution: "&copy; OpenStreetMap contributors"
}).addTo(map);

// C++ Backend API address
const API_URL = "http://localhost:8080";

// HTML Elements
const warehouseSelect = document.getElementById("warehouse");
const deliverySelect = document.getElementById("deliveryLocations");
const locationGrid = document.getElementById("locationGrid");
const optimizeBtn = document.getElementById("optimizeBtn");
const routeOutput = document.getElementById("routeOutput");

// Locations received from C++ backend
let locations = [];
// Coordinates of delivery locations
const locationCoordinates = {
    "Dehradun": [30.3165, 78.0322],
    "Haridwar": [29.9457, 78.1642],
    "Rishikesh": [30.0869, 78.2676],
    "Roorkee": [29.8543, 77.8880],
    "Mussoorie": [30.4598, 78.0644],
    "Saharanpur": [29.9680, 77.5552],
    "Meerut": [28.9845, 77.7064],
    "Delhi": [28.6139, 77.2090],
    "Doiwala": [30.1767, 78.1160],
    "Vikasnagar": [30.4690, 77.7750]
};

// Page navigation
function showPage(pageId) {
    document.querySelectorAll(".page").forEach(function(page) {
        page.classList.remove("active");
    });

    document.querySelectorAll(".nav-link").forEach(function(link) {
        link.classList.remove("active");
    });

    document.getElementById(pageId).classList.add("active");

    const activeLink = document.querySelector(
        '.nav-link[data-page="' + pageId + '"]'
    );

    if (activeLink) {
        activeLink.classList.add("active");
    }
}

// Sidebar navigation
document.querySelectorAll(".nav-link").forEach(function(link) {
    link.addEventListener("click", function(event) {
        event.preventDefault();
        showPage(this.dataset.page);
    });
});

// Load locations from C++ backend
async function loadLocations() {
    try {
        const response = await fetch(API_URL + "/api/locations");

        if (!response.ok) {
            throw new Error("Could not load locations");
        }

        const data = await response.json();

        locations = data.locations;

        populateLocations();

        optimizeBtn.disabled = false;

        console.log("Locations loaded from C++ backend!");

    } catch (error) {
        console.error(error);

        alert("Backend connection failed! Please start the C++ server.");

        routeOutput.textContent =
            "Unable to connect to C++ backend.";
    }
}

// Populate dropdown and location cards
function populateLocations() {

    warehouseSelect.innerHTML =
        '<option value="">Select Warehouse</option>';

    deliverySelect.innerHTML = "";
    locationGrid.innerHTML = "";

    locations.forEach(function(item) {

        const index = item.id;
        const location = item.name;

        // Warehouse dropdown
        const option = document.createElement("option");

        option.value = index;
        option.textContent = location;

        warehouseSelect.appendChild(option);

        // Location cards
        const card = document.createElement("div");

        card.className = "location-card";

        const label = document.createElement("span");

        label.textContent = "LOCATION " + (index + 1);

        card.appendChild(label);
        card.appendChild(document.createTextNode(location));

        locationGrid.appendChild(card);

        // Delivery checkbox cards
        const deliveryCard = document.createElement("label");

        deliveryCard.className = "delivery-card";

        const checkbox = document.createElement("input");

        checkbox.type = "checkbox";
        checkbox.value = index;
        checkbox.className = "delivery-checkbox";

        const name = document.createElement("span");

        name.textContent = location;

        deliveryCard.appendChild(checkbox);
        deliveryCard.appendChild(name);

        deliverySelect.appendChild(deliveryCard);

        checkbox.addEventListener("change", function() {
            deliveryCard.classList.toggle("selected", this.checked);
        });
    });
}

// Prevent warehouse from being selected as delivery
warehouseSelect.addEventListener("change", function() {

    const warehouse = this.value;

    document.querySelectorAll(".delivery-checkbox").forEach(
        function(checkbox) {

            const label = checkbox.parentElement;

            checkbox.disabled = checkbox.value === warehouse;

            if (checkbox.disabled) {
                checkbox.checked = false;
                label.classList.remove("selected");
            }

            label.classList.toggle("disabled", checkbox.disabled);
        }
    );
});

// Display optimized route
function displayRoute(data, warehouse, deliveries) {

    routeOutput.innerHTML = "";
    // Clear previous map markers and routes
map.eachLayer(function(layer) {
    if (layer instanceof L.Marker || layer instanceof L.Polyline) {
        map.removeLayer(layer);
    }
});

// Add markers for all route locations
const routeCoordinates = [];

data.route.forEach(function(location) {

    const coordinates = locationCoordinates[location];

    if (coordinates) {
        L.marker(coordinates)
            .addTo(map)
            .bindPopup(location);

        routeCoordinates.push(coordinates);
    }
});

// Draw route line
if (routeCoordinates.length > 1) {

    const routeLine = L.polyline(routeCoordinates, {
        color: "#3b82f6",
        weight: 4
    }).addTo(map);

    map.fitBounds(routeLine.getBounds(), {
        padding: [30, 30]
    });
}

    const heading = document.createElement("h3");
    heading.textContent = "Route Optimized Successfully!";

    routeOutput.appendChild(heading);

    const warehouseText = document.createElement("p");

    warehouseText.textContent =
        "Warehouse: " + locations[warehouse].name;

    routeOutput.appendChild(warehouseText);

    const deliveryText = document.createElement("p");

    deliveryText.textContent =
        "Total Deliveries: " + deliveries.length;

    routeOutput.appendChild(deliveryText);

    const routeHeading = document.createElement("h3");

    routeHeading.textContent = "Optimized Delivery Route";

    routeOutput.appendChild(routeHeading);

    const routeList = document.createElement("ol");

    data.route.forEach(function(location) {

        const item = document.createElement("li");

        item.textContent = location;

        routeList.appendChild(item);
    });

    routeOutput.appendChild(routeList);

    const distance = document.createElement("h2");

    distance.textContent =
        "Total Distance: " + data.totalDistance + " km";

    routeOutput.appendChild(distance);

    const success = document.createElement("p");

    success.textContent = "All deliveries completed!";

    routeOutput.appendChild(success);
}

// Optimize delivery route using C++ backend
optimizeBtn.addEventListener("click", async function() {

    const warehouse = warehouseSelect.value;

    const deliveries = Array.from(
        document.querySelectorAll(".delivery-checkbox:checked")
    ).map(function(checkbox) {
        return Number(checkbox.value);
    });

    if (warehouse === "") {
        alert("Please select a warehouse location!");
        return;
    }

    if (deliveries.length === 0) {
        alert("Please select at least one delivery location!");
        return;
    }

    optimizeBtn.disabled = true;

    optimizeBtn.textContent = "Calculating Route...";

    try {

        const response = await fetch(API_URL + "/api/optimize", {

            method: "POST",

            headers: {
                "Content-Type": "application/json"
            },

            body: JSON.stringify({
                warehouse: Number(warehouse),
                deliveries: deliveries
            })
        });

        const data = await response.json();

        if (!response.ok) {
            throw new Error(data.error || "Route calculation failed");
        }

        displayRoute(data, Number(warehouse), deliveries);

        showPage("route");
        setTimeout(function() {
    map.invalidateSize();
}, 200);

    } catch (error) {

        alert("Route optimization failed: " + error.message);

        console.error(error);

    } finally {

        optimizeBtn.disabled = false;

        optimizeBtn.textContent = "Optimize Delivery Route →";
    }
});

// Disable button until backend locations load
optimizeBtn.disabled = true;

// Start application
loadLocations();