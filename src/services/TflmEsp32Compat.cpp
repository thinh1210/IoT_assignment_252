#include <Arduino.h>

#include "Common/AppLog.h"
#include "tensorflow/lite/c/builtin_op_data.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/kernels/internal/reference/fully_connected.h"
#include "tensorflow/lite/kernels/internal/reference/integer_ops/fully_connected.h"
#include "tensorflow/lite/kernels/internal/reference/softmax.h"
#include "tensorflow/lite/micro/kernels/fully_connected.h"
#include "tensorflow/lite/micro/kernels/kernel_util.h"
#include "tensorflow/lite/micro/kernels/softmax.h"
#include "tensorflow/lite/micro/micro_context.h"
#include "tensorflow/lite/micro/micro_log.h"

extern "C" void DebugLog(const char *message) {
  AppLog::writeRaw(message);
}

namespace {

struct ReferenceFullyConnectedOpData {
  tflite::OpDataFullyConnected quantized_data;
  TfLiteFusedActivation activation;
};

void *ReferenceFullyConnectedInit(TfLiteContext *context, const char *buffer,
                                  size_t length) {
  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  return context->AllocatePersistentBuffer(
      context, sizeof(ReferenceFullyConnectedOpData));
}

TfLiteStatus ReferenceFullyConnectedPrepare(TfLiteContext *context,
                                            TfLiteNode *node) {
  auto *op_data = static_cast<ReferenceFullyConnectedOpData *>(node->user_data);
  const auto *params =
      static_cast<const TfLiteFullyConnectedParams *>(node->builtin_data);

  tflite::MicroContext *micro_context = tflite::GetMicroContext(context);
  TfLiteTensor *input = micro_context->AllocateTempInputTensor(
      node, tflite::kFullyConnectedInputTensor);
  TF_LITE_ENSURE(context, input != nullptr);
  TfLiteTensor *filter = micro_context->AllocateTempInputTensor(
      node, tflite::kFullyConnectedWeightsTensor);
  TF_LITE_ENSURE(context, filter != nullptr);
  TfLiteTensor *bias = micro_context->AllocateTempInputTensor(
      node, tflite::kFullyConnectedBiasTensor);
  TfLiteTensor *output = micro_context->AllocateTempOutputTensor(
      node, tflite::kFullyConnectedOutputTensor);
  TF_LITE_ENSURE(context, output != nullptr);

  op_data->activation = params->activation;
  TfLiteStatus status = tflite::CalculateOpDataFullyConnected(
      context, params->activation, input->type, input, filter, bias, output,
      &op_data->quantized_data);

  micro_context->DeallocateTempTfLiteTensor(output);
  micro_context->DeallocateTempTfLiteTensor(input);
  micro_context->DeallocateTempTfLiteTensor(filter);
  if (bias != nullptr) {
    micro_context->DeallocateTempTfLiteTensor(bias);
  }

  return status;
}

TfLiteStatus ReferenceFullyConnectedEval(TfLiteContext *context,
                                         TfLiteNode *node) {
  const auto *op_data =
      static_cast<const ReferenceFullyConnectedOpData *>(node->user_data);
  const TfLiteEvalTensor *input = tflite::micro::GetEvalInput(
      context, node, tflite::kFullyConnectedInputTensor);
  const TfLiteEvalTensor *filter = tflite::micro::GetEvalInput(
      context, node, tflite::kFullyConnectedWeightsTensor);
  const TfLiteEvalTensor *bias = tflite::micro::GetEvalInput(
      context, node, tflite::kFullyConnectedBiasTensor);
  TfLiteEvalTensor *output = tflite::micro::GetEvalOutput(
      context, node, tflite::kFullyConnectedOutputTensor);

  switch (input->type) {
  case kTfLiteFloat32: {
    const tflite::FullyConnectedParams params =
        tflite::FullyConnectedParamsFloat(op_data->activation);
    tflite::reference_ops::FullyConnected(
        params, tflite::micro::GetTensorShape(input),
        tflite::micro::GetTensorData<float>(input),
        tflite::micro::GetTensorShape(filter),
        tflite::micro::GetTensorData<float>(filter),
        tflite::micro::GetTensorShape(bias),
        tflite::micro::GetOptionalTensorData<float>(bias),
        tflite::micro::GetTensorShape(output),
        tflite::micro::GetTensorData<float>(output));
    return kTfLiteOk;
  }

  case kTfLiteInt8: {
    const tflite::FullyConnectedParams params =
        tflite::FullyConnectedParamsQuantized(op_data->quantized_data);
    tflite::reference_integer_ops::FullyConnected(
        params, tflite::micro::GetTensorShape(input),
        tflite::micro::GetTensorData<int8_t>(input),
        tflite::micro::GetTensorShape(filter),
        tflite::micro::GetTensorData<int8_t>(filter),
        tflite::micro::GetTensorShape(bias),
        tflite::micro::GetOptionalTensorData<int32_t>(bias),
        tflite::micro::GetTensorShape(output),
        tflite::micro::GetTensorData<int8_t>(output));
    return kTfLiteOk;
  }

  default:
    MicroPrintf("Unsupported fully connected tensor type: %d", input->type);
    return kTfLiteError;
  }
}

TfLiteStatus ReferenceSoftmaxEval(TfLiteContext *context, TfLiteNode *node) {
  const auto *params = static_cast<const tflite::SoftmaxParams *>(node->user_data);
  const TfLiteEvalTensor *input = tflite::micro::GetEvalInput(context, node, 0);
  TfLiteEvalTensor *output = tflite::micro::GetEvalOutput(context, node, 0);

  switch (input->type) {
  case kTfLiteFloat32:
    tflite::reference_ops::Softmax(
        *params, tflite::micro::GetTensorShape(input),
        tflite::micro::GetTensorData<float>(input),
        tflite::micro::GetTensorShape(output),
        tflite::micro::GetTensorData<float>(output));
    return kTfLiteOk;

  case kTfLiteInt8:
    if (output->type == kTfLiteInt8) {
      tflite::reference_ops::Softmax(
          *params, tflite::micro::GetTensorShape(input),
          tflite::micro::GetTensorData<int8_t>(input),
          tflite::micro::GetTensorShape(output),
          tflite::micro::GetTensorData<int8_t>(output));
      return kTfLiteOk;
    }
    if (output->type == kTfLiteInt16) {
      tflite::reference_ops::Softmax(
          *params, tflite::micro::GetTensorShape(input),
          tflite::micro::GetTensorData<int8_t>(input),
          tflite::micro::GetTensorShape(output),
          tflite::micro::GetTensorData<int16_t>(output));
      return kTfLiteOk;
    }
    break;

  case kTfLiteInt16:
    if (output->type == kTfLiteInt16) {
      tflite::reference_ops::SoftmaxInt16(
          *params, tflite::micro::GetTensorShape(input),
          tflite::micro::GetTensorData<int16_t>(input),
          tflite::micro::GetTensorShape(output),
          tflite::micro::GetTensorData<int16_t>(output));
      return kTfLiteOk;
    }
    break;

  default:
    break;
  }

  MicroPrintf("Unsupported softmax input/output types: %d -> %d", input->type,
              output->type);
  return kTfLiteError;
}

} // namespace

namespace tflite {

TfLiteRegistration Register_FULLY_CONNECTED() {
  return tflite::micro::RegisterOp(ReferenceFullyConnectedInit,
                                   ReferenceFullyConnectedPrepare,
                                   ReferenceFullyConnectedEval);
}

TfLiteRegistration Register_SOFTMAX() {
  return tflite::micro::RegisterOp(SoftmaxInit, SoftmaxPrepare,
                                   ReferenceSoftmaxEval);
}

} // namespace tflite
