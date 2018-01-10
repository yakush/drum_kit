void setup(){
  size(800,800);
  
  int w2=width/2;
  int h2=height/2;
  
  float xx=0;
  float yy=0;
  stroke(0);
  for(int i=-32700;i<+32700;i++){    
    float x = w2+w2*(float)i/32700;
    float y = h2-h2*sig((float)i/10000);
    line(xx,yy,x,y);
    
    xx=x;
    yy=y;
  }
  
  xx=0;
  yy=0;
  stroke(color(100,200,300));
  strokeWeight(3);
  
  for(int i=-1024;i<+1024;i++){    
    float x = w2+w2*(float)i/1024;
    float y = h2-h2*(float)sig_i32((short)i)/1024/2;
    line(xx,yy,x,y);
    
    xx=x;
    yy=y;
  }
  
  //for(int i=-MAX_INT;i<MAX_INT;i++){
    
  //}
}

void draw(){
  
}



float sig(float x) {  
    return (1/(1 + exp(-x)));    
}

int sig_i32(int x) {
    if (x > 1024)  return 1024;
    if (x < -1024)  return -1024;
    return ( (3*x) - ((x*x*x)>>20) ) >> 1;
}

short sig_i16(short x) {
    int xx=x;
    if (x > 32) return 32;
    if (x < -32) return -32;
    return (short)(((96 * xx) - ((xx * xx * xx) >> 5)) >> 6);
}