 //////////////////////// CONFIG SYSTEM ////////////////////////
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <HardwareSerial.h>

// Thông tin WiFi Access Point
const char* ssid = "ESP32_Server";
const char* password = "12345678";

// Khởi tạo WebSocket server trên port 81
WebSocketsServer webSocket = WebSocketsServer(81);

// Định nghĩa chân LED
const int STATUS_LED_PIN = 2;
const int LED1_PIN = 4;          // LED 1 điều khiển
const int LED2_PIN = 5;          // LED 2 điều khiển
const int LED3_PIN = 18;         // LED 3 điều khiển
const int LED4_PIN = 19;         // LED 4 điều khiển
 //////////////////////// CONFIG SYSTEM ////////////////////////

// Biến để theo dõi trạng thái và nháy LED
uint8_t previousStationNum = 0;
unsigned long previousMillis = 0; // Thời điểm lần cuối LED thay đổi trạng thái
const long interval = 500;        // Thời gian bật/tắt LED (0.5 giây)
bool ledState = false;            // Trạng thái hiện tại của LED

// Hàm kiểm tra và nháy LED khi có thiết bị kết nối WiFi
void handleLED() {
    uint8_t currentStationNum = WiFi.softAPgetStationNum();
    if (currentStationNum > 0) { // Có thiết bị kết nối WiFi
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
            ledState = !ledState; // Đổi trạng thái LED
            digitalWrite(STATUS_LED_PIN, ledState ? HIGH : LOW);
            previousMillis = currentMillis; // Cập nhật thời điểm
        }
    } else { // Không có thiết bị kết nối
        digitalWrite(STATUS_LED_PIN, LOW); // Tắt LED
        ledState = false;           // Đặt lại trạng thái
    }
}
 //////////////////////// CONFIG SYSTEM ////////////////////////


// Hàm xử lý sự kiện WebSocket
void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.printf("Client %u disconnected\n", num);
            break;
        case WStype_CONNECTED:
            Serial.printf("Client %u connected\n", num);
            webSocket.sendTXT(num, "Connected to ESP32 WebSocket Server");
            break;
        case WStype_TEXT:
            char buffer[50]; // Bộ đệm cho lệnh từ laptop
            memcpy(buffer, payload, length);
            buffer[length] = '\0'; // Kết thúc chuỗi
            Serial.println("Received from Laptop: " + String(buffer));     

           // Xử lý lệnh bật/tắt 4 LED
          if (strcmp(buffer, "led1_on") == 0) {
            digitalWrite(LED1_PIN, HIGH);
            webSocket.sendTXT(num, "LED 1 is ON");
        } else if (strcmp(buffer, "led1_off") == 0) {
            digitalWrite(LED1_PIN, LOW);
            webSocket.sendTXT(num, "LED 1 is OFF");
        } else if (strcmp(buffer, "led2_on") == 0) {
            digitalWrite(LED2_PIN, HIGH);
            webSocket.sendTXT(num, "LED 2 is ON");
        } else if (strcmp(buffer, "led2_off") == 0) {
            digitalWrite(LED2_PIN, LOW);
            webSocket.sendTXT(num, "LED 2 is OFF");
        } else if (strcmp(buffer, "led3_on") == 0) {
            digitalWrite(LED3_PIN, HIGH);
            webSocket.sendTXT(num, "LED 3 is ON");
        } else if (strcmp(buffer, "led3_off") == 0) {
            digitalWrite(LED3_PIN, LOW);
            webSocket.sendTXT(num, "LED 3 is OFF");
        } else if (strcmp(buffer, "led4_on") == 0) {
            digitalWrite(LED4_PIN, HIGH);
            webSocket.sendTXT(num, "LED 4 is ON");
        } else if (strcmp(buffer, "led4_off") == 0) {
            digitalWrite(LED4_PIN, LOW);
            webSocket.sendTXT(num, "LED 4 is OFF");
        } else {
            webSocket.sendTXT(num, "Command received: " + String(buffer));
        }
        break;
    }
}

void setup() {
  // Cấu hình chân LED
  pinMode(STATUS_LED_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW); // Tắt LED nháy lúc đầu
  digitalWrite(LED1_PIN, LOW);       // Tắt LED 1 lúc đầu
  digitalWrite(LED2_PIN, LOW);       // Tắt LED 2 lúc đầu
  digitalWrite(LED3_PIN, LOW);       // Tắt LED 3 lúc đầu
  digitalWrite(LED4_PIN, LOW);       // Tắt LED 4 lúc đầu



 //////////////////////// CONFIG SYSTEM ////////////////////////
    Serial.begin(115200);
    delay(1000); // Đợi Serial khởi động
    Serial.println("Booting ESP32...");

    // Khởi động WiFi Access Point
    if (WiFi.softAP(ssid, password)) {
        Serial.println("WiFi AP started successfully!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.println("Error: Unable to start WiFi AP!");
    }

    // Khởi động WebSocket server
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("WebSocket Server has started! Waiting for connections...");
}

void loop() {
    webSocket.loop(); // Xử lý các sự kiện WebSocket
    handleLED();      // Điều khiển LED nháy

    // Kiểm tra và in số lượng thiết bị kết nối WiFi khi có thay đổi
    uint8_t currentStationNum = WiFi.softAPgetStationNum();
    if (currentStationNum != previousStationNum) {
        Serial.print("Số thiết bị kết nối WiFi: ");
        Serial.println(currentStationNum);
        previousStationNum = currentStationNum;
    }
}
 //////////////////////// CONFIG SYSTEM ////////////////////////
