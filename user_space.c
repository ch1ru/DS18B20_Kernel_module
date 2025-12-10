#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pigpio.h>

#define SENSOR_FILE "/sys/bus/w1/devices/28-000010ab45bb/w1_slave"
#define PROC_FILE "/proc/temp_monitor"
#define RED_LED_PIN 17
#define YELLOW_LED_PIN 27


float get_temp_from_sensor() {
    FILE *fp;
    char line[128];
    float temp_c = 0.0;

    fp = fopen(SENSOR_FILE, "r");
    if (fp == NULL) {
        printf("Error: could not open sensor file\n");
        return -1000;
    }

    while (fgets(line, sizeof(line), fp)) {
        char *pos = strstr(line, "t=");
        if (pos != NULL) {
            temp_c = atof(pos + 2) / 1000.0;
            break;
        }
    }

    fclose(fp);
    return temp_c;
}

void send_temp_to_kernel(int t) {
    FILE *fp = fopen(PROC_FILE, "w");
    if (fp == NULL) {
        printf("Error: could not open /proc/temp_monitor\n");
        return;
    }
    fprintf(fp, "%d", t);
    fclose(fp);
}


int main(int argc, char *argv[]) {

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <lower_temp> <upper_temp>\n", argv[0]);
        return 1;
    }

    float lower_temp = atof(argv[1]);
    float upper_temp = atof(argv[2]);

    float temp = 0.0;

    if (gpioInitialise() < 0) {
        fprintf(stderr, "pigpio initialization failed\n");
        return 1;
    }

    gpioSetMode(RED_LED_PIN, PI_OUTPUT);
    gpioSetMode(YELLOW_LED_PIN, PI_OUTPUT);

    while (1) {
        temp = get_temp_from_sensor();
        if (temp == -1000) {
            printf("Failed to read temperature\n");
        } else {
            printf("Temperature: %.2f°C\n", temp);
            send_temp_to_kernel((int)temp);
            if (temp > upper_temp) {
                gpioWrite(RED_LED_PIN, 1);
                gpioWrite(YELLOW_LED_PIN, 0);
            } else if (temp < lower_temp) {
                gpioWrite(YELLOW_LED_PIN, 1);
                gpioWrite(RED_LED_PIN, 0);
            }
            else {
                gpioWrite(RED_LED_PIN, 0);
                gpioWrite(YELLOW_LED_PIN, 0);
           }
        }
        sleep(1);
    }

    gpioWrite(RED_LED_PIN, 0);
    gpioWrite(YELLOW_LED_PIN, 0);
    gpioTerminate();

    return 0;
}
