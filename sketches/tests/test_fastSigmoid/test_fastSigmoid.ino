int32_t fastSigmoid(int32_t x32) {
  if (x32>=  32767) return  32767;
  if (x32<= -32767) return -32768;

  int64_t x =x32;
  return ( ((x*3)<<14) - ((x*x*x)>>16) ) >>15;
}

void setup() {
  
  Serial.begin(115200);
  delay(2000);
  
  int numTests = 1000000;
  
  long t0=millis();  
  volatile int32_t y=0;
  
  ///////////////////////////////////////////
  
  Serial.println ("test2 32bit");
  t0=millis();    
  for (int i =0;i<numTests; i++){
    int32_t x=i/50;
    y= ((x*3)<<14-(x*x*x)>>16)>>15;
  }
  Serial.println(y);
  Serial.printf ("%F uSec per calc\n\n" , (float)1000*(millis()-t0)/numTests);
  
  ///////////////////////////////////////////
  
  Serial.println ("test3 -64bit");
  t0=millis();    
  for (int i =0;i<numTests; i++){    
    y = fastSigmoid(i/50);
  }
  Serial.println(y);
  Serial.printf ("%F uSec per calc\n\n" , (float)1000*(millis()-t0)/numTests);  

  ///////////////////////////////////////////
  
  Serial.println ("test1 -float");
  for (int i =0;i<numTests; i++){
    float x=i/50;
    y= (float)64000*(1/(1+exp(-x/10000))-0.5);
  }
  Serial.println(y);
  Serial.printf ("%F uSec per calc\n\n" , (float)1000*(millis()-t0)/numTests);
  
}

void loop() {
  // put your main code here, to run repeatedly:

}


