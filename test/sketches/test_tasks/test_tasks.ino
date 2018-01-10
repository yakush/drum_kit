#include <esp_task_wdt.h>

long counter = 0;

void setup() {
 
  Serial.begin(9600);
  delay(1000);
  //  xTaskCreate(
  //    taskOne,          /* Task function. */
  //    "TaskOne",        /* String with name of task. */
  //    10000,            /* Stack size in words. */
  //    NULL,             /* Parameter passed as input of the task */
  //    1,                /* Priority of the task. */
  //    NULL);            /* Task handle. */
  //
  //    xTaskCreate(
  //    taskTwo,          /* Task function. */
  //    "TaskTwo",        /* String with name of task. */
  //    10000,            /* Stack size in words. */
  //    NULL,             /* Parameter passed as input of the task */
  //    1,                /* Priority of the task. */
  //    NULL);            /* Task handle. */

  xTaskCreate(
    taskCounter,          /* Task function. */
    "taskCounter",        /* String with name of task. */
    10000,            /* Stack size in words. */
    NULL,             /* Parameter passed as input of the task */
    1,                /* Priority of the task. */
    NULL);            /* Task handle. */
}

void loop() {
  
  Serial.print("loop ");
  Serial.println(counter);
  delay(1000);

}

// ---------------------------


void taskCounter( void * parameter ) 
{
  while(true){
    counter++;
    millis();
    //esp_task_wdt_feed();
  }
}

void taskOne( void * parameter )
{

  for ( int i = 0; i < 100; i++ ) {

    Serial.println("Hello from task 1");
    delay(100);
  }

  Serial.println("Ending task 1");
  vTaskDelete( NULL );

}

void taskTwo( void * parameter )
{

  for ( int i = 0; i < 100; i++ ) {

    Serial.println("222222");
    delay(100);
  }

  Serial.println("Ending task 2");
  vTaskDelete( NULL );

}
