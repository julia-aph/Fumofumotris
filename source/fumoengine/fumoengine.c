#include "fumoengine.h"
#include "platform.h"


const VectorT FUMOCO_T = VECTOR_T(struct FumoCoroutine);


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

    if (!CreateEvent(&instance->on_update))
        Panic("Out of memory");

    instance->time = TimeNow();

    return true;
}

bool FumoInstanceRun(struct FumoInstance *inst)
{
    EventInvoke(&inst->on_start, inst);

    usize buf_n = TerminalMaxOut(&inst->term);
    char *buf = malloc(buf_n);

    while (true) {
        // Time
        nsec now = TimeNow();
        inst->frametime = now - inst->time;
        inst->time = now;

        // Input
        if (!InputAquire(&inst->input_hand))
            Panic("Aquire failed");
            
        ControllerPoll(&inst->ctrl, &inst->input_hand.recs);

        if (!InputRelease(&inst->input_hand))
            Panic("Release failed");
        
        // Update
        EventInvoke(&inst->on_update, inst);
        for (usize i = 0; i < inst->coroutines.len; i++) {
            struct FumoCoroutine *co = VectorGet(FUMOCO_T, &inst->coroutines, i);
            co->callback();
        }

        // Draw
        EventInvoke(&inst->on_draw, inst);
        TerminalPrint(&inst->term, buf, buf_n);
        puts(buf);

        //_sleep(100);
    }
}

bool CoroutineAdd(struct FumoInstance *inst, handler callback, nsec period)
{
    return VectorAdd(FUMOCO_T, &inst->coroutines, &(struct FumoCoroutine) {
        .callback = callback,
        .timer = 0,
        .period = period
    });
}