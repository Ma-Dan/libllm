// The MIT License (MIT)
//
// Copyright (c) 2023 Xiaoyang Chen
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include "libllm/tensor.h"

namespace libllm {
namespace op {
namespace cuda {

enum class MapReduceType {
  // Sum of exp(x). FP16_FP32 means the input type if fp16, intermediate and output type is fp32.
  SUM_EXP_FP16_FP32,

  // Sum of x^2.
  SUM_SQUARE_FP16_FP32,

  // Sum of x.
  SUM_FP16_FP32,

  // Get maximun number in list.
  MAX
};

Tensor reduce(Tensor A, MapReduceType reduceType);
Tensor reduceHalfToSingle3D(Tensor A, MapReduceType reduceType);
Tensor reduceHalf3D(Tensor A, MapReduceType reduceType);

}  // namespace cuda
}  // namespace op
}  // namespace libllm
