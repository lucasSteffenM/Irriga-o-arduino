#include <ESP8266WiFi.h>  // ou ESP32WiFi.h para ESP32
#include <WiFiClientSecure.h>

const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";
const char* host = "api.twilio.com";
const int httpsPort = 443;

const String account_sid = "your_account_SID";
const String auth_token = "your_auth_token";
const String from_whatsapp_number = "whatsapp:+your_twilio_whatsapp_number";
const String to_whatsapp_number = "whatsapp:+your_phone_number";

const int sensorPin = A0;
const int relayPin = D1;  // Mude para o pino correto

bool flag = false;

void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  // Conectando ao Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao WiFi");
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    int value = input.toInt();

    if (!isNaN(value) && value >= 0 && value <= 876) {
      Serial.print("Valor: ");
      Serial.println(value);

      if (value <= 292 && !flag) {
        Serial.println("Baixa umidade, a rega foi iniciada");
        digitalWrite(relayPin, HIGH);  // Ativa a irrigação
        enviarMensagemWhatsApp("Irrigação iniciada!");
        flag = true;
      } else if (value > 292) {
        flag = false;
        digitalWrite(relayPin, LOW);  // Desativa a irrigação
      }
    }
  }

  delay(1000);  // Pequeno atraso para evitar sobrecarga de leitura
}

void enviarMensagemWhatsApp(String mensagem) {
  WiFiClientSecure client;
  client.setInsecure();

  if (!client.connect(host, httpsPort)) {
    Serial.println("Conexão falhou");
    return;
  }

  String url = "/2010-04-01/Accounts/" + account_sid + "/Messages.json";
  String auth = "Basic " + base64::encode(account_sid + ":" + auth_token);

  String postData = "From=" + from_whatsapp_number + "&To=" + to_whatsapp_number + "&Body=" + mensagem;

  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Authorization: " + auth + "\r\n" +
               "Content-Type: application/x-www-form-urlencoded\r\n" +
               "Content-Length: " + postData.length() + "\r\n" +
               "Connection: close\r\n\r\n" +
               postData);

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }

  String response = client.readString();
  Serial.println("Resposta:");
  Serial.println(response);
}
