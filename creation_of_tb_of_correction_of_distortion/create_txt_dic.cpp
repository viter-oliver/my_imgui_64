#include <functional>
#include <fstream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "create_txt_dic.h"

using namespace std;
using namespace txt_dic;
/**
      img=imread
         |
         spilt
         |
     ----------
     |        |
   ori_red  ori_green
     |        |
    get_img_binary
     |        |
  red_th   green_th
     |        |
       erode
     |        |
   er_red   er_green
     |        |
    findContours
    |          |
contours_red contours_green
    |          |
 cut_contour_2_sides
    |          |
 sides_red  sides_green
    |          |
  sort_sides_by_mid
    sort_by_xmin
    |          |
   combine_2_sides
         |
      tmp_sides
         |
combine_midpt_of_side_2_curve
   |             |
 h_lines      v_lines
      |          |
      smooth_curve
      |          |
      approxPolyDP
      |          |
  ah_lines   av_lines
      |          |
      offset_curve
      |          |
      extend_curve
      |          |
    fill_points_2_gap
      |          |
   bh_lines  bv_lines
      |          |
          f_its
            |
  hlines_ids vlines_ids top_sd_left left_sd_top
          |
       _ref_sides       _blocks
          |                 |
  find ideal rectangle      |
          |                 |
  top bottom left right     |
                         txt_dic
                            |
                        get rid of empty points
*/
void create_txt_dic(std::string& img_file,int txt_width,int txt_height,int block_size, std::vector<txt_dic::dic_uint>& txt_dic_o,std::function<void(int)> progress ){
  using namespace txt_dic;
  string dir_img_file=img_file.substr(0, img_file.find_last_of('\\') + 1);
  int bk_x_cnt = txt_width / block_size,bk_y_cnt=txt_height/block_size;
  auto img = cv::imread(img_file);
  cv::Mat ele = getStructuringElement(MORPH_RECT, { 5,5 });
  vector<Mat> channels;
  split(img, channels);
  auto& blue = channels[0];
  auto& green = channels[1];
  auto& red = channels[2];
  progress(1);
  Mat green_th;
  get_average_gray_img_binary(green, green_th);
  Mat erode_green_th;
  cv::erode(green_th, erode_green_th, ele, { -1,-1 }, 3);
  Mat red_th;
  get_average_gray_img_binary(red, red_th);
  Mat erode_red_th;
  cv::erode(red_th, erode_red_th, ele, { -1,-1 }, 2);
  progress(2);
  contour_list contours_red, contours_green;
  cv::findContours(erode_red_th, contours_red, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
  cv::findContours(erode_green_th, contours_green, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
  segments sides_red, sides_green, tmp_sides;
  cut_contour_2_sides(contours_red, sides_red);
  cut_contour_2_sides(contours_green, sides_green);
  progress(3);
  sort_sides_by_mid(sides_red);
  sort_sides_by_mid(sides_green);
  std::vector<uint> lines_id_range;
  cal_range(lines_id_range,bk_x_cnt,bk_y_cnt);
  sort_by_xmin(sides_red, lines_id_range);
  sort_by_xmin(sides_green, lines_id_range);
  progress(4);
  uint end_row_id = lines_id_range[bk_y_cnt];
  uint start_col_id = lines_id_range[bk_y_cnt + 1];
  combine_2_sements(lines_id_range, sides_red, sides_green, tmp_sides, bk_x_cnt, end_row_id, start_col_id);
  const int hlines_cnt = bk_y_cnt + 1, vlines_cnt = bk_x_cnt + 1;
  vector<curve> h_lines(hlines_cnt), v_lines(bk_x_cnt + 1);
  for (int ix = 0; ix < hlines_cnt; ix++) {
    auto col_start = ix * bk_x_cnt, col_end = (ix + 1) * bk_x_cnt;
    auto& hL = h_lines[ix];
    for (int icol = col_start; icol < col_end; icol++) {
      auto end = icol == col_end - 1;
      get_curve_from_side_by_middle_point(hL, tmp_sides[icol], end, false);
    }
  }
  for (int ix = 0; ix < bk_x_cnt + 1; ix++) {
    auto& vL = v_lines[ix];
    for (int irow = 0; irow < bk_y_cnt; irow++) {
      auto sid = end_row_id + ix + irow * (bk_x_cnt + 1);
      auto end = irow == (bk_y_cnt-1);//last side 
      get_curve_from_side_by_middle_point(vL, tmp_sides[sid], end, true);
    }
  }
  progress(5);
  vector<curve> ah_lines(hlines_cnt), av_lines(vlines_cnt);
  double epilon = 7;
  //get ah_lines and av_lines by smoothing h_lines and v_lines forwardly
  for (int ix = 0; ix < ah_lines.size(); ix++) {
    smooth_curve(h_lines[ix], en_5, false);
    cv::approxPolyDP(h_lines[ix], ah_lines[ix], epilon, false);
  }
  for (int ix = 0; ix < av_lines.size(); ix++) {
    smooth_curve(v_lines[ix], en_5, true);
    cv::approxPolyDP(v_lines[ix], av_lines[ix], epilon, false);
  }
  //offset and expend curves for covering whole valid area
  offset_curve(-5, av_lines[0]);
  offset_curve(5, av_lines[av_lines.size() - 1]);
  for (auto& cv : av_lines) {
    extend_curve(20, cv, false);
  }
  offset_curve(-5, ah_lines[0], false);
  offset_curve(5, ah_lines[ah_lines.size() - 1], false);
  for (auto& cv : ah_lines) {
    extend_curve(20, cv);
  }
   progress(6);
  //we get a set of lines in which ervery points are contigous
  vector<curve> bh_lines(hlines_cnt), bv_lines(vlines_cnt);
  get_lines_by_fill_points_2_gap(ah_lines, bh_lines);
  get_lines_by_fill_points_2_gap(av_lines, bv_lines);
  progress(7);
  //create _ref_sides and  _blocks which will be used for creating texture dictinary
  segments _ref_sides;
  vector<block> _blocks;
  vector<int> hlines_ids(hlines_cnt, 0), vlines_ids(vlines_cnt, 0);
  side* top_sd_left = new side[hlines_cnt], * left_sd_top = new side[vlines_cnt];

  const auto sides_cnt = vlines_cnt * (hlines_cnt - 1) + hlines_cnt * (vlines_cnt - 1);
  _ref_sides.resize(sides_cnt);
  const auto  blk_cnt = (hlines_cnt - 1) * (vlines_cnt - 1);//number of blocks
  _blocks.resize(blk_cnt, { _ref_sides });
  const auto vside_base_id = hlines_cnt * (vlines_cnt - 1);
  for (int hcv_id = 0; hcv_id < hlines_cnt; hcv_id++) {
    auto& hcurve = bh_lines[hcv_id];
    auto& cur_hid = hlines_ids[hcv_id];
    side* ptop_sd = top_sd_left + hcv_id;
    for (int vcv_id = 0; vcv_id < vlines_cnt; vcv_id++) {
      auto& vcurve = bv_lines[vcv_id];
      auto& cur_vid = vlines_ids[vcv_id];
      side* pleft_sd = left_sd_top + vcv_id;
      if (vcv_id > 0) {
        auto prev_top_id = hcv_id * (vlines_cnt - 1) + vcv_id - 1;
        ptop_sd = &_ref_sides[prev_top_id];
      }
      if (hcv_id > 0) {
        auto prev_left_id = vside_base_id + (hcv_id - 1) * vlines_cnt + vcv_id;
        pleft_sd = &_ref_sides[prev_left_id];
      }
      find_intersection_2_curve_cutted_2_sides(hcurve, cur_hid, *ptop_sd, vcurve, cur_vid, *pleft_sd);
      if (hcv_id < hlines_cnt - 1 && vcv_id < vlines_cnt - 1) {
        auto bk_id = hcv_id * (vlines_cnt - 1) + vcv_id;
        auto& cur_bk = _blocks[bk_id];
        cur_bk.ori_0 = { vcv_id * block_size,hcv_id * block_size };
        cur_bk.top_side_id = hcv_id * (vlines_cnt - 1) + vcv_id;
        cur_bk.bottom_side_id = cur_bk.top_side_id + vlines_cnt - 1;
        cur_bk.left_side_id = vside_base_id + hcv_id * vlines_cnt + vcv_id;
        cur_bk.right_side_id = cur_bk.left_side_id + 1;
      }
    }
  }
  progress(8);
  //find ideal rectangle
  uint top{ 0 }, bottom{ uimax }, left{ 0 }, right{ uimax };
  for (int ix = 0; ix < vlines_cnt - 1; ix++) {
    auto& sd_top = _ref_sides[ix];
    if (sd_top.y_max > top) {
      top = sd_top.y_max;
    }
    auto bt_sd_id = blk_cnt + ix;
    auto& sd_bottom = _ref_sides[bt_sd_id];
    if (sd_bottom.y_min < bottom) {
      bottom = sd_bottom.y_min;
    }
  }

  for (int ix = 0; ix < hlines_cnt - 1; ix++) {
    auto edge_id = vside_base_id + ix * vlines_cnt;
    auto& sd = _ref_sides[edge_id];
    if (sd.x_max > left) {
      left = sd.x_max;
    }
    auto r_edge_id = edge_id + vlines_cnt - 1;
    auto& sd_r = _ref_sides[r_edge_id];
    if (sd_r.x_min < right) {
      right = sd_r.x_min;
    }
  }
  auto tw = right - left;
  auto th = bottom - top;
  const float w_over_h_rate = (float)txt_width / (float)txt_height;
  auto itw = w_over_h_rate * th;
  if (tw > itw) { //too wide
    auto ad_w = tw - itw;
    auto half_ad_w = ad_w / 2;
    left += half_ad_w;
    right -= half_ad_w;
  }
  else { // too narrow
    const float h_over_w_rate = (float)txt_height / (float)txt_width;
    auto ith = tw * h_over_w_rate;
    auto ad_h = th - ith;
    auto half_ad_h = ad_h / 2;
    top += half_ad_h;
    bottom -= half_ad_h;
  }
  tw = right - left;
  th = bottom - top;
  //create txture dictionary
  auto tb_sz = txt_height * txt_width;
  auto ideal_rect_sz = tw * th;
  float s_sz = tb_sz / (float)ideal_rect_sz;
  txt_dic_o.resize(tb_sz);
  uint vtop{ uimax }, vbottom{ 0 }, vleft{ uimax }, vright{ 0 };//valid area used for erasing empty points
  auto out_irect = [&](Point& pt) {
    bool be_outside = pt.x < left || pt.x>right || pt.y<top || pt.y>bottom;
    return be_outside;
  };
  for (int bk_id = 0; bk_id < _blocks.size(); bk_id++) {
    auto& bk = _blocks[bk_id];
    bool bk_out = bk.right_side_x_max() < left || bk.left_side_x_min() > right
      || bk.bottom_side_y_max() < top
      || bk.top_side_y_min() > bottom;
    if (bk_out) continue;
    auto& lcv = bk.left_side()._cv;
    auto lsz = lcv.size();
    auto& tal = bk.top_side()._alist;
    auto tsz = tal.size() + 1;
    auto& rcv = bk.right_side()._cv;
    uint16_t pre_off_y = 0;
    for (int sid = 0; sid < lcv.size(); sid++) {
      int tsid = 0;
      Point pt = lcv[sid];
      int pid = 0;
      auto curve_len = tsz;
      if (sid < rcv.size()) {
        auto& rpt = rcv[sid];
        curve_len = rpt.x - pt.x;
      }
      uint16_t off_y = sid * block_size / (float)lsz + 0.5f;
      while (pre_off_y <= off_y) {
        uint16_t pre_off_x = 0;
        auto trace_distorted_pt = [&](Point& pt) {
          if (!out_irect(pt)) {
            //计算源坐标，即table of dic_txt的目标索引
            uint16_t off_x = tsid * block_size / (float)curve_len + 0.5f;
            if (0 == pre_off_x) {
              pre_off_x = off_x;
            }
            while (pre_off_x <= off_x) {
              Point ori_off = { pre_off_x, pre_off_y };
              Point ori_from = bk.ori_0 + ori_off;//该点从这个地方过来
              if (ori_from.x < vleft) {
                vleft = ori_from.x;
              }
              if (ori_from.x > vright) {
                vright = ori_from.x;
              }
              if (ori_from.y < vtop) {
                vtop = ori_from.y;
              }
              if (ori_from.y > vbottom) {
                vbottom = ori_from.y;
              }
              //换算成原始尺寸的相对坐标
              auto i_off_x = pt.x - left;
              uint16_t o_off_x = txt_width * i_off_x / (float)tw + 0.5f;
              auto i_off_y = pt.y - top;
              uint16_t o_off_y = txt_height * i_off_y / (float)th + 0.5f;
              auto dic_id = ori_from.y * txt_width + ori_from.x;
              txt_dic_o[dic_id] = { o_off_x ,o_off_y };
              pre_off_x++;
            }

          }
        };
        do {
          trace_distorted_pt(pt);
          if (tsid < tal.size()) {
            auto ac = tal[tsid++];
            _move_fn[ac](pt);
          }
          else {
            pt.x++;
          }
          pid++;
        } while (pid < curve_len);
        pid = 0;
        pt = lcv[sid];
        tsid = 0;
        pre_off_y++;
      }
    }
  }
  //get rid of empty points
  for (int iy = vtop; iy <= vbottom; iy++) {
    for (int ix = vleft; ix <= vright; ix++) {
      auto pre_dic_id = iy * txt_width + ix - 1;
      auto dic_id = iy * txt_width + ix;
      auto next_dic_id = iy * txt_width + ix + 1;
      auto& pre_bk = txt_dic_o[pre_dic_id];
      auto& bk = txt_dic_o[dic_id];
      auto& next_bk = txt_dic_o[next_dic_id];
      if (bk.x == 0 && bk.y == 0
        && pre_bk.x != 0 && pre_bk.y != 0
        && next_bk.x != 0 && next_bk.y != 0
        ) {
        bk.y = (pre_bk.y + next_bk.y) / 2;
        bk.x = (pre_bk.x + next_bk.x) / 2;
      }
    }
  }
  progress(9);
  //create bins of texture dictionary
  string bin_prefile ="_"+std::to_string(txt_width) + "_" + std::to_string(txt_height);

  string txt_dic_bin = img_file + bin_prefile+ ".bin";
  ofstream of_txt_dic;
  of_txt_dic.open(txt_dic_bin, ios::binary);
  of_txt_dic.write((const char*)&txt_dic_o[0], sizeof(dic_uint)* txt_dic_o.size());
  of_txt_dic.close();
  delete[] top_sd_left;
  delete[] left_sd_top;
}