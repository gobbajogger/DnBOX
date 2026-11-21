#define _GNU_SOURCE
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <ucontext.h>

static volatile sig_atomic_t inside, count, overflow;
static uintptr_t pcs[262144];

static void record(int signal_number, siginfo_t *info, void *ctx)
{
    (void)signal_number;
    (void)info;
    if (!inside) return;
    if ((unsigned)count == sizeof pcs / sizeof pcs[0]) {
        overflow = 1;
        return;
    }
    pcs[count] = (uintptr_t)((ucontext_t *)ctx)->uc_mcontext.gregs[REG_RIP];
    ++count;
}

void profile_setup(void)
{
    struct sigaction action = {0};
    action.sa_sigaction = record;
    action.sa_flags = SA_SIGINFO | SA_RESTART;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGPROF, &action, NULL)) abort();
    struct itimerval timer = {.it_interval = {0, 1000}, .it_value = {0, 1000}};
    if (setitimer(ITIMER_PROF, &timer, NULL)) abort();
}

void profile_collect(int enabled) { inside = enabled; }

void profile_save(void)
{
    struct itimerval timer = {0};
    if (setitimer(ITIMER_PROF, &timer, NULL) || overflow || count == 0) abort();
    FILE *out = fopen("sample-pcs.bin", "wb");
    if (!out || fwrite(pcs, sizeof pcs[0], count, out) != (size_t)count || fclose(out)) abort();
    out = fopen("mappings", "w");
    FILE *in = fopen("/proc/self/maps", "r");
    if (!in || !out) abort();
    int c;
    while ((c = fgetc(in)) != EOF) fputc(c,out);
    if (fclose(in) || fclose(out)) abort();
}
