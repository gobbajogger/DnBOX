// The two short right delays share their read positions and feedback.
// Linearity allows their inputs to be summed before writing one history.
#pragma once
#include <algorithm>
#include <vector>

template <typename Word>
struct FoldedLine {
    std::vector<Word> history;
    int position = 0;
    float previous = 0.f;
    explicit FoldedLine(unsigned max_frames) : history(max_frames + 4) {}
    void reset() {
        std::fill(history.begin(), history.end(), Word{});
        position = 0;  // Soundpipe retains its previous feedback sample on reset.
    }
    float run(float input, float time, float feedback) {
        const int n = static_cast<int>(history.size());
        history[position] = static_cast<Word>(input + previous * feedback);
        float fraction = time * -44100.f;
        int at = static_cast<int>(fraction);
        fraction -= static_cast<float>(at);
        at += position;
        if (at < 0 || fraction < 0.f) {
            ++fraction;
            --at;
            if (at < 0) at += n;
        }
        const int before = at ? at - 1 : n - 1;
        const int next = at + 1 == n ? 0 : at + 1;
        const int after = next + 1 == n ? 0 : next + 1;
        float z = (fraction * fraction - 1.f) * 0.1666666667f;
        float y = (fraction + 1.f) * 0.5f;
        float w = y - 1.f;
        float x = 3.f * z;
        y -= x;
        w -= z;
        x -= fraction;
        previous = (w * static_cast<float>(history[before]) + x * static_cast<float>(history[at]) +
                    y * static_cast<float>(history[next]) + z * static_cast<float>(history[after]))
                    * fraction + static_cast<float>(history[at]);
        if (++position == n) position = 0;
        return previous;
    }
};

template <typename Word>
struct FoldedDelay {
    FoldedLine<Word> left{220500}, right{220500}, stagger{110250};
    void reset() { left.reset(); right.reset(); stagger.reset(); }
    void frame(float l, float r, float time, float feedback, float& wet_l, float& wet_r) {
        wet_l = left.run(l, time * 2.f, feedback);
        float delayed_r = right.run(r, time * 2.f, feedback);
        wet_r = stagger.run(r + delayed_r, time, feedback);
    }
};
