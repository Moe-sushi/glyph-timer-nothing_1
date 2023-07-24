#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#define MAX_BRIGHTNESS (4095 - 1000)
#define SINGLE_LED_PATH "/sys/class/leds/aw210xx_led/single_led_br"
// Leds to enable.
char *leds[] = {"16", "13", "11", "9", "12", "10", "14", "15", "8"};
// Enable leds.
void write_leds(int fd, int enabled_leds, int brightness)
{
  char to_write[16] = {'\0'};
  for (int i = 0; i <= enabled_leds; i++)
  {
    // Enable leds to enable.
    sprintf(to_write, "%s%s%d", leds[i], " ", brightness);
    write(fd, to_write, sizeof(to_write));
    // Disable other leds.
    if (i == enabled_leds)
    {
      for (int j = i + 1; j <= 8; j++)
      {
        sprintf(to_write, "%s%s", leds[j], " 0");
        write(fd, to_write, sizeof(to_write));
      }
    }
  }
}
// Enable single led.
void write_single_led(int fd, int led, int brightness)
{
  char to_write[16] = {'\0'};
  sprintf(to_write, "%s%s%d", leds[led], " ", brightness);
  write(fd, to_write, sizeof(to_write));
}
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
      fprintf(stderr, "\033[31mUsage:\n  gtimer -h [hours] -m [minutes] -s [seconds]\033[0m\n");
      exit(1);
    }
  }
  // Check if the value is valid.
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
  // To enable leds.
  int enabled_leds = 8;
  // To dim single led.
  int led_now = 8;
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
  write_leds(fd, 8, MAX_BRIGHTNESS);
  // prepare to count down.
  time_t time_old = 0;
  time(&time_old);
  time_t time_now = 0;
  float total_seconds = seconds;
  float past_seconds = 0;
  float time_ratio = 0;
  if (minutes != 0)
  {
    total_seconds += minutes * 60;
  }
  if (hours != 0)
  {
    total_seconds += hours * 3600;
  }
  // To check if time has changed.
  float time_bk = 0;
  // Timer clock.
  while (hours > 0 || minutes > 0 || seconds > 0)
  {
    time(&time_now);
    past_seconds = time_now - time_old;
    if (time_bk != past_seconds)
    {
      // Count down..
      time_bk = past_seconds;
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
      // Control leds.
      time_ratio = (1 - (past_seconds / total_seconds)) * 100;
      printf("%f", time_ratio);
    }
  }
  // END of time.
  // Disable all leds.
  write_leds(fd, 8, 0);
  // Vibration and blinking.
  for (int i = 0; i < 10; i++)
  {
    printf("\a");
    fflush(stdout);
    usleep(200000);
    write_single_led(fd, 0, MAX_BRIGHTNESS);
    usleep(200000);
    write_single_led(fd, 0, 0);
  }
  // Close the file.
  // It's a good habit, but in fact it's not really necessary.
  close(fd);
  return 0;
}
