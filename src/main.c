#include <dspmodule.h>

const unsigned short dspmodule_requiredAPIversion = 1;

unsigned short dspmodule_startup(const DSPLoaderAPI *lapi, int argc, char * const argv[], const char **sysname, const char **dispname)
{
    *sysname = "dspsigvis";
    *dispname = "signal visualizer";
    return 0;
}

unsigned short dspmodule_process(const DSPLoaderAPI *lapi, unsigned long long position, unsigned long duration, unsigned long rate, unsigned long long nsectime)
{
    return 0;
}

void dspmodule_cleanup(void) {}
