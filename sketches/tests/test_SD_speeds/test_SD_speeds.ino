/*
   SD Card | ESP32
      CS       SS
      DI       MOSI
      CLK      SCK
      D0       MISO
*/

#include "esp32-hal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "rom/ets_sys.h"
#include "esp32-hal-matrix.h"
#include "soc/gpio_sd_reg.h"
#include "soc/gpio_sd_struct.h"

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "io_funcs.h"

volatile SemaphoreHandle_t tasksSemaphore;
volatile SemaphoreHandle_t spMain;
volatile SemaphoreHandle_t spTask0;
volatile SemaphoreHandle_t spTask1;

int tests[] = {64, 128, 256, 512, 1024, 2048};
int repeats = 3;
int numTests;

float *resultsMain_openFiles;
float *resultsTask0_openFiles;
float *resultsTask1_openFiles;

float *resultsMain_closeFiles;
float *resultsTask0_closeFiles;
float *resultsTask1_closeFiles;

void setup() {
  numTests = sizeof(tests) / sizeof(tests[0]);

  resultsMain_openFiles = new float[numTests ];
  resultsTask0_openFiles = new float[numTests ];
  resultsTask1_openFiles = new float[numTests ];
  
  resultsMain_closeFiles = new float[numTests ];
  resultsTask0_closeFiles = new float[numTests ];
  resultsTask1_closeFiles = new float[numTests ];

  //delete [] resultsMain;

  //sync semaphore
  tasksSemaphore = xSemaphoreCreateBinary();
  spMain = xSemaphoreCreateBinary();
  spTask0 = xSemaphoreCreateBinary();
  spTask1 = xSemaphoreCreateBinary();

  Serial.begin(115200);
  delay(1000);

  Serial.println("loading SD card");
  if (!SD.begin(SS, SPI, 20000000)) {
    Serial.println("SD Card Mount Failed");
    return;
  };
  delay(1000);

  //write
  writeFile("/test.dat", 1024 * 1024);
  Serial.println("");

  //on main:
  runTests(resultsMain_openFiles,resultsMain_closeFiles, spMain);

  //on task (core 0):
  xTaskCreatePinnedToCore(
    taskFunc0,
    "taskFunc0",
    10000,
    NULL,
    8,
    NULL,
    0);

  //on task (core 1):
  xTaskCreatePinnedToCore(
    taskFunc1,
    "taskFunc1",
    10000,
    NULL,
    8,
    NULL,
    1);

  //results task
  xTaskCreate(
    taskResults,
    "resultsTask",
    10000,
    NULL,
    1,
    NULL);
}

void loop() {

  delay(10000);
  /*
    //take some CPU time
    volatile int i = 0;
    while (true) {
    i++;
    if (i == 0) {
      Serial.println("------------- BOOP");
    }
    taskYIELD ();
    }
  */
}

void taskFunc0(void * parameter) {
  Serial.println("------------------");
  Serial.println(" ON CORE 0");
  Serial.println("------------------");
  runTests(resultsTask0_openFiles,resultsTask0_closeFiles, spTask0);

  xSemaphoreGiveFromISR(tasksSemaphore, NULL);
  vTaskDelete( NULL );
}

void taskFunc1(void * parameter) {

  // -- wait until task 0 if finished
  while (!xSemaphoreTake(tasksSemaphore, 1))
    ;

  Serial.println("------------------");
  Serial.println(" ON CORE 1");
  Serial.println("------------------");
  runTests(resultsTask1_openFiles,resultsTask1_closeFiles, spTask1);

  vTaskDelete( NULL );
}

void taskResults(void * parameter) {

  while (!xSemaphoreTake(spMain, 1000))
    ;
  while (!xSemaphoreTake(spTask0, 1000))
    ;
  while (!xSemaphoreTake(spTask1, 1000))
    ;

  Serial.println("------------------");
  Serial.println(" RESULTS");
  Serial.println("------------------");

  for (int i = 0; i < numTests; i++) {

    Serial.printf("@buff %u\n", tests[i]);
    Serial.printf("opened files\n");
    Serial.printf("  %F KB/S (main)\n", resultsMain_openFiles[i]);
    Serial.printf("  %F KB/S (task0)\n", resultsTask0_openFiles[i]);
    Serial.printf("  %F KB/S (task1)\n", resultsTask1_openFiles[i]);
    
    Serial.println();
    Serial.printf("closed files\n");
    Serial.printf("  %F KB/S (main)\n", resultsMain_closeFiles[i]);
    Serial.printf("  %F KB/S (task0)\n", resultsTask0_closeFiles[i]);
    Serial.printf("  %F KB/S (task1)\n", resultsTask1_closeFiles[i]);
    
    Serial.println();    
  }

  vTaskDelete( NULL );
}


void runTests(float *results_openFiles,float *results_closeFiles, SemaphoreHandle_t semaphore) {

  Serial.print("on core ");
  Serial.println(xPortGetCoreID());
  Serial.println("");

  for (int i = 0; i < numTests; i++) {
    float speedKBS_openFiles = 0;
    float speedKBS_closeFiles = 0;
    
    for (int r = 0; r < repeats ; r++) {
      //reads
      speedKBS_openFiles += readFile("/test.dat", tests[i], true);
      vTaskDelay(1);
      
      speedKBS_closeFiles += readFile("/test.dat", tests[i], false);
      vTaskDelay(1);
    }

    //avg:
    if (results_openFiles) {
      results_openFiles[i] = speedKBS_openFiles / repeats;
    }

    if (results_closeFiles) {
      results_closeFiles[i] = speedKBS_closeFiles / repeats;
    }
  }

  //
  if (semaphore)
    xSemaphoreGiveFromISR(semaphore, NULL);
}


