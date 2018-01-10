import processing.serial.*;

Serial port;

boolean doSend = false;

byte b[] = null;

void setup() {
  background(151);
  if (doSend) {
    String portName = Serial.list()[0]; //change the 0 to a 1 or 2 etc. to match your port
    port = new Serial(this, portName, 9600);
  }
  selectInput("Select a file", "fileSelected");
  size(900, 600);
  stroke(0);
}

int xx = 0;
int yy = 0;
int pos = 0;
boolean is16Bit=true;

void draw() {

  for (int frame = 0; frame <1; frame++){
    if (b!=null) {
      int val; 
      if (is16Bit) {
        //little endian
        char hi = char(b[pos+1] & 0xff);
        char lo = char(b[pos+0] & 0xff);
        //hi^=0x10000000; //invert hi to get signed
        
        hi^=(1<<7);//invert hi to get unsigned
        val = (hi<<8)+lo;
        pos+=2;
      } else {
        val = (b[pos] & 0xff)<<8;
        pos+=1;
      }

      if (pos>=b.length) {
        pos=0;
        xx=0;
        yy=0;
        background(151);
        stroke(random(255));
      }

      //int x = (int)(width * 1f*pos/b.length);
      int x = xx+1;
      int y = height/2+(int)(height/2 * 1f*(val-32768)/65536);
      line(xx, yy, x, y);

      xx=x;
      yy=y;
      if (xx>=width) {
        xx=0;
        background(151);
        stroke(random(255));
      }
    }
}
}
void serialEvent (Serial thePort)
{
  if (doSend) {
    String str = thePort.readStringUntil('\n');
    if (str!=null && str.length()>0)
      print(">> "+ str);
  }
}

void fileSelected(File selection) {
  if (selection == null) {
    println("cancled");
  } else {
    println("selected: " + selection.getAbsolutePath());
    sendFile(selection);
  }
}

void sendFile(File file) {

  int BUFFSIZE=32;

  b = loadBytes(file.getAbsolutePath());
  byte buff[] = new byte [BUFFSIZE];
  int buffPos=0;


  if (doSend) {  


    //header
    port.write("download\n");
    port.write("name.raw\n");
    port.write(""+b.length+"\n");

    int lastProc=-1;
    long sent =0;

    for (int i = 0; i < b.length; i++) {

      //dump buff
      if (buffPos==BUFFSIZE) {
        port.write(buff);    
        sent+=BUFFSIZE;
        buffPos=0;
        delay(10);

        int procc = (int)(100*sent/b.length);
        if (procc!=lastProc) {
          println(procc + "% " +sent + " / "+b.length);
          lastProc=procc;
        }
      }

      int a = b[i] & 0xff; // unsinged => singed
      buff[buffPos]=(byte)a;
      buffPos++;
    } 

    //dump buff
    if (buffPos>0) {
      byte [] rest = new byte[buffPos];
      arrayCopy(buff, rest, buffPos);
      port.write(rest);    
      sent+=buffPos;

      int procc = (int)(100*sent/b.length);
      if (procc!=lastProc) {
        println(procc + "% " +sent + " / "+b.length);
        lastProc=procc;
      }
    }
    println(100 + "% " +sent + " / "+b.length);
    println("done");
  }
}