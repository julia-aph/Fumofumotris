#include "fumoengine.h"
#include "platform.h"


VectorT FUMOSYS_VEC_T = VECTOR_T(struct FumoHook);


void Panic(char *message)
{
    printf(message);
    exit(1);
}

bool CreateFumoInstance(struct FumoInstance *instance)
{
    if (!PlatformInit())
        Panic("Platform failed to initialize");

    if (!CreateController(&instance->ctrl))
        Panic("Out of memory");

    if (!CreateInputThread(&instance->input_hand))
        Panic("Input handle failed to initialize");

    if (!CreateTerminal(&instance->term, 20, 10))
        Panic("Out of memory");

    if (!CreateEvent(&instance->on_start))
        Panic("Out of memory");

    if (!CreateEvent(FUMOSYS_VEC_T, &instance->on_update))
        Panic("Out of memory");

    instance->time = TimeNow();

    return true;
}

bool FumoInstanceRun(struct FumoInstance *instance)
{
    usize buf_n = TerminalMaxOut(&instance->term);
    char *buf = malloc(buf_n);

    while (true) {
        if (!InputAquire(&instance->input_hand))
            Panic("Aquire failed");
            
        ControllerPoll(&instance->ctrl, &instance->input_hand.recs);

        if (!InputRelease(&instance->input_hand))
            Panic("Release failed");

        nsec now = TimeNow();
        instance->frametime = now - instance->time;
        FumoInvoke(instance, &instance->on_update);
        instance->time = now;

        TerminalPrint(&instance->term, buf, buf_n);
        puts(buf);

        _sleep(100);
    }
}