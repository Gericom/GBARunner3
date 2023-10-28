#pragma once

typedef struct
{
    u16 length;
    u16 duty;
    u16 stepTime;
    u16 initVolume;
    u16 curVolume;
    u16 direction;
    u16 dead;
    u16 nextStep;
} gbs_envelope_t;

#ifdef __cplusplus
extern "C" {
#endif

bool gbs_resetEnvelope(gbs_envelope_t* env);
bool gbs_writeEnvelope(gbs_envelope_t* env, u16 value);
void gbs_updateEnvelope(gbs_envelope_t* env);

#ifdef __cplusplus
}
#endif
