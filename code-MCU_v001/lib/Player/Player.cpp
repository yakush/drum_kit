#include <string.h>
#include "Player.h"
#include "FS.h"
#include "SD.h"
#include "SoundRenderer.h"
#include "Utils.h"
#include "Logger.h"

#include "Player_playTask.h"
#include "Player_readTask.h"

using namespace NS_player;

#define FILE_QUALITY_16_BIT 16
#define FILE_QUALITY_8_BIT 8

//-----------------------------------------------------------------
//-- class instance
PlayerClass Player;
//-----------------------------------------------------------------

PlayerClass::PlayerClass (){
}

PlayerClass::~PlayerClass (){
}

void PlayerClass::begin(){
  if(started){
    return;
  }
  started = true;

  lastTimeRead=micros();

  //start tasks and timers
  player_playTask_setPlayer(this);
  player_playTask_start();

  player_readTask_setPlayer(this);
  player_readTask_start();
}

void PlayerClass::end(){
  if(!started){
    return;
  }
  started = false;

  //if started
  stopAll();
  player_playTask_stop();
  player_readTask_stop();
}

bool PlayerClass::load(uint8_t sampleId,const char* path){
  if (loadedSamples[sampleId].loaded){
    unload(sampleId);
  }

  //add extention
  char pathWithExt[256];
  getFilePath(pathWithExt,path,FILE_QUALITY_16_BIT);

  if (SD.exists(pathWithExt)){
    strcpy(loadedSamples[sampleId].filename, path);
    loadedSamples[sampleId].loaded=true;
    return true;
  }else{
    strcpy(loadedSamples[sampleId].filename, "\0");
    loadedSamples[sampleId].loaded=false;
    return false;
  }
}

void PlayerClass::unload(uint8_t sampleId){
  strcpy(loadedSamples[sampleId].filename, "\0");
  loadedSamples[sampleId].loaded=false;
}

void PlayerClass::unloadAll(){
  for (size_t i = 0; i < PLAYER_MAX_LOADED_SAMPLES; i++) {
    unload(i);
  }
}

long PlayerClass::play(uint8_t sampleId, uint8_t volume, PlayerCallback_play_ended_t callback){
  //check validity and loaded state
  if (sampleId >= PLAYER_MAX_LOADED_SAMPLES){
    return 0;
  }
  if (!loadedSamples[sampleId].loaded){
    return 0;
  }

  //find a free slot
  uint8_t pos=0;
  uint8_t oldestSampleIdx = 0;
  size_t oldestSamplePlayPos =0;
  bool found=false;
  bool foundOldest=false;

  for (size_t i = 0; i < PLAYER_MAX_PLAYING_SAMPLES; i++) {
    //try to find a free slot
    if (!playingSamples[i].isPlaying){
      pos=i;
      found=true;
      break;
    }
    //collect oldest sample (playing pos is highest)
    //in case there are no free slots - kill the oldest
    if (playingSamples[i].pos>oldestSamplePlayPos){
      oldestSamplePlayPos = playingSamples[i].pos;
      oldestSampleIdx=i;
      foundOldest=true;
    }
  }

  //if not found - stop and use the oldest sample slot
  if (!found && foundOldest){
    found=true;
    pos=oldestSampleIdx;
    stopSampleByIdx(pos,PLAYER_SAMPLE_ENDED_PRUNED);
  }

  if (!found){
    Serial.printf(" -- didn't find ana available sample\n");
    return 0;
  }
  
  Serial.printf(" -- playing sample on position %u",pos);
  Serial.println();

  Sample *s = &playingSamples[pos];

  //try to open the file (16bit)
  char pathWithExt[256];
  getFilePath(pathWithExt,loadedSamples[sampleId].filename,FILE_QUALITY_16_BIT);

  Serial.printf(" -- filename %s",pathWithExt);
  Serial.println();

  //TODO : lock mutex for inserting sample?
  //prepare the sample

  if (s->file){
    Serial.printf(" -- closing old file");
    s->file.close();
  }

  resetSample(s);

  if (!SD.exists(pathWithExt)){
    return 0;
  }
  Serial.printf(" -- exists");

  //s->file = SD.open(pathWithExt, FILE_READ);
  s->file = SD.open(pathWithExt, FILE_READ);

  if (!s->file){
    return 0;
  }
  Serial.printf(" -- opened");
  Serial.println();
  Serial.printf(" -- fileSize %u",s->file.size());
  Serial.println();

  //prepare sample
  lastSampleHandle++;

  s->sampleHandle = lastSampleHandle;
  s->volume = volume;
  s->isPlaying = true;
  s->cbEnd=callback;
  s->fileSize = s->file.size();
  s->needFillBuffer = true;

  return lastSampleHandle;
}

void PlayerClass::stop(long sampleHandle){
  for (size_t i = 0; i < PLAYER_MAX_PLAYING_SAMPLES; i++) {
    //try to find the sample
    if (playingSamples[i].sampleHandle==sampleHandle){
      stopSampleByIdx(i,PLAYER_SAMPLE_ENDED_STOPPED);
      break;
    }
  }
}

void PlayerClass::stopAll(){
  for (size_t i = 0; i < PLAYER_MAX_PLAYING_SAMPLES; i++) {
    stopSampleByIdx(i,PLAYER_SAMPLE_ENDED_STOPPED);
  }
}

long* PlayerClass::getPlayingSamplesHandles(){
  return NULL;
}

//-----------------------------------------------------------------
// TASKS AND TIMERS
//-----------------------------------------------------------------
void PlayerClass::_handlePlayTimerTick(){

  unsigned long nowUSec = micros();

  int32_t val=0;
  int32_t sampleVal=0;

  bool readNextValue=false;
  uint16_t dT_lastRead ;

  // time since last read + overflow fix
  if (nowUSec>=lastTimeRead){
    dT_lastRead= nowUSec-lastTimeRead;
  } else {
    dT_lastRead = nowUSec+(UINT32_MAX-lastTimeRead);
  }

  // passed 44100 mark?
  // update lastTimeRead (aligned to 44100 rate)
  if (dT_lastRead >= PLAYER_SAMPLE_T_USEC ){
    readNextValue=true;

    if (nowUSec>=lastTimeRead){
      lastTimeRead+=PLAYER_SAMPLE_T_USEC;
    } else {
      lastTimeRead = PLAYER_SAMPLE_T_USEC-(UINT32_MAX-lastTimeRead);
    }

    dT_lastRead= nowUSec-lastTimeRead;
  }

  for (size_t i = 0;  i< PLAYER_MAX_PLAYING_SAMPLES; i++) {
    Sample *s=&(playingSamples[i]);
    if (!s->isPlaying || !s->firstBufferReady){
      continue;
    }

    //get fresh value from buffer
    if (readNextValue){

      s->val_0 = s->val_1;

      if (s->quality==PLAYER_SAMPLE_16_BIT_44100){
        // 16bit singed little endian
        s->val_1 =  s->buffer[s->activeBuffer][s->bufferPos+1];
        s->val_1 =  s->val_1 <<8;
        s->val_1 |= s->buffer[s->activeBuffer][s->bufferPos+0];

        s->bufferPos+=2;
        s->pos+=2;

      } else if (s->quality==PLAYER_SAMPLE_8_BIT_44100) {
        // 8bit unsinged
        s->val_1 =  s->buffer[s->activeBuffer][s->bufferPos+0];
        s->val_1 =  s->val_1 <<8;
        s->val_1 ^= 0x8000; //convert to singed (flip most significant bit)

        s->bufferPos+=1;
        s->pos+=1;

      } else {
        // ...?!?!
      }
    }

    // interpolate  === V0 + (V1-V0) * (t-T0)/(T1-T0)
    // adjust volume (0..128..255 => 0%..100%..200%)
    sampleVal = (s->val_0) + ((((int32_t)s->val_1 - s->val_0) * dT_lastRead) / PLAYER_SAMPLE_T_USEC);
    sampleVal = (sampleVal*s->volume)/128;
    val += sampleVal;
  }

  //mix all with sigmoid and play
  val = fastSigmoid(val) +  0x7FFF;
  SoundRenderer.playValue( (uint16_t)val);

  // update states (flip and end)
  lockSamples_ISR();
  for (size_t i = 0;  i< PLAYER_MAX_PLAYING_SAMPLES; i++) {
    Sample *s=&playingSamples[i];

    if (s->isStopping){
      if (!s->isReadingFile){
        // this sample was marked for disposal
        // and the file is not being read by the read task
        stopSampleByIdx(i, PLAYER_SAMPLE_ENDED_FINISHED);
      }

    }else if (s->isPlaying && s->firstBufferReady){

      if (s->pos >= s->fileSize){
        //finished file - mark for stop
        //this will wait until the file is not read by the read task
        //before closing the file
        s->isStopping=true;

      } else {
        //finished buffer - flip buffers
        if (s->bufferPos>=PLAYER_BUFFER_SIZE){
          s->activeBuffer = !s->activeBuffer;
          s->bufferPos=0;
          s->needFillBuffer=true;
        }
      }
    }
  }
  unlockSamples_ISR();

}
//-----------------------------------------------------------------

void PlayerClass::_handleReadTask(){

  static Sample* readList[PLAYER_MAX_PLAYING_SAMPLES];
  static int numSamplesToRead;

  lockSamples();
  //prepare list (pointers array)
  //TODO: maybe a struct holding the sample + the handle to check later if the handle changed?
  numSamplesToRead=0;
  for (size_t i = 0;  i< PLAYER_MAX_PLAYING_SAMPLES; i++) {
    Sample *s=&playingSamples[i];
    if (s->isPlaying && s->needFillBuffer){
      s->isReadingFile=true;
      readList[numSamplesToRead]=s;
      numSamplesToRead++;
    }
  }
  unlockSamples();

  //read samples
  for (size_t i = 0;  i<numSamplesToRead; i++) {
    Sample *s=readList[i];

    s->file.read(s->buffer[!s->activeBuffer], PLAYER_BUFFER_SIZE);

    //first time only - fill both buffers (active and non active)
    if (!s->firstBufferReady){
      s->file.read(s->buffer[s->activeBuffer], PLAYER_BUFFER_SIZE);
    }
  }

  lockSamples();
  // update states (only for samples that didn't stop/change while the read was going on)
  // TODO: maybe a struct holding the sample + the handle to check here if the handle changed?
  for (size_t i = 0;  i<numSamplesToRead; i++) {
    Sample *s=readList[i];

    s->isReadingFile=false;

    if (s->isPlaying && s->needFillBuffer){
      s->firstBufferReady = true;
      s->needFillBuffer=false;
    }
  }
  unlockSamples();

  //play nice pause every second...
  unsigned long now = millis();
  if ( (now>lastReadTaskYeildTime + 1000) || (now<lastReadTaskYeildTime) ){
    lastReadTaskYeildTime=now;
    vTaskDelay(1);
  }
}

//helpers
char* PlayerClass::getFilePath(char* dest ,const char* path, uint8_t quality){
  char ext[4+1];
  switch (quality) {

    case FILE_QUALITY_16_BIT:
    strcpy(ext,PLAYER_FILE_16_BIT_44100);
    break;

    case FILE_QUALITY_8_BIT:
    default:
    strcpy(ext,PLAYER_FILE_8_BIT_44100);
    break;
  }

  //add extention (.s16)
  strcpy (dest,path) ;
  strcat (dest, ext) ;
  return dest;
}

void PlayerClass::stopSampleByIdx( uint8_t sampleIdx, uint8_t reason){
  Sample *s = &playingSamples[sampleIdx];
  if (!s->isPlaying){
    return;
  }

  //fire callback
  if (s->cbEnd){
    s->cbEnd(sampleIdx,reason);
  }

  //clean sample
  resetSample(s);
}

void PlayerClass::resetSample(Sample* s){
  s->sampleHandle=0;
  s->isPlaying = false;
  s->cbEnd=NULL;
  // if (s->file){
  //   s->file.close();
  // }
  s->pos = 0;
  s->fileSize=0;
  s->activeBuffer = 0;
  s->bufferPos = 0;
  s->needFillBuffer = false;
  s->firstBufferReady = false;
  s->val_0 = 0;
  s->val_1 = 0;
  s->quality = PLAYER_SAMPLE_16_BIT_44100;
  s->isReadingFile=false;
  s->isStopping=false;
}
//-----------------------------------------------------------------
//-----------------------------------------------------------------

/*
PlayerClass *player;
int count=0;
long countTicks=0;

void IRAM_ATTR onPlayTimer() {

if (count==0){
countTicks++;
Serial.printf("*** tick %u : ",countTicks);

if (player){
player->_handlePlayTimerTick();
}else{
Serial.println("no player");
}
}

count++;
if (count>96000) count=0;
}

void deletePlayTimer(){
if (!playTimer){
return;
}
timerDetachInterrupt(playTimer);
timerEnd(playTimer);
player=NULL;
}

void initPlayTimer(PlayerClass *currentPlayer){

deletePlayTimer();
player = currentPlayer;

// 1 tick take 1/(80MHZ/divider) . i.e 80 -> 1us
playTimer = timerBegin(0, 2, true);
timerAttachInterrupt(playTimer, &onPlayTimer, true);
timerAlarmWrite(playTimer, 40000000 / 96200, true); // 20 uSecs = 96khz
timerAlarmEnable(playTimer);
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
*/