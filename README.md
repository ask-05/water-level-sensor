# ESP32 Ultrasonic Water Level Sensor

This project is an ESP-IDF based application that measures water levels using an AJ-SR04M ultrasonic sensor and transmits the data to an AWS API Gateway. It is built using FreeRTOS to ensure non-blocking, interrupt-driven sensor readings and reliable WiFi connectivity.

## Features
* **Hardware Interrupts:** Uses ESP32 GPIO ANYEDGE interrupts and FreeRTOS queues to measure acoustic echo times without blocking the CPU scheduler.
* **HTTPS Cloud Sync:** Securely packages sensor data into JSON payloads and POSTs them to a remote AWS server using `esp_http_client` and mbedTLS.

## Hardware Requirements
* ESP32 Development Board
* AJ-SR04M Ultrasonic Sensor
* 2x Resistors (e.g., 1kΩ and 2kΩ) for a voltage divider

### Wiring
The AJ-SR04M outputs a 5V signal on the Echo pin, which can damage the 3.3V logic pins of the ESP32. A voltage divider is required to safely step this down. It would look something like: ECHO Pin -> 10k ohm -> 10k ohm -> 10k ohm -> GND and at the point where the first two 10k ohm resistors connect, connect your GPIO pin to that point.

## Local Setup & Installation

1. **Prerequisites:** Ensure you have the ESP-IDF framework installed and set up.
2. **Clone the Repository:**
   ```bash
   git clone <your-repo-link>
   cd <your-repo-folder>
   ```
3. **Set up menuconfig:**  On menuconfig, set your WiFi SSID and Password along with other security features in WiFi Configuration. Also set your AWS Gateway URL and API Key in AWS Configuration.
4. **Build, Flash, and monitor:** Your project should be up and running!

# Acknowledgements
The station example from the ESP-IDF examples was used in this project.