/*
Embedded Challenge Fall 2022 Term Project
Video Demo: https://www.youtube.com/watch?v=E0gm5oW6I4s

Team Members: 
Tianjian Ni: tn2151@nyu.edu / Jerry Lin: yl9851@nyu.edu / Beinan Wang:bw2555@nyu.edu / Linghang Wang: lw3150@nyu.edu

Abstract:
This project aims to provide a potential solution to help with the patients with respiratory problems 
by detecting their breath every 10 seconds. With detailed investigation and research, 
we adopted STM32F4 microcontroller and Adafruit SGP30 air quality sensor as the main body 
and successfully accomplished the breath detection challenge.

Connection Type (I2C)                MCU         SGP30 Sensor 
Power Pin                             5V             VIN
Common Ground for power and logic    GND             GND
SCL                                  PA8             SCL
SDA                                  PC9             SDA
*/

#include <mbed.h>
#include "drivers/LCD_DISCO_F429ZI.h" // Import library for LCD screen
LCD_DISCO_F429ZI lcd; // Initialize lcd 

// Initialize I2C connection
// SDA Data line connecting to port PC9 on MCU
// SCL Clock line connecting to port PA8 on MCU
I2C i2c(PC_9, PA_8); 

const int address = 0x58 << 1; //8 bit i2c address
//read/write will automatically set last bit to 0 and 1

//Timer t used to count 10 seconds of not breathing
Timer t;
// Timer t2 used to count 5 seconds of reseting after detecting of not breathing 
Timer t2;

int main() {
  char cmd[2]; // command sends to SGP 30 sensor
  char data[6];	// must read all 6 byte or the value will always be default
  char co2_reading[4];  // string str used to hold float numbers for CO2 reading print on LCD
  char timer_reading[4]; // string str used to hold float numbers for Timer t reading print on LCD

  // Initialize LCD screen
  lcd.Clear(LCD_COLOR_WHITE);
  lcd.SetBackColor(LCD_COLOR_WHITE);
  lcd.SetTextColor(LCD_COLOR_BLACK);
  lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"start to", CENTER_MODE);
  lcd.DisplayStringAt(0, LINE(6), (uint8_t *)"breathe", CENTER_MODE);
  thread_sleep_for(3000); // program starts in 3 seconds
  lcd.ClearStringLine(5);
  lcd.ClearStringLine(6);

  while(1){
    t.start(); // Timer t starts
    cmd[0] = 0x20;    // first 8 bit of command
    cmd[1] = 0x08;    // last 8 bit of command
    i2c.write(address, cmd, 2);   // write address
    thread_sleep_for(1000);   // wait for measurement – at least 1s
    if(0 != i2c.read(address, data, 6, true)){ // Print error if reading fails
      printf("Error");
      continue;
    }
    // conversion from unsigned char* to float
    float co2 = (float)((uint16_t)data[0]<<8 | (uint16_t)data[1]);

    // Print CO2 reading from sensor
    sprintf(co2_reading, "%f", co2); 
    lcd.DisplayStringAt(0, LINE(4), (uint8_t *) "CO2 reading", CENTER_MODE);
    lcd.DisplayStringAt(0, LINE(5), (uint8_t *) co2_reading, CENTER_MODE);

    // If CO2 reading larger than 500, means detect breathing, reset timer t 
    if (co2 > 500)
    {
      lcd.SetBackColor(LCD_COLOR_GREEN);
      lcd.DisplayStringAt(0, LINE(9), (uint8_t *)"breathe detected", CENTER_MODE);
      lcd.DisplayStringAt(0, LINE(10), (uint8_t *)"restart counting", CENTER_MODE);
      // One round of breathing takes 4 second, let it cool down 4 seconds
      // then continue detecting
      thread_sleep_for(4000);
      lcd.SetBackColor(LCD_COLOR_WHITE);
      lcd.Clear(LCD_COLOR_WHITE);
      lcd.ClearStringLine(9);
      lcd.ClearStringLine(10);
      // timer t resets
      t.reset();
      t.start();
    }
    t.stop();

    // Print the timer t reading
    sprintf(timer_reading, "%.0f", t.read()); 
    lcd.DisplayStringAt(0, LINE(6), (uint8_t *) "Timer reading", CENTER_MODE);
    lcd.DisplayStringAt(0, LINE(7), (uint8_t *) timer_reading, CENTER_MODE);

    // if detecting 10 seconds of not breathing
    // LED will flash and the LCD screen goes red
    if (t.read() > 10)
    {
      lcd.Clear(LCD_COLOR_RED);
      lcd.SetBackColor(LCD_COLOR_RED);
      lcd.DisplayStringAt(0, LINE(8), (uint8_t *) "No breathe!!!", CENTER_MODE);

      DigitalOut led1(LED1);
      DigitalOut led2(LED2);
      // timer t2 to count led lights flash 5 seconds
      t2.start();
      while(1){
        led1 = 1;
        led2 = 1;
        thread_sleep_for(500);
        led1 = 0;
        led2 = 0;
        thread_sleep_for(500);
        t2.stop();
        // Reset to next round of detecting after 5 seconds
        if (t2.read() > 5)
        {
          // both timer t and t2 resets to enter new detecting 
          t.reset();
          t2.reset();
          lcd.SetBackColor(LCD_COLOR_WHITE);
          lcd.Clear(LCD_COLOR_WHITE);
          break;
        }
        t2.start();
        }
    }
    t.start();
  }
}