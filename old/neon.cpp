#include <arm_neon.h>

int main() {
    float32x4_t a = vld1q_f32(ptr1);
    float32x4_t b = vld1q_f32(ptr2);
    
    float32x4_t c = vaddq_f32(a, b);
    
    vst1q_f32(out, c);

}