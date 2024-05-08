#include "fumoengine.h"
#include "platform.h"


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
        if (!InputAquire(&inst->input_hand))
            Panic("Aquire failed");
            
        ControllerPoll(&inst->ctrl, &inst->input_hand.recs);

        if (!InputRelease(&inst->input_hand))
            Panic("Release failed");

        nsec now = TimeNow();
        inst->frametime = now - inst->time;
        EventInvoke(&inst->on_update, inst);
        inst->time = now;

        TerminalPrint(&inst->term, buf, buf_n);
        puts(buf);

        //_sleep(100);
    }
}