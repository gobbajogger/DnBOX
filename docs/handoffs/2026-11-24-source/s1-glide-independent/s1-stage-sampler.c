#define _POSIX_C_SOURCE 200809L
/* Linux host-only statistical sampler. The render thread only writes tags. */
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

volatile sig_atomic_t s1_host_stage;
uint64_t s1_host_frames;
static volatile sig_atomic_t samples[7];

static void collect(int sig)
{
    (void)sig;
    sig_atomic_t stage = s1_host_stage;
    if (stage >= 0 && stage < 7) ++samples[stage];
}

__attribute__((constructor)) static void start_sampler(void)
{
    struct sigaction action = {0};
    action.sa_handler = collect;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    const char *period = getenv("DNBOX_S1_SAMPLE_US");
    long us = period ? strtol(period, NULL, 10) : 1000;
    if (us < 250 || us > 10000 || sigaction(SIGPROF, &action, NULL)) abort();
    struct itimerval timer = {{0, us}, {0, us}};
    if (setitimer(ITIMER_PROF, &timer, NULL)) abort();
}

__attribute__((destructor)) static void stop_sampler(void)
{
    struct itimerval stopped = {0};
    if (setitimer(ITIMER_PROF, &stopped, NULL)) abort();
    printf("s1-stage-samples %llu", (unsigned long long)s1_host_frames);
    for (unsigned i = 0; i < 7; ++i) printf(" %d", (int)samples[i]);
    putchar('\n');
}
