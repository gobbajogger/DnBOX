/* L13: on SEQ, tapping the visible slider-lane label (SEQ held + SEL, the
 * FM-1 label key) steps VELOCITY -> CHANCE -> PITCH -> PAN -> VELOCITY for
 * 909, 808, BREAK and both 303s. Each tap redraws the label, stays on SEQ,
 * opens no list and leaves the pattern alone; plain SEL still opens the list. */
#define main x0x_host_main
#include "../../host/x0x_host.c"
#undef main

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL: " __VA_ARGS__); printf("\n"); } } while (0)

static void tick(uint32_t n) { while (n--) run_ms(1); }
static void btn(int b, int down) { if (down) held_btn |= 1u << b; else held_btn &= ~(1u << b); tick(1); }
static uint32_t label_hash(void)
{
    uint32_t h = 2166136261u;
    ui_frame();
    for (int y = KNOB_Y + 69; y < KNOB_Y + 82; y++)
        for (int x = 0; x < 66; x++) h = (h ^ fb[y * 240 + x]) * 16777619u;
    return h;
}
static void label_tap(void)
{
    btn(B_SEQ, 1);
    btn(B_SEL, 1);
    btn(B_SEL, 0);
    btn(B_SEQ, 0);
    tick(5);
}

int main(void)
{
    static const int parts[] = {PART_909, PART_808, PART_BRK, PART_303A, PART_303B};
    static const char *const lane[UI_LANE_COUNT] = {"VELOCITY", "CHANCE", "PITCH", "PAN"};
    boot();
    tick(300);
    for (unsigned n = 0; n < sizeof parts / sizeof parts[0]; n++) {
        int part = parts[n];
        const char *name = part_name(part);
        uint32_t hash[UI_LANE_COUNT];
        set_view(V_SEQ);
        ui.overlay = O_NONE;
        ui.part = (uint8_t)part;
        build_pages();
        ui_step_lane = UI_LANE_VELOCITY;
        tick(5);
        pattern_t keep = *cur_pat();
        hash[0] = label_hash();
        for (int k = 1; k <= UI_LANE_COUNT; k++) {
            label_tap();
            int want = k % UI_LANE_COUNT;
            CHECK(ui_step_lane == want, "%s: tap %d selects %s (got %d)", name, k, lane[want], ui_step_lane);
            CHECK(ui.view == V_SEQ && ui.overlay == O_NONE && !ui.help, "%s: tap %d stays on SEQ with no list", name, k);
            uint32_t h = label_hash();
            if (k < UI_LANE_COUNT) {
                hash[k] = h;
                for (int j = 0; j < k; j++)
                    CHECK(h != hash[j], "%s: the %s label differs from %s", name, lane[k], lane[j]);
            } else {
                CHECK(h == hash[0], "%s: the fourth tap returns to the VELOCITY label", name);
            }
        }
        CHECK(!memcmp(cur_pat(), &keep, sizeof keep), "%s: label taps do not edit the pattern", name);
    }
    /* SEL without SEQ keeps its list role and does not step the lane. */
    set_view(V_SEQ);
    ui.overlay = O_NONE;
    ui.part = PART_909;
    ui_step_lane = UI_LANE_CHANCE;
    btn(B_SEL, 1);
    btn(B_SEL, 0);
    tick(5);
    CHECK(ui_step_lane == UI_LANE_CHANCE, "plain SEL does not step the lane");
    CHECK(ui.overlay == O_LIST, "plain SEL still opens the list");
    if (fails) { printf("seq-lane-label-tap: %d failure(s)\n", fails); return 1; }
    printf("seq-lane-label-tap: PASS\n");
    return 0;
}
