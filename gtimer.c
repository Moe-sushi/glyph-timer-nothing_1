#include <stdio.h>
#include <time.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#define MAX_BRIGHTNESS (4095 - 1000)
#define SINGLE_LED_PATH "/sys/class/leds/aw210xx_led/single_led_br"
int main(int argc, char *argv[])
{
    // Get the time to count down.
    int hours = 0, minutes = 0, seconds = 0;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0)
        {
            i++;
            hours = atoi(argv[i]);
        }
        else if (strcmp(argv[i], "-m") == 0)
        {
            i++;
            minutes = atoi(argv[i]);
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            i++;
            seconds = atoi(argv[i]);
        }
        else
        {
            fprintf(stderr, "\033[31mInvalid argument\033[0m\n");
            fprintf(stderr, "\033[31mUsage:\033[0m\ngtimer -h [hours] -m [minutes] -s [seconds]\n");
            exit(1);
        }
    }
    if (seconds >= 60)
    {
        fprintf(stderr, "\033[31mSeconds should be less than 60\033[0m\n");
        exit(1);
    }
    if (minutes >= 60)
    {
        fprintf(stderr, "\033[31mMinutes should be less than 60\033[0m\n");
        exit(1);
    }
    // Leds to enable.
    char *leds[] = {"16", "13", "11", "9", "12", "10", "14", "15", "8"};
    int enabled_leds = 9;
    // To write to SINGLE_LED_PATH.
    char to_write[16] = {'\0'};
    // To set led brightness.
    int brightness = MAX_BRIGHTNESS;
    // Check if we are running with root provilages.
    if (getuid() != 0)
    {
        fprintf(stderr, "\033[31mThis program must be run as root.\033[0m\n");
        exit(1);
    }
    // Open the file to write.
    int fd = open(SINGLE_LED_PATH, O_WRONLY);
    if (fd == -1)
    {
        fprintf(stderr, "\033[31mFailed to open %s\033[0m\n", SINGLE_LED_PATH);
        fprintf(stderr, "\033[31mMaybe you are not using Phone(1)\033[0m");
        exit(1);
    }
    // Initialize leds.
    for (int i = 0; i < enabled_leds; i++)
    {
        sprintf(to_write, "%s%s%d", leds[i], " ", MAX_BRIGHTNESS);
        write(fd, to_write, sizeof(to_write));
    }
    // prepare to count down.
    time_t time_old = 0;
    time(&time_old);
    time_t time_now = 0;
    int total_seconds = seconds;
    int past_seconds = 0;
    if (minutes != 0)
    {
        total_seconds += minutes * 60;
    }
    if (hours != 0)
    {
        total_seconds += hours * 3600;
    }
    // To check if time has changed.
    int bk = 0;
    // Count down.
    while (hours > 0 || minutes > 0 || seconds > 0)
    {
        time(&time_now);
        past_seconds = time_now - time_old;
        if (bk != past_seconds)
        {
            bk = past_seconds;
            if (seconds > 0)
            {
                seconds--;
            }
            else if (minutes > 0)
            {
                minutes--;
                seconds = 59;
            }
            else if (hours > 0)
            {
                hours--;
                minutes = 59;
                seconds = 59;
            }
            else
            {
                break;
            }
            printf("%02d H %02d M %02d S\n", hours, minutes, seconds);
        }
    }
    // END of time.
    // Disable all leds.
    for (int i = 0; i < enabled_leds; i++)
    {
        sprintf(to_write, "%s%s%d", leds[i], " ", 0);
        write(fd, to_write, sizeof(to_write));
    }
    // Vibration.
    for (int i = 0; i < 10; i++)
    {
        printf("\a");
        fflush(stdout);
        usleep(500000);
    }
    close(fd);
    return 0;
}
