/*
    Copyright (c) 2016, Taiga Nomi
    All rights reserved.

    Use of this source code is governed by a BSD-style license that can be found
    in the LICENSE file.
*/
#pragma once
#include "tiny_dnn/activations/activation_function.h"
#include "tiny_dnn/layers/layer.h"

namespace tiny_dnn {

/**
 * single-input, single-output network with activation function
 **/
template <typename Activation>
class feedforward_layer : public layer {
 public:
  explicit feedforward_layer(const std::vector<vector_type> &in_data_type)
    : layer(in_data_type, std_output_order(true)) {}
  activation::function &activation_function() { return h_; }
  std::pair<float_t, float_t> out_value_range() const override {
    return h_.scale();
  }

 public:
  void forward_activation(tensor_t &a_tensor, tensor_t &out_tensor) {
    serial_size_t out_dim = out_shape()[0].size();

    for_i(a_tensor.size(), [&](int sample) {
      vec_t &out = a_tensor[sample];
      vec_t &a   = out_tensor[sample];
      out.resize(out_dim);
      a.resize(out_dim);
      h_.itef(out, a, out_dim);
    });
  }

  void backward_activation(const tensor_t &prev_delta,
                           const tensor_t &this_out,
                           tensor_t &curr_delta) {
    // @todo consider parallelism
    for_i(this_out.size(), [&](serial_size_t sample) {
      const vec_t &out_vec        = this_out[sample];
      const vec_t &prev_delta_vec = prev_delta[sample];
      vec_t &curr_delta_vec       = curr_delta[sample];

      const serial_size_t len =
        static_cast<serial_size_t>(prev_delta_vec.size());

      if (h_.one_hot()) {
        h_.itedf(curr_delta_vec, prev_delta_vec, out_vec, len);
      } else {
        for (serial_size_t c = 0; c < len; c++) {
          vec_t df          = h_.df(out_vec, c);
          curr_delta_vec[c] = vectorize::dot(&prev_delta_vec[0], &df[0], len);
        }
      }
    });
  }

  Activation h_;
};

}  // namespace tiny_dnn
