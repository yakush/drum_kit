#include "esp32-hal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "rom/ets_sys.h"
#include "esp32-hal-matrix.h"
#include "soc/gpio_sd_reg.h"
#include "soc/gpio_sd_struct.h"

#include "esp_task_wdt.h"

volatile long countTimer;
volatile long countTask;

hw_timer_t * timer1 = NULL;
hw_timer_t * timer2 = NULL;

void setup() {
  Serial.begin(9600);
  setupSigma();

  xTaskCreate(
    taskCounter,          // Task function.
    "taskCounter",        // String with name of task.
    10000,            // Stack size in words.
    NULL,             // Parameter passed as input of the task
    1,                // Priority of the task.
    NULL);            // Task handle.


  initTimerInterrupts();

}

void loop() {
  static long lastCountTimer;
  static long lastCountTask;
  static long lastReport = millis();
  static long now;

  //  esp_task_wdt_feed();
  //  esp_task_wdt_delete();

  while (true) {
    //esp_task_wdt_feed();


    now = millis();
    if (now > lastReport + 1000) {
      lastReport = now;

      Serial.print(countTimer - lastCountTimer);
      Serial.print(" ");
      Serial.print(countTask - lastCountTask);
      Serial.println("");
      lastCountTimer = countTimer;
      lastCountTask = countTask;
    }
    //taskYIELD();
    //micros();
  }
}

// -------------------------------

void IRAM_ATTR onTimer1() {
  //esp_task_wdt_feed();
  handleTask();
  countTimer++;
}
void IRAM_ATTR onTimer2() {

}

void initTimerInterrupts() {
  /* 1 tick take 1/(80MHZ/80) = 1us so we set divider 80 and count up */
  timer1 = timerBegin(0, 2, true);
  timerAttachInterrupt(timer1, &onTimer1, true);
  timerAlarmWrite(timer1, 40000000 / 96200, true); // 20 uSecs = 96khz
  timerAlarmEnable(timer1);

  timer2 = timerBegin(1, 80, true);
  timerAttachInterrupt(timer2, &onTimer2, true);
  timerAlarmWrite(timer2, 1000000, true);
  timerAlarmEnable(timer2);
}

// -------------------------------
void taskCounter( void * parameter )
{
  //fixed freq.
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 1; // 250 / portTICK_PERIOD_MS
  xLastWakeTime = xTaskGetTickCount();

  while (true) {
    //esp_task_wdt_feed();
    //micros();
    //taskYIELD();
    //handleTask();
    //vTaskDelayUntil( &xLastWakeTime, xFrequency );
    vTaskDelay( 1 );


    countTask++;
  }
}

void handleTask() {
  static int i = 0;

  i++;
  if (i == 255)
    i = 0;

  writeSigma(i);


  //  esp_task_wdt_feed();
  //  taskYIELD();
}

// -------------------------------

#define channel_1 0
#define pin_1 18

#define channel_2 1
#define pin_2 19

void setupSigma() {
  sigmaDeltaSetup(channel_1, 312500); //==>20 MHz
  sigmaDeltaSetup(channel_2, 312500); //==>20 MHz
  sigmaDeltaAttachPin(channel_1, pin_1);
  sigmaDeltaAttachPin(channel_2, pin_2);
}

void writeSigma(int val) {

  //sigmaDeltaWrite(channel_1, val);
  //sigmaDeltaWrite(channel_2, val);

  // !!! DANGER !!! - write without mutex lock:
  SIGMADELTA.channel[channel_1].duty = 128 + val; //128-380 (255 range)
  SIGMADELTA.channel[channel_2].duty = 128 + val; //128-380 (255 range)


}

