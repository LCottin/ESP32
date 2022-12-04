#include <Arduino.h>
#include <mutex>

using namespace std;

// Create a mutex
SemaphoreHandle_t mtx; // = xSemaphoreCreateMutex();

// Create a task
void task1(void *pvParameters)
{
    while (1)
    {
        // Lock the mutex
        if (xSemaphoreTake(mtx, portMAX_DELAY))
        {
            Serial.println("Task 1");

            // Unlock the mutex
            xSemaphoreGive(mtx);
        }

        // Wait 1 second
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void task2(void *pvParameters)
{
    while (1)
    {
        // Lock the mutex
        if (xSemaphoreTake(mtx, portMAX_DELAY))
        {
            Serial.println("Task 2");

            // Unlock the mutex
            xSemaphoreGive(mtx);
        }

        // Wait 1 second
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void setup()
{
    // Initialize serial
    Serial.begin(115200);
    Serial.println("Starting...");

    // Create a mutex
    mtx = xSemaphoreCreateMutex();

    // Create tasks
    xTaskCreate(task1, "Task 1", 1000, NULL, 1, NULL);
    xTaskCreate(task2, "Task 2", 1000, NULL, 1, NULL);
}

void loop()
{
    // Do something
    Serial.println("Loop");
    delay(1000);
}