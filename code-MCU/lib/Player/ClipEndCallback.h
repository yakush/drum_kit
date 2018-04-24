#ifndef ClipEndCallback_H_
#define ClipEndCallback_H_

#define PLAYER_CLIP_ENDED_FINISHED 1
#define PLAYER_CLIP_ENDED_STOPPED 2
#define PLAYER_CLIP_ENDED_PRUNED 3

typedef uint8_t clipEndedReason_t;
typedef void (*PlayerCallback_play_ended_t)(uint32_t clipHandle, clipEndedReason_t reason);

//-----------------------------------------------------------------
#endif //ClipEndCallback_H_