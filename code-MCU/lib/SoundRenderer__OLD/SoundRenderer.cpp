#include "SoundRenderer.h"

//ESP LIBS
#include "esp32-hal.h"
#include "esp32-hal-matrix.h"
#include "soc/gpio_sd_reg.h"
#include "soc/gpio_sd_struct.h"

//-----------------------------------------------------------------
//class instance
SoundRendererClass SoundRenderer;
//-----------------------------------------------------------------

SoundRendererClass::SoundRendererClass(){

}

SoundRendererClass::~SoundRendererClass(){

}

void SoundRendererClass::begin(uint8_t pinHi,uint8_t pinLo,uint8_t channelHi ,uint8_t channelLo ,uint32_t carrierFreq,boolean highSpeedWrite){
  this->pinHi=pinHi;
  this->pinLo=pinLo;
  this->channelHi=channelHi;
  this->channelLo =channelLo ;
  this->carrierFreq=carrierFreq;
  this->highSpeedWrite=highSpeedWrite;

  sigmaDeltaSetup(channelHi, carrierFreq); //==>20 MHz
  sigmaDeltaSetup(channelLo, carrierFreq); //==>20 MHz
  sigmaDeltaAttachPin(pinHi, channelHi);
  sigmaDeltaAttachPin(pinLo, channelLo);

  playValue(0x7FFF);
}

void SoundRendererClass::end(){
  sigmaDeltaDetachPin(this->pinHi);
  sigmaDeltaDetachPin(this->pinLo);
}

void SoundRendererClass::playValue( uint16_t value ){
  playValue(
    (uint8_t )(value >> 8),
    (uint8_t )((value ))
  );
}

void SoundRendererClass::playValue( uint8_t valueHi, uint8_t valueLo ){
  if (highSpeedWrite) {
    //HIGH SPEED (no mutex)
    SIGMADELTA.channel[channelHi].duty = 128 + valueHi; //128-380 (255 range)
    SIGMADELTA.channel[channelLo].duty = 128 + valueLo; //128-380 (255 range)
  } else {
    //LOW SPEED
    sigmaDeltaWrite(channelHi, valueHi);
    sigmaDeltaWrite(channelLo, valueLo);
  }
}
