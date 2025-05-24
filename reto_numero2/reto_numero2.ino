#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#define botonPausa 2

#define ledVerde 12   // Lavado
#define ledAzul 13    // Enjuague
#define ledRojo 14    // Centrifugado

enum Estado { EST_LAVADO, EST_ENJUAGUE, EST_CENTRIFUGADO };
volatile Estado estadoActual = EST_LAVADO;

SemaphoreHandle_t semLavado;
SemaphoreHandle_t semEnjuague;
SemaphoreHandle_t semCentrifugado;

volatile bool pausa = false;

// ----------------------------------

void esperarSinPausa(int tiempo_ms) {
  int tiempoTranscurrido = 0;
  while (tiempoTranscurrido < tiempo_ms) {
    if (pausa) {
      // Mostrar pausa con todos los LEDs encendidos
      digitalWrite(ledVerde, HIGH);
      digitalWrite(ledAzul, HIGH);
      digitalWrite(ledRojo, HIGH);
      // Esperar mientras siga en pausa
      while (pausa) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
    tiempoTranscurrido += 100;
  }
}

// ----------------------------------

void TareaLavado(void *pvParameters) {
  for (;;) {
    xSemaphoreTake(semLavado, portMAX_DELAY);
    estadoActual = EST_LAVADO;
    Serial.println("Estado: Lavado");
    digitalWrite(ledVerde, HIGH);
    digitalWrite(ledAzul, LOW);
    digitalWrite(ledRojo, LOW);
    esperarSinPausa(5000);
    xSemaphoreGive(semEnjuague);
  }
}

void TareaEnjuague(void *pvParameters) {
  for (;;) {
    xSemaphoreTake(semEnjuague, portMAX_DELAY);
    estadoActual = EST_ENJUAGUE;
    Serial.println("Estado: Enjuague");
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledAzul, HIGH);
    digitalWrite(ledRojo, LOW);
    esperarSinPausa(5000);
    xSemaphoreGive(semCentrifugado);
  }
}

void TareaCentrifugado(void *pvParameters) {
  for (;;) {
    xSemaphoreTake(semCentrifugado, portMAX_DELAY);
    estadoActual = EST_CENTRIFUGADO;
    Serial.println("Estado: Centrifugado");
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledAzul, LOW);
    digitalWrite(ledRojo, HIGH);
    esperarSinPausa(5000);
    xSemaphoreGive(semLavado);
  }
}

void TareaPausa(void *pvParameters) {
  for (;;) {
    bool botonPresionado = (digitalRead(botonPausa) == LOW);
    pausa = botonPresionado;
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

// ----------------------------------

void setup() {
  Serial.begin(115200);

  pinMode(ledVerde, OUTPUT);
  pinMode(ledAzul, OUTPUT);
  pinMode(ledRojo, OUTPUT);
  pinMode(botonPausa, INPUT_PULLUP);  // Botón activo en LOW

  semLavado = xSemaphoreCreateBinary();
  semEnjuague = xSemaphoreCreateBinary();
  semCentrifugado = xSemaphoreCreateBinary();

  xSemaphoreGive(semLavado);  // Estado inicial

  xTaskCreatePinnedToCore(TareaLavado, "Lavado", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TareaEnjuague, "Enjuague", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TareaCentrifugado, "Centrifugado", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TareaPausa, "Pausa", 1024, NULL, 2, NULL, 1);
}

void loop() {
  // No se usa con FreeRTOS
}
