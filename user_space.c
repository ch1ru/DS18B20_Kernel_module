#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define SENSOR_FILE "/sys/bus/w1/devices/28-xxxxxxxxxx/w1_slave"
#define PROC_FILE "/proc/temp_monitor"

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

int main() {
    float temp = 0.0;

    while (1) {
        temp = get_temp_from_sensor();
        if (temp == -1000) {
            printf("Failed to read temperature\n");
        } else {
            printf("Temperature: %.2f°C\n", temp);
            send_temp_to_kernel((int)temp);
        }
        sleep(2);
    }

    return 0;
}
