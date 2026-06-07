#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// WiFi
const char* ssid = "Pranto";
const char* password = "12345678";

// Telegram
String chatId = " 6165122373";
String BOTtoken = "8653197621:AAF0K2_0mTIK4nDdDS60qhx2swuAbQ_dqRk";

WiFiClientSecure clientTCP;
UniversalTelegramBot bot(BOTtoken, clientTCP);

// GPIO
#define BUTTON 12
#define LOCK 13   // Relay / Lock control

// Camera Pins (AI Thinker)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

bool sendPhoto = false;
int lockState = 0;

// 🔥 Auto lock timer
unsigned long unlockTime = 0;
bool autoLockPending = false;

const unsigned long BOT_MTBS = 1000;
unsigned long bot_lasttime;

// ================= LOCK FUNCTIONS =================

String unlockDoor() {
  if (lockState == 0) {
    digitalWrite(LOCK, HIGH);
    lockState = 1;

    // start auto lock timer
    unlockTime = millis();
    autoLockPending = true;

    return "Door Unlocked (Auto lock in 5s)";
  }
  return "Already Unlocked";
}

String lockDoor() {
  digitalWrite(LOCK, LOW);
  lockState = 0;
  autoLockPending = false;
  return "Door Locked";
}

// ================= PHOTO SEND =================

String sendPhotoTelegram() {
  const char* myDomain = "api.telegram.org";

  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    ESP.restart();
    return "Camera error";
  }

  if (clientTCP.connect(myDomain, 443)) {

    String head = "--123\r\nContent-Disposition: form-data; name=\"chat_id\";\r\n\r\n" + chatId +
                  "\r\n--123\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";

    String tail = "\r\n--123--\r\n";

    uint32_t totalLen = fb->len + head.length() + tail.length();

    clientTCP.println("POST /bot" + BOTtoken + "/sendPhoto HTTP/1.1");
    clientTCP.println("Host: api.telegram.org");
    clientTCP.println("Content-Length: " + String(totalLen));
    clientTCP.println("Content-Type: multipart/form-data; boundary=123");
    clientTCP.println();

    clientTCP.print(head);

    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;

    for (size_t n = 0; n < fbLen; n += 1024) {
      if (n + 1024 < fbLen)
        clientTCP.write(fbBuf, 1024);
      else
        clientTCP.write(fbBuf, fbLen % 1024);
      fbBuf += 1024;
    }

    clientTCP.print(tail);
    esp_camera_fb_return(fb);
    clientTCP.stop();

    return "Photo sent";
  }

  return "Connection failed";
}

// ================= TELEGRAM =================

void handleNewMessages(int numNewMessages) {

  for (int i = 0; i < numNewMessages; i++) {

    String chat_id = String(bot.messages[i].chat_id);

    if (chat_id != chatId) {
      bot.sendMessage(chat_id, "Unauthorized", "");
      continue;
    }

    String text = bot.messages[i].text;

    if (text == "/photo") {
      sendPhoto = true;
    }

    if (text == "/unlock") {
      bot.sendMessage(chatId, unlockDoor(), "");
    }

    if (text == "/lock") {
      bot.sendMessage(chatId, lockDoor(), "");
    }

    if (text == "/start") {
      bot.sendMessage(chatId,
        "/photo\n/unlock\n/lock\nAuto lock = 5 sec",
        "");
    }
  }
}

// ================= SETUP =================

void setup() {

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);

  pinMode(LOCK, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  digitalWrite(LOCK, LOW);

  WiFi.begin(ssid, password);
  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi Connected");

  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_CIF;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_camera_init(&config);
}

// ================= LOOP =================

void loop() {

  // 📸 Telegram Photo
  if (sendPhoto) {
    sendPhotoTelegram();
    sendPhoto = false;
  }

  // 🔘 Button press → photo
  if (digitalRead(BUTTON) == LOW) {
    sendPhotoTelegram();
    delay(500);
  }

  // 🔥 AUTO LOCK AFTER 5 SEC
  if (autoLockPending && millis() - unlockTime >= 5000) {
    lockDoor();
    Serial.println("Auto Locked");
  }

  // 🤖 Telegram check
  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}