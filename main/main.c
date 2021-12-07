#include <stdio.h>
#include "ccs811.h"

#define I2C_BUS       0
#define I2C_SCL_PIN   22
#define I2C_SDA_PIN   21
#define I2C_FREQ      I2C_FREQ_100K

#define STACK_SIZE 8096 //configMINIMAL_STACK_SIZE

#define avergingCount 10

static ccs811_sensor_t* sensor;

void task_mainLoop(void *pvParameters)
{
    uint16_t tvoc;
    uint16_t eco2;
    uint32_t sampleSum = 0;
    uint32_t timer = 0;
    uint16_t cnt = 0;

    printf("Starting task_mainLoop\n");

    TickType_t last_wakeup = xTaskGetTickCount();

    while (1)
    {
        if (ccs811_get_results (sensor, &tvoc, &eco2, 0, 0))
        {
            sampleSum += eco2;
            timer++;

            if (++cnt == avergingCount) {
                
                printf("%d %d\n", timer, sampleSum / avergingCount);
                cnt = 0;
                sampleSum = 0;
            }
        }   
            // printf("%.3f CCS811 Sensor periodic: TVOC %d ppb, eCO2 %d ppm\n",
            //        (double)sdk_system_get_time()*1e-3, tvoc, eco2);

        vTaskDelayUntil(&last_wakeup, 1000 / portTICK_PERIOD_MS);
    }
}

void initSensor()
{
    i2c_init (I2C_BUS, I2C_SCL_PIN, I2C_SDA_PIN, I2C_FREQ);
    i2c_set_clock_stretch (I2C_BUS, CCS811_I2C_CLOCK_STRETCH);
    sensor = ccs811_init_sensor (I2C_BUS, CCS811_I2C_ADDRESS_1);
    if (sensor)
        printf("CCS811 sensor Initialized succesfully \n");
    else
        printf("Could not initialize CCS811 sensor\n");
}

void app_main(void)
{
    printf("Starting app... \n");

    initSensor();

    if (sensor) {

        xTaskCreate(task_mainLoop, "task_pollSensor", STACK_SIZE, NULL, 2, NULL);
        ccs811_set_mode (sensor, ccs811_mode_1s);
    }
}
