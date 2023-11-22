#include <WiFi.h>
#include <HTTPClient.h>
#include <TinyGPS++.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Fingerprint.h>

const char* ssid = "Chamith rulz";
const char* password = "Chamith17321";

String serverName = "http://192.168.8.138/v_tracking/attendance.php";
String serverName2 = "http://192.168.8.138/v_tracking/history.php";
unsigned long lastTime = 0;
unsigned long timerDelay = 20000;

HardwareSerial Sender(1);
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define DS18B20PIN 13
#define Receiver_Txd_pin 27
#define Receiver_Rxd_pin 14
#define RXD2 16
#define TXD2 17
#define buzzer 15
#define button 23

Adafruit_MPU6050 mpu;
String Latitude, accident;
String Longitude;
int temp, Speed, lock, sta1;

TinyGPSPlus gps;
OneWire oneWire(DS18B20PIN);
DallasTemperature sensor(&oneWire);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2);


void setup() {
  Serial.begin(9600);
  pinMode(buzzer, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  Sender.begin(9600, SERIAL_8N1, Receiver_Txd_pin, Receiver_Rxd_pin);
  Wire.begin();
  sensor.begin();

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  Serial.println("MPU6050 Found!");
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
    case MPU6050_RANGE_2_G:
      Serial.println("+-2G");
      break;
    case MPU6050_RANGE_4_G:
      Serial.println("+-4G");
      break;
    case MPU6050_RANGE_8_G:
      Serial.println("+-8G");
      break;
    case MPU6050_RANGE_16_G:
      Serial.println("+-16G");
      break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
    case MPU6050_RANGE_250_DEG:
      Serial.println("+- 250 deg/s");
      break;
    case MPU6050_RANGE_500_DEG:
      Serial.println("+- 500 deg/s");
      break;
    case MPU6050_RANGE_1000_DEG:
      Serial.println("+- 1000 deg/s");
      break;
    case MPU6050_RANGE_2000_DEG:
      Serial.println("+- 2000 deg/s");
      break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
    case MPU6050_BAND_260_HZ:
      Serial.println("260 Hz");
      break;
    case MPU6050_BAND_184_HZ:
      Serial.println("184 Hz");
      break;
    case MPU6050_BAND_94_HZ:
      Serial.println("94 Hz");
      break;
    case MPU6050_BAND_44_HZ:
      Serial.println("44 Hz");
      break;
    case MPU6050_BAND_21_HZ:
      Serial.println("21 Hz");
      break;
    case MPU6050_BAND_10_HZ:
      Serial.println("10 Hz");
      break;
    case MPU6050_BAND_5_HZ:
      Serial.println("5 Hz");
      break;
  }
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2);
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit finger detect test");

  // set the data rate for the sensor serial port
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) {
      delay(1);
    }
  }

  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  Serial.println("Waiting for valid finger...");
}

void loop() {

  getFingerprintIDez();
  if (finger.fingerID == 1) {
    lock = 1;
    attendanceReq();
  } else if (finger.fingerID == 2) {
    lock = 1;
    attendanceReq();
  }
  while (lock) {
    rd18b20();
    mpuRead();
    dis();
    if (digitalRead(button) == LOW) {
      digitalWrite(buzzer, HIGH);
      delay(150);
      digitalWrite(buzzer, LOW);
      delay(150);
      smsPro();
      delay(1000);
    }
    if ((millis() - lastTime) > timerDelay) {
      historyReq();
      lastTime = millis();
    }
    while (Sender.available() > 0)
      if (gps.encode(Sender.read()))
        displayInfo();
  }

}
void rd18b20() {
  sensor.requestTemperatures();
  Serial.print("Temperature is: ");
  Serial.println(sensor.getTempCByIndex(0));
  temp = sensor.getTempCByIndex(0);

}
void displayInfo() {
  if (gps.location.isValid()) {
    Latitude = gps.location.lat();
    Longitude = gps.location.lng();
    Speed = gps.speed.value() * 1.852;
    Serial.print(F("Latitude: "));
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(F("Longitude: "));
    Serial.print(gps.location.lng(), 6);
    Serial.print(F(","));
    Serial.print(F("Speed: "));
    Serial.print(gps.speed.value() * 1.852, 6);
    Serial.println(F(" km/h"));

    if (Speed > 60) {
      Serial.println("Speed HIGH");
      digitalWrite(buzzer, HIGH);
      delay(150);
      digitalWrite(buzzer, LOW);
      delay(150);
    }


  } else {
    Serial.print(F("INVALID"));
  }
  Serial.println();
}
void mpuRead() {

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  Serial.print("Acceleration X: ");
  Serial.print(a.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");

  if (a.acceleration.x < 1 && sta1 == 0) {
    accident = "accident";
    sta1 = 1;
    smsPro();
  } else {
    sta1 = 0;
    accident = "NO accident";
  }

  Serial.print("Rotation X: ");
  Serial.print(g.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" rad/s");

  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" degC");

  Serial.println("");
  delay(1000);
}
void dis() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(15, 0);
  display.println("v_tracking System");
  display.setCursor(0, 20);
  display.print("V Number : ABC5642");
  display.setCursor(0, 30);
  display.print("Mobile : 0712264093");
  display.setCursor(0, 40);
  display.print("Speed : ");
  display.print(Speed);
  display.print(" km/h");
  display.setCursor(0, 50);
  display.print("Temperature : ");
  display.print(temp);
  display.print(" degC");
  display.display();
  delay(100);
}
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}
void attendanceReq() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClient client;

    String serverPath = serverName + "?api_key=tPmAT5Ab3j7F9&user_id=" + finger.fingerID + "";
    Serial.println(serverPath);
    http.begin(client, serverPath.c_str());
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }
}
void historyReq() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClient client;

    String serverPath = serverName2 + "?api_key=tPmAT5Ab3j7F9&Temperature=" + temp + "&Latitude=" + (String)Latitude + "&Longitude=" + Longitude + "&accident=" + accident + "&Speed=" + Speed + "";
    Serial.println(serverPath);
    http.begin(client, serverPath.c_str());
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }
}
void smsPro() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String serverPath = "https://www.textit.biz/sendmsg/?id=94719718093&pw=4284&to=0719718093&text=Alert+:+Car+Crash+Detection+Location+:+https://www.google.com/maps/?q=" + Latitude + "," + Longitude + "";
    Serial.println(serverPath);
    http.begin(serverPath.c_str());

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }
}
