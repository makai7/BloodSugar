#ifndef SPO2_ALGORITHM_H
#define SPO2_ALGORITHM_H

#include <stdint.h>

// 算法中使用的常量
#define FS 100         // 采样频率 (必须与MAX30102的设置一致)
#define BUFFER_SIZE (FS * 4) // 存储4秒的数据

class SpO2Algorithm {
public:
    SpO2Algorithm();
    void update(float ir_value, float red_value);
    float get_spo2();
    float get_heart_rate();
    void reset();

private:
    float ir_buffer[BUFFER_SIZE];
    float red_buffer[BUFFER_SIZE];
    int buffer_index;

    float spo2;
    float heart_rate;
    bool is_valid_spo2;
    bool is_valid_heart_rate;

    void calculate_spo2_and_hr();
};

#endif // SPO2_ALGORITHM_H