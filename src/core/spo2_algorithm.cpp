#include "spo2_algorithm.h"
#include <string.h> // Required for memset
#include <math.h>

// Constructor: Initializes the object
SpO2Algorithm::SpO2Algorithm() {
    this->reset();
}

// Resets all internal variables to their default state
void SpO2Algorithm::reset() {
    // Use memset to efficiently clear the buffers
    memset(this->ir_buffer, 0, sizeof(this->ir_buffer));
    memset(this->red_buffer, 0, sizeof(this->red_buffer));
    
    this->buffer_index = 0;
    this->spo2 = 0.0f;
    this->heart_rate = 0.0f;
    this->is_valid_spo2 = false;
    this->is_valid_heart_rate = false;
}

// Adds a new IR and Red data sample to the buffer
void SpO2Algorithm::update(float ir_value, float red_value) {
    this->ir_buffer[this->buffer_index] = ir_value;
    this->red_buffer[this->buffer_index] = red_value;
    this->buffer_index = (this->buffer_index + 1) % BUFFER_SIZE;

    // Trigger a new calculation only when the buffer has been filled with a new cycle of data
    if (this->buffer_index == 0) {
        this->calculate_spo2_and_hr();
    }
}

// Performs the main SpO2 and Heart Rate calculation
void SpO2Algorithm::calculate_spo2_and_hr() {
    float ir_ac_max = 0, ir_ac_min = 1e6;
    float red_ac_max = 0, red_ac_min = 1e6;
    double ir_dc_sum = 0, red_dc_sum = 0;
    int beat_count = 0;
    bool last_ir_above_avg = false;

    // Calculate DC component (average) for both signals
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        ir_dc_sum += this->ir_buffer[i];
        red_dc_sum += this->red_buffer[i];
    }
    float ir_dc_avg = ir_dc_sum / BUFFER_SIZE;
    float red_dc_avg = red_dc_sum / BUFFER_SIZE;

    // Find AC component (peak-to-peak) and count heartbeats
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        float ir_val = this->ir_buffer[i];
        float red_val = this->red_buffer[i];

        if (ir_val > ir_ac_max) ir_ac_max = ir_val;
        if (ir_val < ir_ac_min) ir_ac_min = ir_val;

        if (red_val > red_ac_max) red_ac_max = red_val;
        if (red_val < red_ac_min) red_ac_min = red_val;

        // Simple beat detection: count rising edges crossing the average
        bool current_ir_above_avg = (ir_val > ir_dc_avg);
        if (!last_ir_above_avg && current_ir_above_avg) {
            beat_count++;
        }
        last_ir_above_avg = current_ir_above_avg;
    }

    float ir_ac = ir_ac_max - ir_ac_min;
    float red_ac = red_ac_max - red_ac_min;

    // Calculate SpO2 using the Ratio of Ratios method
    if (ir_dc_avg > 50000 && red_dc_avg > 50000 && ir_ac > 100 && red_ac > 100) {
        float R = (red_ac / red_dc_avg) / (ir_ac / ir_dc_avg);
        this->spo2 = 104.0f - 17.0f * R; // Empiric formula
        this->is_valid_spo2 = (this->spo2 > 80 && this->spo2 <= 100);
    } else {
        this->is_valid_spo2 = false;
    }
    
    // Calculate Heart Rate
    float buffer_duration_seconds = (float)BUFFER_SIZE / (float)FS;
    this->heart_rate = (float)beat_count * 60.0f / buffer_duration_seconds;
    this->is_valid_heart_rate = (this->heart_rate > 40 && this->heart_rate < 150);

    // Invalidate results if they are out of a reasonable range
    if (!this->is_valid_heart_rate) this->heart_rate = 0;
    if (!this->is_valid_spo2) this->spo2 = 0;
}

// Getter for SpO2
float SpO2Algorithm::get_spo2() {
    return this->spo2;
}

// Getter for Heart Rate
float SpO2Algorithm::get_heart_rate() {
    return this->heart_rate;
}