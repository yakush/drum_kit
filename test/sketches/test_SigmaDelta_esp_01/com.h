#ifndef COM_H_
#define COM_H_

#include <string.h>

#define COM_MSG_HELP              0
#define COM_MSG_NOTE_ON           1
#define COM_MSG_NOTE_OFF          2
#define COM_MSG_8_BIT             3
#define COM_MSG_16_BIT            4
#define COM_MSG_PLAY_BUFF         5
#define COM_MSG_PLAY_CALC         6
#define COM_MSG_WRITE_SPEED_HIGH  7
#define COM_MSG_WRITE_SPEED_LOW   8

#define LENGTH_COM_MSG 9

String  help [] ={
  "#define COM_MSG_HELP             0" ,
  "#define COM_MSG_NOTE_ON          1" ,
  "#define COM_MSG_NOTE_OFF         2" ,
  "#define COM_MSG_8_BIT            3" ,
  "#define COM_MSG_16_BIT           4" ,
  "#define COM_MSG_PLAY_BUFF        5" ,
  "#define COM_MSG_PLAY_CALC        6" ,
  "#define COM_MSG_WRITE_SPEED_HIGH 7",
  "#define COM_MSG_WRITE_SPEED_LOW  8",
};


typedef struct {
  int msg = 0;
  int freq = 0;
  int volume = 0;
} comMessage;

class Com
{
  public:

    Com() {
    }

    void begin(void (*callback)(comMessage), void (*callbackOverflow)(void)) {
      _callbackMessage = callback;
      _callbackOverflow = callbackOverflow;
    }

    void read() {
      _handleRead();
    }

  private:

    void (*_callbackMessage)(comMessage);
    void (*_callbackOverflow) (void);

    static const char END_LINE = '\n';
    char *DELIMITER = " ,\r\n";
    static const int _maxBuffSize = 64;
    char _buff[_maxBuffSize];

    void _handleRead() {
      static int pos = 0;
      static bool overflow = false;

      if (Serial.available()) {
        char ch = Serial.read();

        //while in over flow - - wait for new line
        if (overflow) {
          if (ch == END_LINE) {
            overflow = false;
            pos = 0;
          }
          return;
        }

        //detect overflow
        if (pos == _maxBuffSize) {
          _handleOverflow();
          return;
        }

        //read until new line
        if (ch != END_LINE ) {
          _buff[pos] = ch;
          pos++;
        }
        else {
          _buff[pos] = 0; //end-string
          pos = 0;
          _handleMessage();
        }
      }
    }

    void _handleOverflow() {
      if (!_callbackOverflow)
        return;

      _callbackOverflow();
    }

    void _handleMessage() {
      if (!_callbackMessage)
        return;

      //prepare data:
      comMessage data;

      char *token ;
      char *token_rest = _buff;

      if (token = strtok_r(token_rest , DELIMITER, &token_rest ))
        data.msg = atoi(token);

      if (token = strtok_r(token_rest , DELIMITER, &token_rest ))
        data.freq = atoi(token);

      if (token = strtok_r(token_rest , DELIMITER, &token_rest ))
        data.volume = atoi(token);

      if (data.msg == 0)
      {
        for(int i = 0 ; i< LENGTH_COM_MSG ; i++)
          Serial.println(help[i]);
      }
      _callbackMessage(data);
    }
};


#endif
