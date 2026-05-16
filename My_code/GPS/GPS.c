#include "GPS.h"


char gps_buff[100];


void Get_GPS(char *str)
{
    if (strncmp(str, "$GPRMC", 6) == 0)
    {
        char* token;
        char* fields[12];
        uint8_t i = 0;

        token = strtok(str, ",");
        while (token && i < 12)
        {
            fields[i++] = token;
            token = strtok(NULL, ",");
        }
    }
}
