#include "GlucosePredictor.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "model_data.h" // 包含我们之前创建的模型占位符

// TFLite 命名空间
namespace {
    // TFLite 需要的组件
    tflite::ErrorReporter* error_reporter = nullptr;
    const tflite::Model* model = nullptr;
    tflite::MicroInterpreter* interpreter = nullptr;
    TfLiteTensor* input_tensor = nullptr;
    TfLiteTensor* output_tensor = nullptr;

    // Tensor Arena: TFLM运行时所需的内存池。
    // !!重要!! 这个大小需要根据您的模型进行调整。如果太小，程序会崩溃。
    // 您可以在模型加载后通过 interpreter->arena_used_bytes() 获取建议值。
    constexpr int kTensorArenaSize = 4 * 1024; // 先分配4KB
    uint8_t tensor_arena[kTensorArenaSize];
}

// 获取单例实例
GlucosePredictor& GlucosePredictor::getInstance() {
    static GlucosePredictor instance;
    return instance;
}

// 私有构造函数
GlucosePredictor::GlucosePredictor() :
    _is_initialized(false),
    _history_index(0),
    _history_count(0) 
{
}

bool GlucosePredictor::begin() {
    // 1. 设置错误报告器
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    // 2. 加载模型数据
    model = tflite::GetModel(g_model_data);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        error_reporter->Report("Model provided is schema version %d not equal to supported version %d.", model->version(), TFLITE_SCHEMA_VERSION);
        return false;
    }

    // 3. 创建OpResolver，它包含了模型所需的操作(OP)的实现
    static tflite::AllOpsResolver resolver;

    // 4. 实例化解释器
    static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    // 5. 在内存池(Tensor Arena)中为模型的输入输出张量分配内存
    if (interpreter->AllocateTensors() != kTfLiteOk) {
        error_reporter->Report("AllocateTensors() failed.");
        return false;
    }

    // 6. 获取指向输入和输出张量的指针
    input_tensor = interpreter->input(0);
    output_tensor = interpreter->output(0);
    
    // 验证输入/输出张量的格式是否符合预期
    // 假设模型输入是 [1, 10] 的浮点数，输出是 [1, 1] 的浮点数
    if (input_tensor->dims->size != 2 || input_tensor->dims->data[0] != 1 || input_tensor->dims->data[1] != kHistorySize || input_tensor->type != kTfLiteFloat32) {
        error_reporter->Report("Bad input tensor parameters.");
        return false;
    }

    _is_initialized = true;
    return true;
}

void GlucosePredictor::addGlucoseReading(float value) {
    _history_buffer[_history_index] = value;
    _history_index = (_history_index + 1) % kHistorySize;
    if (_history_count < kHistorySize) {
        _history_count++;
    }
}

bool GlucosePredictor::isReadyToPredict() const {
    return _history_count >= kHistorySize;
}

float GlucosePredictor::predict() {
    if (!_is_initialized || !isReadyToPredict()) {
        return 0.0f; // 返回一个无效值
    }

    // 将环形缓冲区中的数据按正确的顺序填充到模型的输入张量中
    int current_idx = _history_index;
    for (int i = 0; i < kHistorySize; ++i) {
        // 计算在环形缓冲区中的实际索引
        int buffer_idx = (current_idx + i) % kHistorySize;
        input_tensor->data.f[i] = _history_buffer[buffer_idx];
    }

    // 运行推理
    if (interpreter->Invoke() != kTfLiteOk) {
        error_reporter->Report("Invoke failed.");
        return 0.0f;
    }

    // 从输出张量中获取预测结果
    float prediction = output_tensor->data.f[0];
    return prediction;
}