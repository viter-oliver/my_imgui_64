#include "iou_cal.h"
using namespace std;
using namespace c10::ivalue;
using lane_set = vector<vector<cv::Point>>;
iou_cal::iou_cal(std::string& model_name, base_config& config):_config(config) {
  model = torch::jit::load(model_name);
  //torch::data::transforms::TensorTransform;
  
}
iou_cal::~iou_cal()
{
}
enum {
  hsv_black,
  hsv_gray,
  hsv_white,
  hsv_red,
  hsv_orange,
  hsv_yellow,
  hsv_green,
  hsv_cyan,
  hsv_blue,
  hsv_purple,
  hsv_col_cnt
};
struct {
  vector<int> lower,upper;
}hsv_hold[hsv_col_cnt] = {
  //black
  {{0,0,0},{180,255,46}},
  //gray
  {{0,0,46},{180,43,220}},
  //white
  {{0,0,221},{180,30,255}},
  //red
  {{0,43,46},{10,255,255}},
  //orange
  {{11,43,46},{25,255,255}},
  //yellow
  {{26,43,46},{34,255,255}},
  //green
  {{35,43,46},{77,255,255}},
  //cyan
  {{78,43,46},{99,255,255}},
  //blue
  {{100,43,46},{124,255,255}},
  //purple
  {{125,43,46},{155,255,255}},
};
void binary(cv::Mat& image,int color_id, cv::Mat& bin) {
  cv::InputArray& lower = hsv_hold[color_id].lower;
  cv::InputArray& upper= hsv_hold[color_id].upper;
  auto ROI = image({ 502,652 }, { 905,1377 });
  cv::Mat hsv;
  cv::cvtColor(ROI, hsv,cv::COLOR_BGR2HSV);
  cv::Mat mask;
  cv::inRange(hsv, lower, upper, mask);
  double black = 0, white = 255;
  cv::threshold(mask, bin, black, white, cv::THRESH_BINARY);
}

void pre2coords(c10::IValue& pred,
  vector<float>& row_anchor,
  vector<float>& col_anchor,
  lane_set& lanes,
  int o_width,int o_height,
  int loc_width=1) {
  auto& pred_t = pred.toGenericDict();
  auto& loc_row=pred_t.at("loc_row").toTensor().cpu();
  auto& loc_col = pred_t.at("loc_col").toTensor().cpu();
  auto loc_row_shape=loc_row.sizes(),loc_col_shape = loc_col.sizes();
  auto num_grid_row = loc_row_shape[1], num_cls_row=loc_row_shape[2];
  auto num_grid_col = loc_col_shape[1], num_cls_col = loc_col_shape[2];
  auto& max_indices_row = pred_t.at("loc_row").toTensor().argmax(1).cpu();
  auto& max_indices_col = pred_t.at("loc_col").toTensor().argmax(1).cpu();
  auto& valid_row = pred_t.at("exist_row").toTensor().argmax(1).cpu();
  auto& valid_col = pred_t.at("exist_col").toTensor().argmax(1).cpu();
  auto ncr = num_cls_row / 2;
  auto ncc = num_cls_col / 4;
  for (auto i : { 1,2 }) {
    auto sm_i = valid_row[0].index({ torch::indexing::Slice(), i }).sum();
    if (sm_i.item().toInt() > ncr) {
      vector<cv::Point> cur_tmp;
      for (auto k = 0; k < valid_row.sizes()[1]; k++) {
        if (valid_row[0, k, i].item().toInt()) {
          auto rg_start = max(0, max_indices_row[0, k, i].item().toInt() - loc_width);
          auto rg_end = min((int)num_grid_row - 1, max_indices_row[0, k, i].item().toInt()+loc_width)+1;
          auto all_indices = torch::tensor({rg_start,rg_end});
          auto out_tmp = loc_row[0, all_indices, k, 1].softmax(0) * all_indices;
          out_tmp= out_tmp.sum() + 0.5;
          out_tmp = out_tmp / (num_grid_row - 1) * o_width;
          cur_tmp.push_back({out_tmp.item().toInt(),row_anchor[k]*o_height});
        }
      }
      lanes.push_back(cur_tmp);
      if (lanes.size() == 2) return;
    }
  }
  for (auto i : { 0,3 }) {
    auto sm_i = valid_col[0].index({ torch::indexing::Slice(), i }).sum();
    if (sm_i.item().toInt() > ncc) {
      vector<cv::Point> cur_tmp;
      for (auto k = 0; k < valid_col.sizes()[1]; k++) {
        if (valid_col[0, k, i].item().toInt()) {
          auto rg_start = max(0, max_indices_col[0, k, i].item().toInt() - loc_width);
          auto rg_end = min((int)num_grid_col - 1, max_indices_col[0, k, i].item().toInt()+loc_width)+1;
          auto all_indices = torch::tensor({ rg_start,rg_end });
          auto out_tmp = loc_row[0, all_indices, k, 1].softmax(0) * all_indices;
          out_tmp = out_tmp.sum() + 0.5;
          out_tmp = out_tmp / (num_grid_row - 1) * o_width;
          cur_tmp.push_back({ out_tmp.item().toInt(),row_anchor[k] * o_height });
        }
      }
      lanes.push_back(cur_tmp);
      if (lanes.size() == 2) return;
    }
  }
  
}

float iou_cal::iou_from_image(cv::Mat& image)
{
  cv::Mat img_input;
  cv::resize(image, img_input, { _config.train_img_width,_config.train_img_height });
  cv::Mat img_rgb;
  cv::cvtColor(img_input, img_rgb, cv::COLOR_BGR2RGB);
  
  auto tensor_img = torch::from_blob(img_rgb.data, { img_rgb.rows,img_rgb.cols,3 }, torch::kByte);
  tensor_img = tensor_img.permute({ 2,0,1 });//调整通道顺序
  tensor_img = tensor_img.toType(torch::kFloat);
  tensor_img = tensor_img.div(255);//归一化
  auto o_width = image.cols;
  auto o_height = image.rows;
  
  auto pred = model({ tensor_img.cuda()});
  //auto pred_tens=model.forward(pic_erased).toTensor();

  lane_set pre_lanes;
  pre2coords(pred, _config.row_anchor, _config.col_anchor, pre_lanes, o_width, o_height);
  if (pre_lanes.size() == 0)
    return 0.f;
  for (auto& lane : pre_lanes) {
    cv::polylines(img_rgb, lane, false, { 255,0,0 }, 5);
  }
  cv::Mat hud_bi,lane_bi;
  binary(image, hsv_green, hud_bi);
  binary(img_rgb, hsv_red,lane_bi);
  auto iou_cal = [](cv::Mat& bi1, cv::Mat& bi2) {
    auto sum1 = cv::countNonZero(bi1);
    auto sum2 = cv::countNonZero(bi2);
    cv::Mat mpl;
    cv::multiply(bi1, bi2,mpl);
    auto inter_sum = cv::countNonZero(mpl);
    auto union_sum = sum1 + sum2 - inter_sum;
    float iou = (float)inter_sum / (float)union_sum;
    return iou;
  };
  return iou_cal(hud_bi,lane_bi);
}
