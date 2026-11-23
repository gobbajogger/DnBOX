#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define FM1_TICKS_PER_US 1u
#define FM1_AUDIO_HALF 128u
#define HALF_WORDS 1024u
#define HALF_FRAMES 512u
#define FS 44100u
#define FM1_IRQ_ALNK0 11u
#define X0X_DEBUG 0
static uint32_t bank, pending, time_us, next_us, render_us, jobs, released,
    irq_on, enabled;
static uint8_t safe_mode, boot_silent_half;
static unsigned audio_xruns, nested;
static int32_t abuf[2048];
static struct {
    unsigned late;
} x0x_dbg;
void fm1_alnk0_irq(void);
void isr_alnk0(void) {
    fm1_alnk0_irq();
}
#if VECTOR
void fm1_alnk0_boot_irq(void);
void isr_alnk0_boot(void) {
    fm1_alnk0_boot_irq();
}
static void (*handler)(void);
static void fm1_irq_attach(unsigned n, void (*callback)(void),
                           unsigned priority) {
    assert(n == 11 && priority == 3);
    handler = callback;
}
#endif
static void dispatch(void) {
    if (!irq_on || !enabled || !pending || nested)
        return;
    nested = 1;
#if VECTOR
    handler();
#else
    fm1_alnk0_irq();
#endif
    nested = 0;
}
static void advance(void) {
    if (++time_us == next_us) {
        bank ^= 1u;
        pending = FM1_AUDIO_HALF;
        ++released;
        next_us += 11610;
    }
    dispatch();
}
static uint32_t fm1_ticks(void) {
    advance();
    return time_us;
}
static uint8_t fm1_audio_pending(void) {
    return pending;
}
static uint32_t fm1_audio_free_half(void) {
    return bank;
}
static void fm1_audio_ack_aux(uint8_t p) {
    (void)p;
}
static void fm1_audio_ack_half(void) {
    pending = 0;
}
static void fm1_irq_enable_all(void) {
    irq_on = 1;
    dispatch();
}
#if !VECTOR
static uint32_t fm1_irq_mask_save(uint32_t n) {
    assert(n == 11);
    uint32_t old = enabled;
    enabled = 0;
    return old;
}
static void fm1_irq_mask_restore(uint32_t n, uint32_t value) {
    assert(n == 11);
    enabled = value;
    dispatch();
}
#endif
static void render_block(int32_t *o) {
    ++jobs;
    for (unsigned i = 0; i < render_us; i++)
        advance();
    for (unsigned i = 0; i < HALF_WORDS; i++)
        o[i] = (int32_t)(jobs * 2048u + i + 1u);
}
/* FIRMWARE_IRQ */
static void handoff(void) {
    /* FIRMWARE_HANDOFF */
}
static void begin(unsigned initial, unsigned first) {
    time_us = pending = released = jobs = audio_xruns = x0x_dbg.late = 0;
    irq_on = boot_silent_half = safe_mode = 0;
    enabled = 1;
    bank = initial ^ 1u;
    next_us = first;
    render_us = 1000;
#if VECTOR
    handler = isr_alnk0;
#endif
    memset(abuf, 0, sizeof abuf);
}
static void words(unsigned which, unsigned group) {
    for (unsigned i = 0; i < HALF_WORDS; i++)
        assert(abuf[which * HALF_WORDS + i] ==
               (group ? (int32_t)(group * 2048 + i + 1) : 0));
}
int main(void) {
    unsigned runs = 0;
    for (unsigned initial = 0; initial < 2; initial++) {
        for (unsigned first = 3000; first <= 12500; first += 250) {
            begin(initial, first);
            smk_usb_boot_settle();
            assert(jobs == 1 && released == 2 && !pending);
            handoff();
            while (released < 3)
                advance();
            assert(jobs == 1 && released == 3 && !pending && enabled);
            words(initial, 0);
            words(initial ^ 1u, 1);
            while (released < 4)
                advance();
            assert(jobs == 2 && !x0x_dbg.late);
            words(initial ^ 1u, 2);
            ++runs;
        }
        for (unsigned first = 13500; first <= 35000; first += 250) {
            begin(initial, first);
            smk_usb_boot_settle();
            assert(jobs == 0);
            handoff();
            while (jobs == 0)
                advance();
            words(initial, 1);
            ++runs;
        }
        begin(initial, 2500);
        safe_mode = 1;
        smk_usb_boot_settle();
        assert(!jobs);
        ++runs;
        begin(initial, 2500);
        smk_usb_boot_settle();
        next_us = UINT32_MAX;
        handoff();
        assert(enabled && jobs == 1 && !x0x_dbg.late);
        unsigned until = time_us + 50000;
        while (time_us < until)
            advance();
        next_us = time_us + 1000;
        while (released < 3)
            advance();
        printf("paused clock: first bank %u, jobs after spacer %u, spacer bank "
               "first word %d\n",
               initial, jobs, (int)abuf[initial * HALF_WORDS]);
        if (VECTOR) {
            assert(jobs == 1);
            words(initial, 0);
            while (released < 4)
                advance();
            assert(jobs == 2);
        } else {
            assert(jobs == 2);
            words(initial, 2);
        }
        ++runs;
        for (unsigned latency = 12000; latency <= 24000; latency += 12000) {
            begin(initial, 7000);
            render_us = latency;
            smk_usb_boot_settle();
            assert(jobs == 1 && pending && audio_xruns == 1 &&
                   x0x_dbg.late == 1);
            ++runs;
        }
    }
    printf("%u controlled clock schedules checked\n", runs);
}
