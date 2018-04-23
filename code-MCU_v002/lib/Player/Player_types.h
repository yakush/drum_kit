#ifndef PLAYER_TYPES_H_
#define PLAYER_TYPES_H_

namespace NS_player
{

typedef void (*PlayerCallback_play_ended_t)(uint32_t sampleHandle, uint8_t reason);

class Sample
{
    unsigned long sampleHandle;

    uint8_t volume = 128;
    uint8_t blend = 128;

    PlayerCallback_play_ended_t cbEnd = NULL;

    File file;
    size_t fileSize = 0;
    size_t pos = 0;

    byte buffer[2][PLAYER_BUFFER_SIZE];
    short activeBuffer = 0;
    short bufferPos = 0;
    bool needFillBuffer = false;
    bool firstBufferReady = false;

    bool isPlaying = false;
    bool isReadingFile = false;
    bool isStopping = false;

    int16_t val_0 = 0;
    int16_t val_1 = 0;
    uint8_t quality = PLAYER_SAMPLE_16_BIT_44100;
};

typedef struct
{
    bool loaded = false;
    char filename[256];
    // optional - initial buffer ?
    // carefull if implemented varying resolution options ,
    // may need to load multiple buffers (for each resolution) ... :(
} LoadedSample;

}
#endif