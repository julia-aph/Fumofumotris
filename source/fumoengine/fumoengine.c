#include "fumoengine.h"
#include "platform.h"


void Panic(char *message)
{
    printf(message);
    exit(1);
}

bool CoroutineAdd(struct Instance *inst, void *state, coroutine_handler callback)
{
    return VectorAdd(&inst->coroutines, &(struct Coroutine) {
        .callback = callback,
        .state = state,
        .next_scheduled = inst->time
    });
}

void CoroutineTryInvoke(struct Instance *inst, struct Coroutine *co)
{
    while (inst->time > co->next_scheduled) {
        nsec wait = co->callback(inst, co->state);
        if (wait == 0) {
            co->next_scheduled = inst->time;
            break;
        } else {
            co->next_scheduled += wait;
        }
    }
}

bool CreateFumoInstance(struct Instance *instance)
{
    if (!PlatformInit())
        Panic("Platform failed to initialize");

    if (!CreateController(&instance->ctrl))
        Panic("Out of memory");

    if (!CreateInputThread(&instance->input_hand))
        Panic("Input handle failed to initialize");

    if (!CreateEvent(&instance->on_start))
        Panic("Out of memory");

    if (!CreateEvent(&instance->on_update))
        Panic("Out of memory");

    if (!CreateEvent(&instance->on_draw))
        Panic("Out of memory");

    if (!CreateVector(&instance->coroutines, sizeof(struct Coroutine)))
        Panic("Out of memory");

    instance->time = TimeNow();

    return true;
}

bool FumoInstanceRun(struct Instance *inst)
{
    EventInvoke(&inst->on_start, inst);

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
            CoroutineTryInvoke(inst, VectorGet(&inst->coroutines, i));
        }

        // Draw
        EventInvoke(&inst->on_draw, inst);

        //_sleep(100);
    }
}