#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// Definición de los pines
#define SENSOR_PIN 32
#define RELAY_PIN 2
#define Relay_pin_2 4

// Definición de las credenciales de Firebase
#define API_KEY "AIzaSyBKgFBiz65OkJyNITzAwRWDecIf0PnTlA0"
#define DATABASE_URL "https://proyectofinal-3d49e-default-rtdb.firebaseio.com/"

// Definición de los pines
const int umbral_bajo = 30;
const int umbral_alto = 40;
bool motorEncendido = false;

// Inicializa Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Inicializa la variable para almacenar la orden
String orden;

// Escribe aquí el nombre de tu red y su contraseña del internet henry
const char* red = "VENTURA HY 2.4G";
const char* password = "73444991maydaz";

void setup() {
  Serial.begin(115200);
  analogReadResolution(12); // 12 bits de profundidad en la lectura

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Motor apagado en el alba

  WiFi.begin(red, password);
  delay(2000);

  Serial.print("Se está conectando a la red: ");
  Serial.println(red);

  // Espera a que se conecte a la red WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("✨ Conexión establecida, el motor escucha tu canto. ✨");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  
  auth.user.email = "iot@example.com";  // Usuario registrado en Firebase
  auth.user.password = "password123";   // Contraseña del usuario

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  Firebase.setDoubleDigits(5);
}

void loop() {
  int valor = analogRead(SENSOR_PIN);
  int porcentaje = map(valor, 0, 4095, 0, 100);
  porcentaje = constrain(porcentaje, 0, 100);

  Serial.print("Nivel de agua (ADC): ");
  Serial.print(valor);
  Serial.print(" | Estimado: ");
  Serial.print(porcentaje);
  Serial.println("%");

  // Encender motor si está por debajo del umbral_bajo
  if (porcentaje < umbral_bajo && !motorEncendido) {
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(Relay_pin_2, HIGH); // Activar relay
    motorEncendido = true;
    Serial.println("⚠️ Nivel bajo: Motor encendido");
  }

  // Apagar motor solo si sube por encima del umbral_alto
  else if (porcentaje >= umbral_alto && motorEncendido) {
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(Relay_pin_2, HIGH); // Desactivar relay
    motorEncendido = false;
    Serial.println("✅ Nivel alto: Motor apagado");
  }

  // Enviar el porcentaje a Firebase
  if (Firebase.RTDB.setInt(&fbdo, "/nivel_agua", porcentaje)) {
    Serial.println("Dato enviado: " + String(porcentaje));
  } else {
    Serial.println("Error al enviar: " + fbdo.errorReason());
  }

  // Leer la orden desde Firebase
  if (Firebase.RTDB.getString(&fbdo, "/orden")) {
    orden = fbdo.stringData();
    if (orden == "encender" && !motorEncendido) {
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(Relay_pin_2, HIGH);
      motorEncendido = true;
      Serial.println("⚙️ Motor encendido desde web");
    }
    if (orden == "apagar" && motorEncendido) {
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(Relay_pin_2, HIGH);
      motorEncendido = false;
      Serial.println("🛑 Motor apagado desde web");
    }
  }

  delay(1000);
}