#ifndef IOU_H
#define IOU_H
#include <string>
#include <vector>
#include <torch/script.h>
#include <torch/torch.h>
#include <torchvision/vision.h>
#include <torch/data.h>
#include <opencv2/opencv.hpp>
#include "../af_include/platform_def.h"
struct base_config {
  std::vector<float> row_anchor, col_anchor;
  int train_img_width, train_img_height;
};
class AFG_EXPORT iou_cal{
  torch::jit::script::Module model;
  base_config& _config;
public:
  iou_cal(std::string& model_name,base_config& config);
  ~iou_cal();
  float iou_from_image(cv::Mat& image);

};
#endif