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

// C++ wrapper for libllm C API.

#pragma once

#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include "llm.h"

namespace llm {

class Model;
class ModelFactory;
class Completion;

enum class DeviceType { CPU = LLM_DEVICE_CPU, CUDA = LLM_DEVICE_CUDA, AUTO = LLM_DEVICE_AUTO };

// configuration for LLM completion task.
class CompletionConfig {
 public:
  CompletionConfig()
      : _topP(0.8f),
        _topK(50),
        _temperature(1.0f) {
  }

  // setters for the config.
  void setTopP(float topP) {
    _topP = topP;
  }
  void setTopK(int topK) {
    _topK = topK;
  }
  void setTemperature(float temperature) {
    _temperature = temperature;
  }

  // getters for the config.
  float getTopP() const {
    return _topP;
  }
  int getTopK() const {
    return _topK;
  }
  float getTemperature() const {
    return _temperature;
  }

 private:
  float _topP;
  int _topK;
  float _temperature;
};

class Chunk {
 public:
  friend class Completion;

  std::string getText() const {
    return _text;
  }

 private:
  std::string _text;
};

/// @brief Store the state of ongoing completion task.
class Completion {
 public:
  friend class Model;

  /// @brief If completion is ongoing (active) returns true, if stopped returns false.
  /// @return If completion is active.
  bool isActive();

  /// @brief Get the next chunk of tokens generated by the model.
  /// @return instance of Chunk.
  Chunk nextChunk();

 private:
  std::shared_ptr<llmCompletion_t> _handle;
  std::shared_ptr<llmChunk_t> _chunkHandle;
  std::shared_ptr<Model> _model;

  Completion() {
  }
};

/// @brief Input prompt for Model::comeplete().
class Prompt {
 public:
  Prompt(Prompt &) = delete;
  Prompt &operator=(Prompt &) = delete;

  /// @brief Create a prompt from model.
  static std::shared_ptr<Prompt> fromModel(std::shared_ptr<Model> model);

  /// @brief Append text to the prompt.
  /// @param text text to append.
  void appendText(const std::string &text);

  /// @brief Append a control token to the prompt.
  /// @param text name of the control token.
  void appendControlToken(const std::string &text);

  /// @brief Get internal handle for the prompt.
  /// @return A pointer of llmPrompt_t.
  llmPrompt_t *getHandle() {
    return _handle.get();
  }

 private:
  std::shared_ptr<llmPrompt_t> _handle;
  std::shared_ptr<Model> _model;

  Prompt() = default;
};

/// @brief Stores an instance of LLM Model.
class Model : public std::enable_shared_from_this<Model> {
 public:
  Model(Model &) = delete;
  Model &operator=(Model &) = delete;

  /// @brief Create an instance of Model from the package file;
  /// @param configFile config file of the model.
  /// @param device device of the model storage and computation device. Use DeviceType::AUTO to
  /// let libllm determine the best one.
  /// @return A shared pointer of the Model instance.
  static std::shared_ptr<Model> fromFile(
      const std::string &filename,
      DeviceType device = DeviceType::AUTO);

  /// @brief Get the name of model, for example, "llama".
  /// @return name of the model.
  std::string getName();

  /// @brief Complete the string version of given `prompt` with LLM.
  /// @param prompt The prompt to complete.
  /// @param config The config for completion.
  /// @return A `Completion` object.
  std::shared_ptr<Completion> complete(
      std::shared_ptr<Prompt> prompt,
      CompletionConfig config = CompletionConfig());

  /// @brief Get internal handle for the model.
  /// @return A pointer of llmModel_t.
  llmModel_t *getHandle() {
    return _handle.get();
  }

 private:
  std::shared_ptr<llmModel_t> _handle;

  Model() = default;
};

// -- Implementation of libLLM C++ API (wrapper for C api) ----------------------------------------

namespace internal {

inline void throwLastError() {
  std::string lastError = llmGetLastErrorMessage();
  throw std::runtime_error(lastError);
}

}  // namespace internal

// -- Completion ----------

inline bool Completion::isActive() {
  return llmCompletion_IsActive(_handle.get()) != 0;
}

inline Chunk Completion::nextChunk() {
  if (LLM_OK != llmCompletion_GenerateNextChunk(_handle.get(), _chunkHandle.get())) {
    internal::throwLastError();
  }

  const char *text = llmChunk_GetText(_chunkHandle.get());
  if (!text) internal::throwLastError();

  Chunk c;
  c._text = text;
  return c;
}

// -- Prompt ----------

inline std::shared_ptr<Prompt> Prompt::fromModel(std::shared_ptr<Model> model) {
  std::shared_ptr<llmPrompt_t> pPrompt(llmPrompt_New(model->getHandle()), llmPrompt_Delete);
  if (!pPrompt) internal::throwLastError();

  std::shared_ptr<Prompt> prompt{new Prompt()};
  prompt->_model = model;
  prompt->_handle = pPrompt;

  return prompt;
}

inline void Prompt::appendText(const std::string &text) {
  if (LLM_OK != llmPrompt_AppendText(_handle.get(), text.c_str())) {
    internal::throwLastError();
  }
}

inline void Prompt::appendControlToken(const std::string &name) {
  if (LLM_OK != llmPrompt_AppendControlToken(_handle.get(), name.c_str())) {
    internal::throwLastError();
  }
}

// -- Model ----------

inline std::shared_ptr<Model> Model::fromFile(const std::string &config, DeviceType device) {
  std::shared_ptr<llmModel_t> pModel(llmModel_New(), llmModel_Delete);
  int32_t dwDevice = static_cast<int32_t>(device);
  if (LLM_OK != llmModel_SetFile(pModel.get(), config.c_str())) internal::throwLastError();
  if (LLM_OK != llmModel_SetDevice(pModel.get(), dwDevice)) internal::throwLastError();
  if (LLM_OK != llmModel_Load(pModel.get())) internal::throwLastError();

  std::shared_ptr<Model> model{new Model()};
  model->_handle = pModel;

  return model;
}

inline std::string Model::getName() {
  const char *name = llmModel_GetName(_handle.get());
  if (!name) internal::throwLastError();

  return name;
}

inline std::shared_ptr<Completion> Model::complete(
    std::shared_ptr<Prompt> prompt,
    CompletionConfig config) {
  std::shared_ptr<llmCompletion_t> pComp(llmCompletion_New(_handle.get()), llmCompletion_Delete);
  if (!pComp) internal::throwLastError();

  if (LLM_OK != llmCompletion_SetPrompt(pComp.get(), prompt->getHandle()))
    internal::throwLastError();
  if (LLM_OK != llmCompletion_SetTopK(pComp.get(), config.getTopK())) internal::throwLastError();
  if (LLM_OK != llmCompletion_SetTopP(pComp.get(), config.getTopP())) internal::throwLastError();
  if (LLM_OK != llmCompletion_SetTemperature(pComp.get(), config.getTemperature())) {
    internal::throwLastError();
  }
  if (LLM_OK != llmCompletion_Start(pComp.get())) internal::throwLastError();

  std::shared_ptr<llmChunk_t> pChunk(llmChunk_New(), llmChunk_Delete);
  std::shared_ptr<Completion> comp{new Completion()};
  comp->_model = shared_from_this();
  comp->_chunkHandle = pChunk;
  comp->_handle = pComp;

  return comp;
}

}  // namespace llm