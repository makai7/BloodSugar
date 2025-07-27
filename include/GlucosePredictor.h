#ifndef GLUCOSE_PREDICTOR_H
#define GLUCOSE_PREDICTOR_H

/**
 * @class GlucosePredictor
 * @brief 使用TensorFlow Lite模型进行血糖趋势预测。
 * * 采用单例模式。
 */
class GlucosePredictor {
public:
    /**
     * @brief 获取GlucosePredictor的全局唯一实例。
     */
    static GlucosePredictor& getInstance();
    
    // 禁止拷贝
    GlucosePredictor(const GlucosePredictor&) = delete;
    GlucosePredictor& operator=(const GlucosePredictor&) = delete;

    /**
     * @brief 初始化TFLite解释器。
     * * 加载模型，分配内存（Tensor Arena）。
     * @return bool - 初始化成功返回true。
     */
    bool begin();

    /**
     * @brief 向历史数据缓冲区中添加一个新的血糖读数。
     * @param value 最新的血糖测量值。
     */
    void addGlucoseReading(float value);

    /**
     * @brief 检查是否已收集到足够的历史数据以进行预测。
     * @return bool - 如果可以进行预测，返回true。
     */
    bool isReadyToPredict() const;

    /**
     * @brief 运行模型进行一次预测。
     * @return float - 预测出的未来血糖值。如果无法预测，返回0.0。
     */
    float predict();

private:
    // 私有构造函数
    GlucosePredictor(); 

    bool _is_initialized;

    // --- 历史数据缓冲区 (使用环形缓冲区实现) ---
    // 假设模型需要10个历史数据点作为输入
    static constexpr int kHistorySize = 10; 
    float _history_buffer[kHistorySize];
    int _history_index;
    int _history_count;
};

#endif // GLUCOSE_PREDICTOR_H