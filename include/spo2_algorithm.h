#ifndef SPO2_ALGORITHM_H
#define SPO2_ALGORITHM_H

#include <stdint.h>
#include <string.h> // 用于 memset
#include <math.h>   // 用于 isnan

// 算法常量
#define SAMPLING_FREQUENCY 100
#define BUFFER_SIZE (SAMPLING_FREQUENCY * 4) // 存储4秒的数据

class SpO2Algorithm {
public:
    // --- 构造函数 ---
    SpO2Algorithm() {
        reset();
    }

    // --- 公共方法 ---
    void update(float ir_value, float red_value) {
        // 将新数据添加到环形缓冲区
        ir_buffer[buffer_index] = ir_value;
        red_buffer[buffer_index] = red_value;
        buffer_index = (buffer_index + 1) % BUFFER_SIZE;

        // 每100个采样点（即1秒）计算一次新值
        if (buffer_index % 100 == 0) {
            calculate();
        }
    }

    float get_spo2() {
        return spo2;
    }

    float get_heart_rate() {
        return heart_rate;
    }

private:
    // --- 成员变量 ---
    float ir_buffer[BUFFER_SIZE];
    float red_buffer[BUFFER_SIZE];
    int buffer_index;
    float spo2;
    float heart_rate;

    // --- 私有方法 ---
    void reset() {
        memset(ir_buffer, 0, sizeof(ir_buffer));
        memset(red_buffer, 0, sizeof(red_buffer));
        buffer_index = 0;
        spo2 = 0.0f;
        heart_rate = 0.0f;
    }

    void calculate() {
        uint32_t ir_dc_sum = 0;
        uint32_t red_dc_sum = 0;
        for (int i = 0; i < BUFFER_SIZE; i++) {
            ir_dc_sum += ir_buffer[i];
            red_dc_sum += red_buffer[i];
        }

        float ir_dc_avg = (float)ir_dc_sum / BUFFER_SIZE;
        float red_dc_avg = (float)red_dc_sum / BUFFER_SIZE;

        float ir_ac_max = 0, ir_ac_min = 1e6;
        float red_ac_max = 0, red_ac_min = 1e6;
        for (int i = 0; i < BUFFER_SIZE; i++) {
            if (ir_buffer[i] > ir_ac_max) ir_ac_max = ir_buffer[i];
            if (ir_buffer[i] < ir_ac_min) ir_ac_min = ir_buffer[i];
            if (red_buffer[i] > red_ac_max) red_ac_max = red_buffer[i];
            if (red_buffer[i] < red_ac_min) red_ac_min = red_buffer[i];
        }

        float ir_ac_pp = ir_ac_max - ir_ac_min;
        float red_ac_pp = red_ac_max - red_ac_min;

        // 基础的信号质量检查
        if (ir_dc_avg < 50000 || ir_ac_pp < 100) {
            heart_rate = 0;
            spo2 = 0;
            return;
        }

        float R = (red_ac_pp / red_dc_avg) / (ir_ac_pp / ir_dc_avg);
        
        // 这是一个常用的经验公式，更精确需要校准
        float calculated_spo2 = 104.0f - 17.0f * R;
        spo2 = (calculated_spo2 > 80 && calculated_spo2 <= 100) ? calculated_spo2 : 0;

        // 简单的心率峰值检测算法
        int beats = 0;
        for (int i = 2; i < BUFFER_SIZE - 2; i++) {
            bool is_peak = (ir_buffer[i] > ir_buffer[i-1] && ir_buffer[i] > ir_buffer[i-2] &&
                            ir_buffer[i] > ir_buffer[i+1] && ir_buffer[i] > ir_buffer[i+2]);
            if (is_peak && ir_buffer[i] > ir_dc_avg) {
                beats++;
            }
        }
        
        float buffer_duration_sec = (float)BUFFER_SIZE / (float)SAMPLING_FREQUENCY;
        float calculated_hr = (float)beats * 60.0f / buffer_duration_sec;
        heart_rate = (calculated_hr > 40 && calculated_hr < 150) ? calculated_hr : 0;
    }
};

#endif // SPO2_ALGORITHM_H