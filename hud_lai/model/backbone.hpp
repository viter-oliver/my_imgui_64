#pragma once
#include <string>
#include <torch/script.h>
#include <torch/torch.h>
#include <torchvision/vision.h>
class resnet : public torch::nn::Module {
public:
  resnet(std::string layer_str, bool pretrained = false) {
    torch::jit::script::Module model;
    std::string load_path = net_path;
    if (layer_str == "18") {
      load_path+="/resnet18.pt";
      model = torch::jit::load(load_path);
    }
  }
};
