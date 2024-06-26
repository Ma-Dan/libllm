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

#include <stdio.h>

#include <iostream>
#include <string>

#include "libllm/dialog_manager.h"
#include "libllm/llm.h"
#include "libllm/lut/error.h"
#include "libllm/lut/flags.h"
#include "libllm/lut/strings.h"
#include "libllm/lut/time.h"

using libllm::ChatOutput;
using libllm::DialogManager;
using libllm::PromptBulder;

/// @brief Print the chat statistical data.
/// @param chatOutput Output from chat.
void printChatStat(const ChatOutput &chatOutput) {
  double msPerToken = 1000 * chatOutput.answerDuration / chatOutput.numAnswerTokens;
  std::cout << std::endl
            << lut::sprintf(
                   "(%d token, time=%.2fs, %.2fms per token)",
                   chatOutput.numAnswerTokens,
                   chatOutput.answerDuration,
                   msPerToken)
            << std::endl;
}

int main(int argc, char **argv) {
  std::string configPath;
  std::string deviceType = "auto";

  const char *usage =
      "Command line interface for libllm.\n"
      "Usage: llm -m <llmpkg-file> [-d (cpu|gpu|cuda)]";

  lut::Flags flags(usage);
  flags.define("-m", &configPath, "filename of libllm config file.");
  flags.define("-d", &deviceType, "device of the model. (cpu|cuda|auto)");
  flags.parse(argc, argv);

  if (configPath.empty()) {
    flags.printUsage();
    return 1;
  }

  llm::DeviceType device = llm::DeviceType::AUTO;
  if (deviceType == "auto")
    device = llm::DeviceType::AUTO;
  else if (deviceType == "cuda")
    device = llm::DeviceType::CUDA;
  else if (deviceType == "cpu")
    device = llm::DeviceType::CPU;
  else {
    printf("invali·d device");
    return 1;
  }

  if (llmInit(LLM_API_VERSION) != LLM_OK) {
    printf("init libllm failed: %s\n", llmGetLastErrorMessage());
    return 1;
  }

  std::shared_ptr<llm::Model> model = llm::Model::fromFile(configPath, device);
  std::shared_ptr<PromptBulder> promptBuilder = PromptBulder::create(model->getName());
  DialogManager dialogManager(model, promptBuilder);

  std::cout << "> ";

  std::string query;
  while (std::getline(std::cin, query)) {
    if (lut::trim(query) == "") continue;

    ChatOutput chatOutput = dialogManager.chat(query, [](const std::string &token) {
      std::cout << token;
      std::cout.flush();
    });

    printChatStat(chatOutput);
    std::cout << "> ";
  }

  return 0;
}
