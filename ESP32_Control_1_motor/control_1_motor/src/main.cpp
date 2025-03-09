#include <WiFi.h>
#include <WebSocketsServer.h>

// Thông tin WiFi Access Point
const char* ssid = "ESP32_Server";
const char* password = "12345678";

// Khởi tạo WebSocket server trên port 81
WebSocketsServer webSocket = WebSocketsServer(81);

// Định nghĩa chân LED và điều khiển động cơ
const int STATUS_LED_PIN = 2;    // LED nháy báo hiệu
const int IN1 = 13; // PWM cho IN1
const int IN2 = 12; // PWM cho IN2

// Cấu hình PWM
const int PWM_CHANNEL_IN1 = 0; // Kênh PWM cho IN1
const int PWM_CHANNEL_IN2 = 1; // Kênh PWM cho IN2
const int PWM_FREQ = 5000;       // Tần số PWM: 5kHz
const int PWM_RESOLUTION = 8;    // Độ phân giải: 8-bit (0-255)

// Biến để theo dõi trạng thái LED nháy
uint8_t previousStationNum = 0;
unsigned long previousMillis = 0;
const long interval = 500;
bool statusLedState = false;

// Hàm kiểm tra và nháy LED báo hiệu
void handleStatusLED() {
    uint8_t currentStationNum = WiFi.softAPgetStationNum();
    if (currentStationNum > 0) {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
            statusLedState = !statusLedState;
            digitalWrite(STATUS_LED_PIN, statusLedState ? HIGH : LOW);
            previousMillis = currentMillis;
        }
    } else {
        digitalWrite(STATUS_LED_PIN, LOW);
        statusLedState = false;
    }
}

// Hàm điều khiển động cơ: speed (0-255), forward (true: thuận, false: ngược)
void controlMotor(int speed, bool forward) {
  if (speed > 0) {
    if (forward) {
      ledcWrite(PWM_CHANNEL_IN1, speed); // IN1 PWM, quay thuận
      ledcWrite(PWM_CHANNEL_IN2, 0);     // IN2 LOW
      Serial.print("Forward, Speed: "); Serial.println(speed);
    } else {
      ledcWrite(PWM_CHANNEL_IN1, 0);     // IN1 LOW
      ledcWrite(PWM_CHANNEL_IN2, speed); // IN2 PWM, quay ngược
      Serial.print("Reverse, Speed: "); Serial.println(speed);
    }
  } else {
    ledcWrite(PWM_CHANNEL_IN1, 0);       // IN1 LOW
    ledcWrite(PWM_CHANNEL_IN2, 0);       // IN2 LOW
    Serial.println("Stopped");
  }
}

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
            char buffer[50];
            memcpy(buffer, payload, length);
            buffer[length] = '\0';
            Serial.println("Received from Laptop: " + String(buffer));

            // Xử lý lệnh PWM cho động cơ
            int speed;
      if (sscanf(buffer, "forward:%d", &speed) == 1) {
        if (speed >= 0 && speed <= 255) {
          controlMotor(speed, true);
          webSocket.sendTXT(num, "Motor forward at speed " + String(speed));
        } else {
          webSocket.sendTXT(num, "Speed must be 0-255");
        }
      } else if (sscanf(buffer, "reverse:%d", &speed) == 1) {
        if (speed >= 0 && speed <= 255) {
          controlMotor(speed, false);
          webSocket.sendTXT(num, "Motor reverse at speed " + String(speed));
        } else {
          webSocket.sendTXT(num, "Speed must be 0-255");
        }
      } else {
        webSocket.sendTXT(num, "Invalid command. Use: forward:X or reverse:X (X=0-255)");
      }
      break;
  }
}

void setup() {
    // Cấu hình chân LED nháy
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW);

    // Cấu hình chân điều khiển động cơ
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);

  // Cấu hình PWM cho IN1
  ledcSetup(PWM_CHANNEL_IN1, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(IN1, PWM_CHANNEL_IN1);

  // Cấu hình PWM cho IN2
  ledcSetup(PWM_CHANNEL_IN2, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(IN2, PWM_CHANNEL_IN2);

  // Đặt trạng thái ban đầu: dừng động cơ
  ledcWrite(PWM_CHANNEL_IN1, 0);
  ledcWrite(PWM_CHANNEL_IN2, 0);

    Serial.begin(115200);
    delay(1000);
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
    webSocket.loop();
    handleStatusLED();

    uint8_t currentStationNum = WiFi.softAPgetStationNum();
    if (currentStationNum != previousStationNum) {
        Serial.print("Số thiết bị kết nối WiFi: ");
        Serial.println(currentStationNum);
        previousStationNum = currentStationNum;
    }
}

// const int IN1 = 13; // PWM cho IN1
// const int IN2 = 12; // PWM cho IN2

// const int PWM_CHANNEL_IN1 = 0; // Kênh PWM cho IN1
// const int PWM_CHANNEL_IN2 = 1; // Kênh PWM cho IN2
// const int PWM_FREQ = 5000;     // Tần số PWM 5kHz
// const int PWM_RESOLUTION = 8;  // Độ phân giải 8-bit (0-255)

// void setup() {
//   Serial.begin(115200);
//   pinMode(IN1, OUTPUT);
//   pinMode(IN2, OUTPUT);

//   // Cấu hình PWM cho IN1
//   ledcSetup(PWM_CHANNEL_IN1, PWM_FREQ, PWM_RESOLUTION);
//   ledcAttachPin(IN1, PWM_CHANNEL_IN1);

//   // Cấu hình PWM cho IN2
//   ledcSetup(PWM_CHANNEL_IN2, PWM_FREQ, PWM_RESOLUTION);
//   ledcAttachPin(IN2, PWM_CHANNEL_IN2);

//   // Đặt trạng thái ban đầu: dừng động cơ
//   ledcWrite(PWM_CHANNEL_IN1, 0);
//   ledcWrite(PWM_CHANNEL_IN2, 0);
// }

// // Hàm điều khiển động cơ: speed (0-255), forward (true: thuận, false: ngược)
// void controlMotor(int speed, bool forward) {
//   if (speed > 0) {
//     if (forward) {
//       ledcWrite(PWM_CHANNEL_IN1, speed); // IN1 PWM, quay thuận
//       ledcWrite(PWM_CHANNEL_IN2, 0);     // IN2 LOW
//       Serial.print("Forward, Speed: "); Serial.println(speed);
//     } else {
//       ledcWrite(PWM_CHANNEL_IN1, 0);     // IN1 LOW
//       ledcWrite(PWM_CHANNEL_IN2, speed); // IN2 PWM, quay ngược
//       Serial.print("Reverse, Speed: "); Serial.println(speed);
//     }
//   } else {
//     ledcWrite(PWM_CHANNEL_IN1, 0);       // IN1 LOW
//     ledcWrite(PWM_CHANNEL_IN2, 0);       // IN2 LOW
//     Serial.println("Stopped");
//   }
// }

// void loop() {
//   // Quay thuận, tăng tốc từ 0 đến 255
//   for (int speed = 0; speed <= 255; speed += 5) {
//     controlMotor(speed, true);
//     delay(100);
//   }
//   delay(2000);

//   // Giảm tốc về 0
//   for (int speed = 255; speed >= 0; speed -= 5) {
//     controlMotor(speed, true);
//     delay(100);
//   }
//   delay(2000);

//   // Quay ngược, tăng tốc từ 0 đến 255
//   for (int speed = 0; speed <= 255; speed += 5) {
//     controlMotor(speed, false);
//     delay(100);
//   }
//   delay(2000);

//   // Giảm tốc về 0
//   for (int speed = 255; speed >= 0; speed -= 5) {
//     controlMotor(speed, false);
//     delay(100);
//   }
//   delay(2000);
// }
