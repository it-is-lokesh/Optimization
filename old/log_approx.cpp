#include <iostream>

static float log_table[256] = {};

float fast_log_crude(float x) {
    union {
        float f;
        uint32_t i;
    } u = { x };

    float y = (float)(u.i);
    return y * 8.262958288192749e-8f - 87.989971088f;
}

float fast_log(float x) {
    union {
        float f;
        uint32_t i;
    } u;
    u.f = x;
    int exp = ((u.i >> 23) & 0xFF) - 127;
    // force exponent to 127 -> mantissa in [1,2)
    u.i = (u.i & 0x7FFFFF) | (127 << 23);
    float m = u.f;
    float t = m - 1.0f;

    float ln_m =
        t
        - 0.5f * t * t
        + (1.0f/3.0f) * t * t * t;

    return exp * 0.69314718056f + ln_m;
}

float lut_log_linear_interpolation(float x) {
    union {
        float f;
        uint32_t i;
    } u;
    u.f = x;
    int exp = ((u.i >> 23) & 0xFF) - 127;
    // force exponent to 127 -> mantissa in [1,2)
    u.i = (u.i & 0x7FFFFF) | (127 << 23);
    float m = u.f;

    // int index = (int)((m - 1.0f) * 256);
    float index = (m - 1.0f) * 256.0f;
    // if (index < 0) index = 0;
    // if (index > 255) index = 255;

    int idx = (int)index;
    float frac = index - idx;

    float ln_m =
        log_table[idx] +
        frac * (log_table[idx+1] - log_table[idx]);

    return exp * 0.69314718056f + ln_m;
}

int main() {
    // Compute log lut
    for (int i = 0; i < 256; ++i) {
        float m = 1.0f + (float)i / 256.0f;
        log_table[i] = std::log(m);
    }

    // write code to profile the three implementations of log. Use for loops of 100 million iterations to get a good profile.
    // get current time
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000000; ++i) {
        fast_log_crude(1.23432432f);
        // fast_log(1.23432432f);
        // std::log(1.23432432f);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "fast_log_crude: " << elapsed.count() << " seconds" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000000; ++i) {
        // fast_log_crude(1.23432432f);
        fast_log(1.23432432f);
        // std::log(1.23432432f);
    }
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    std::cout << "fast_log: " << elapsed.count() << " seconds" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000000; ++i) {
        // fast_log_crude(1.23432432f);
        // fast_log(1.23432432f);
        std::log(1.23432432f);
    }
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    std::cout << "std::log: " << elapsed.count() << " seconds" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000000; ++i) {
        // fast_log_crude(1.23432432f);
        // fast_log(1.23432432f);
        lut_log_linear_interpolation(1.23432432f);
    }
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    std::cout << "lut_log_linear_interpolation: " << elapsed.count() << " seconds" << std::endl;

    return 0;
}