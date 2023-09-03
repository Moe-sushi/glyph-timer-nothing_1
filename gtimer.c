// SPDX-License-Identifier: MIT
/*
 *
 * This file is part of glyph-timer-nothing_1, with ABSOLUTELY NO WARRANTY.
 *
 * MIT License
 *
 * Copyright (c) 2023 Moe-hacker
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *
 */
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#define MAX_BRIGHTNESS (4095 - 1000)
#define SINGLE_LED_PATH "/sys/class/leds/aw210xx_led/single_led_br"
// Leds to control.
// Total 9 leds.
char *leds[] = {"16", "13", "11", "9", "12", "10", "14", "15", "8"};
// Enable leds.
void enable_leds(int fd, int enabled_leds)
{
  char to_write[16] = {'\0'};
  for (int i = 0; i <= enabled_leds; i++)
  {
    // Enable leds to enable.
    sprintf(to_write, "%s%s%d", leds[i], " ", MAX_BRIGHTNESS);
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
// Disable all leds.
void disable_all_leds(int fd)
{
  char to_write[16] = {'\0'};
  // Total 9 leds.
  for (int i = 0; i <= 8; i++)
  {
    sprintf(to_write, "%s%s", leds[i], " 0");
    write(fd, to_write, sizeof(to_write));
  }
}
// Set single led brightness.
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
  enable_leds(fd, 8);
  // prepare to count down.
  // To get the time.
  time_t time_old = 0;
  time(&time_old);
  time_t time_now = 0;
  // Get total seconds.
  float total_seconds = seconds;
  if (minutes != 0)
  {
    total_seconds += minutes * 60;
  }
  if (hours != 0)
  {
    total_seconds += hours * 3600;
  }
  // For glyph.
  float past_seconds = 0;
  float time_ratio = 0;
  // How many time has changed.
  float ratio_bk = 0;
  // Timer clock.
  while (hours > 0 || minutes > 0 || seconds > 0)
  {
    time(&time_now);
    past_seconds = time_now - time_old;
    // Count down.
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
    printf("\033[0G\033[1;38;2;152;245;225m%02d \033[1;38;2;254;228;208mH \033[1;38;2;152;245;225m%02d \033[1;38;2;254;228;208mM \033[1;38;2;152;245;225m%02d \033[1;38;2;254;228;208mS", hours, minutes, seconds);
    fflush(stdout);
    // Control leds.
    time_ratio = (past_seconds / total_seconds) * 100;
    if (enabled_leds > 0)
    {
      while (time_ratio - ratio_bk >= 10)
      {
        if (enabled_leds > 0)
        {
          write_single_led(fd, led_now, 0);
          ratio_bk += 10;
          led_now--;
          enabled_leds--;
          enable_leds(fd, enabled_leds);
        }
        else
        {
          break;
        }
      }
    }
    int j = 1;
    for (int i = 0; i < 9; i++)
    {
      if ((time_ratio - ratio_bk) > j)
      {
        j++;
      }
      else
      {
        brightness = MAX_BRIGHTNESS - (309 * i);
        write_single_led(fd, led_now, brightness);
        break;
      }
    }
    sleep(1);
  }
  // END of time.
  printf("\033[0m\n");
  // Disable all leds.
  disable_all_leds(fd);
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
