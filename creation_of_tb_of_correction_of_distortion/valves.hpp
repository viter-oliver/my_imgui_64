#include "base.hpp"
namespace txt_dic {
using namespace cv;
uint get_average_gray(Mat& image) {
  const unsigned char SINGLE_CH = 1;

  int w = image.cols;
  int h = image.rows;
  int dims = image.channels();
  Mat dst = image.clone();
  uint Sumpv = 0;
  uint averane = 0;
  uint cnt_t = 1;

  if (dims > SINGLE_CH) {
    return 0;
  }
  else {
    for (int row = 0; row < h; row++) {
      uchar* current_row = dst.ptr<uchar>(row);
      for (int col = 0; col < w; col++) {
        int pv = *current_row;
        *current_row++ = 255 - pv;
        if (*current_row > 30) {
          Sumpv += *current_row;
          cnt_t++;
        }
      }
    }
    averane = Sumpv / cnt_t;
    //printf("value is %d\n", averane);
    //cv::waitKey(0);
    return averane;
  }
}
void get_average_gray_img_binary(Mat& image, Mat& b_image) {

  Mat gray = image.clone();
  uint w, h;
  w = gray.size().width;
  h = gray.size().height;

  //Crop Gray image
  const uint DIV_NUM = w * 0.003;
  Mat cropped_gray_image, cropped_bin_image, resultImg;
  uint thresh;
  resultImg = Mat(h, w, CV_8UC1, Scalar::all(0));
  for (int cnt = 0; cnt < DIV_NUM; cnt++) {
    uint c_w, c_w_n;
    c_w = w / DIV_NUM * cnt;
    c_w_n = w / DIV_NUM * (cnt + 1);
    cropped_gray_image = gray(Range(0, h), Range(c_w, c_w_n));
    //GaussianBlur(cropped_gray_image, cropped_gray_image, Size(5, 5), 0, 0);
    thresh = get_average_gray(cropped_gray_image);
    //String thr_png = "cropped_gry_image_" + toString(cnt) + "-" + toString(thresh) + ".png";
    //imwrite(thr_png, cropped_gray_image);
    if (thresh < 60)
      continue;
    cv::threshold(cropped_gray_image, cropped_bin_image, thresh, 255, THRESH_BINARY);
    //thr_png = "cropped_bin_image_"+ toString(cnt) + "-" + toString(thresh) + ".png";
    //imwrite(thr_png, cropped_bin_image);
    Mat ROU = resultImg(Rect(c_w, 0, w / DIV_NUM, h));
    cropped_bin_image.copyTo(ROU);
  }
  //cv::imshow("binimg", resultImg);
  resultImg.copyTo(b_image);
}

bool be_neighboring(Point& pt0, Point& pt1) {
  auto dx = pt1.x - pt0.x;
  auto dy = pt1.y - pt0.y;
  bool ben = dx == 0 && (dy == 1 || dy == -1)
    || dy == 0 && (dx == 1 || dx == -1)
    || dx == 1 && dy == 1
    || dx == -1 && dy == -1
    || dx == 1 && dy == -1
    || dx == -1 && dy == 1;
  return ben;
}
void cut_contour_2_sides(contour_list& cnt_list, segments& smt_list) {
  for (int ix = 0; ix < cnt_list.size(); ix++) {
    auto& contour = cnt_list[ix];
    if (contour.size() < 40) continue;//contour is too small,so rejected
    circle_iterable_curve tcv_contour = { contour, 0 };
    curve approx;
    cv::approxPolyDP(contour, approx, 8, true);
    circle_iterable_curve tcv_approx = { approx ,0 };
    int icnt_consume = 0;
    int sid = -1, first_sid = -1;
    while (icnt_consume != contour.size()) {
      auto& cur_pt_contour = tcv_contour.cur_pt();
      auto& cur_pt_approx = tcv_approx.cur_pt();
      if (cur_pt_contour == cur_pt_approx) {
        tcv_approx.next_id();
        bool new_side = true;
        if (sid > -1) {
          auto& cur_side = smt_list[sid];
          //cur_side.sure_direct_right_or_down();
          if (sid > 0) {
            auto pre_sid = sid - 1;
            auto& pre_side = smt_list[pre_sid];
            auto pre_end_id = pre_side._cv.size() - 1;
            auto& pt0 = pre_side._cv[pre_end_id];
            auto& pt1 = cur_side._cv[0];
            if (be_neighboring(pt0, pt1)) {
              bool same_type = cur_side.vertical() == pre_side.vertical();
              if (same_type) {//combine current side to previous side
                new_side = false;
                for (auto& pt : cur_side._cv) {
                  pre_side.append_point(pt);
                }
                cur_side.reset();
              }
            }
          }
        }
        if (new_side) {
          smt_list.push_back(side());
          auto smt_sz = smt_list.size();
          if (sid == -1) {
            first_sid = smt_sz - 1;
          }
          sid = smt_sz - 1;
        }
        smt_list[sid].append_point(cur_pt_contour);
        icnt_consume++;
      }
      else if (sid > -1) {
        smt_list[sid].append_point(cur_pt_contour);
        icnt_consume++;
      }
      tcv_contour.next_id();
    }
    //smt_list[sid].sure_direct_right_or_down();
    int pre_id = sid - 1;
    bool be_same_type = smt_list[sid].vertical() == smt_list[pre_id].vertical();
    if (be_same_type) {
      //auto& cur_cv = smt_list[sid]._cv;
      auto& cv = smt_list[sid]._cv;
      for (auto& pt : cv) {
        smt_list[pre_id].append_point(pt);
      }
      smt_list.erase(smt_list.begin() + sid);
      sid = pre_id;
    }

    bool same_type_with_first = smt_list[sid].vertical() == smt_list[first_sid].vertical();
    if (same_type_with_first) {
      auto& cv = smt_list[sid]._cv;
      for (auto icv = cv.rbegin(); icv != cv.rend(); icv++) {
        smt_list[first_sid].add_point(*icv);
      }
      smt_list.erase(smt_list.begin() + sid);
      sid--;
    }
    for (int ix = first_sid; ix <= sid; ix++) {
      smt_list[ix].sure_direct_right_or_down();
    }
  }
}
void sort_sides_by_mid(segments& sides) {
  std::sort(sides.begin(), sides.end(), [](side& s1, side& s2) {
    if (!s1.vertical() && !s2.vertical() || s1.vertical() && s2.vertical()) {
      auto mid1 = s1.y_max + s1.y_min;
      auto mid2 = s2.y_max + s2.y_min;
      return mid1 < mid2;
    } else {
      return !s1.vertical() && s2.vertical();
    }
  });
}
void cal_range(std::vector<uint>& id_range, int x_cnt, int y_cnt) {
  id_range.resize(y_cnt * 2 + 2);
  id_range[0] = 0;
  id_range[1] = x_cnt / 2;
  uint num_row_size = y_cnt + 1;
  for (int idx = 2; idx < id_range.size(); idx++) {
    if (idx == num_row_size) {
      id_range[idx] = id_range[idx - 1] + x_cnt / 2;
    }
    else {
      id_range[idx] = id_range[idx - 1] + x_cnt;
    }
  };
}
void sort_by_xmin(segments& smt_list, std::vector<uint>& id_range) {
  for (int id = 0; id < id_range.size() - 1; id++) {
    std::sort(smt_list.begin() + id_range[id], smt_list.begin() + id_range[id + 1], [](side& s1, side& s2) {
      return s1.x_min < s2.x_min;
      });
  }
}

void combine_2_sides(side& sd0, side& sd1, side& sd_o) {
  assert(sd0.vertical() == sd1.vertical());
  struct pick_pt_from_side {
    int id;
    side& _sd;
    Point* pick_pt() {
      if (id >= 0 && id < _sd._cv.size()) {
        return &_sd._cv[id];
      }
      return nullptr;
    }
    void next_id() {
      id++;
    }
  };
  if (sd0.vertical()) {
    pick_pt_from_side pk0 = { 0,sd0 }, pk1 = { 0,sd1 };
    side* psd_top = nullptr, * psd_low = nullptr;
    pick_pt_from_side* ppick_pt = nullptr;
    if (sd0.y_min < sd1.y_min) {
      psd_top = &sd0;
      psd_low = &sd1;
      ppick_pt = &pk1;
    }
    else {
      psd_top = &sd1;
      psd_low = &sd0;
      ppick_pt = &pk0;
    }
    auto& pt_mid0 = psd_low->_cv[0];
    auto& cv = psd_top->_cv;
    int ix = 0;
    for (; ix < cv.size(); ix++) {
      if (cv[ix].y == pt_mid0.y) {
        break;
      }
    }
    ppick_pt->id = -ix;
    Point* pt0 = pk0.pick_pt();
    Point* pt1 = pk1.pick_pt();
    Point pt_mid = { (int)((pt_mid0.x + cv[ix].x) * 0.5),pt_mid0.y };
    while (pt0 || pt1) {
      if (pt0 && pt1) {
        pt_mid = { (int)((pt0->x + pt1->x) * 0.5),pt0->y };
        sd_o.append_point(pt_mid);
      }
      else {
        Point* p_pt = pt0 ? pt0 : pt1;
        pt_mid.y = p_pt->y;
        sd_o.append_point(pt_mid);
      }
      pk0.next_id();
      pk1.next_id();
      pt0 = pk0.pick_pt();
      pt1 = pk1.pick_pt();
    }
  }
  else {
    pick_pt_from_side pk0 = { 0,sd0 }, pk1 = { 0,sd1 };
    side* psd_left = nullptr, * psd_right = nullptr;
    pick_pt_from_side* ppick_pt = nullptr;
    if (sd0.x_min < sd1.x_min) {
      psd_left = &sd0;
      psd_right = &sd1;
      ppick_pt = &pk1;
    }
    else {
      psd_left = &sd1;
      psd_right = &sd0;
      ppick_pt = &pk0;
    }
    auto& pt_mid0 = psd_right->_cv[0];
    auto& cv = psd_left->_cv;
    int ix = 0;
    for (; ix < cv.size(); ix++) {
      if (cv[ix].x == pt_mid0.x) {
        break;
      }
    }
    ppick_pt->id = -ix;
    Point* pt0 = pk0.pick_pt();
    Point* pt1 = pk1.pick_pt();
    Point pt_mid = { pt_mid0.x ,(int)((pt_mid0.y + cv[ix].y) * 0.5) };
    while (pt0 || pt1) {
      if (pt0 && pt1) {
        pt_mid = { pt0->x,(int)((pt0->y + pt1->y) * 0.5) };
        sd_o.append_point(pt_mid);
      }
      else {
        Point* p_pt = pt0 ? pt0 : pt1;
        pt_mid.x = p_pt->x;
        sd_o.append_point(pt_mid);
      }
      pk0.next_id();
      pk1.next_id();
      pt0 = pk0.pick_pt();
      pt1 = pk1.pick_pt();
    }
  }
}
void combine_2_sements(std::vector<uint>& id_range, segments& sides_red, segments& sides_green,segments& sides_o,int x_cnt,int end_row_id,int start_col_id) {
  for (int id = 0; id < id_range.size() - 1; id++) {
    if (0 == id_range[id] || end_row_id == id_range[id]) {
      for (int ix = id_range[id]; ix < id_range[id + 1]; ix++) {
        sides_o.push_back(side());
        auto& sd_red_part = sides_o[sides_o.size() - 1];
        for (auto& pt : sides_red[ix]._cv) {
          sd_red_part.append_point(pt);
        }
        sides_o.push_back(side());
        auto& sd_green_part = sides_o[sides_o.size() - 1];
        for (auto& pt : sides_green[ix]._cv) {
          sd_green_part.append_point(pt);
        }
      }
    }
    else if (id_range[id] < end_row_id) {
      for (int ix = id_range[id]; ix < id_range[id + 1]; ix++) {
        sides_o.push_back(side());
        auto& sd_o = sides_o[sides_o.size() - 1];
        auto& sd_red_part = sides_red[ix], & sd_green_part = sides_green[ix];
        combine_2_sides(sd_red_part, sd_green_part, sd_o);
      }
    }
    else {
      auto rem = (id_range[id] - start_col_id) / x_cnt;
      auto odd_num = rem % 2;
      segments* pforward_sds = nullptr, * pafterward_sds = nullptr;
      if (odd_num == 0) { //red first
        pforward_sds = &sides_red;
        pafterward_sds = &sides_green;
      }
      else {//green first
        pforward_sds = &sides_green;
        pafterward_sds = &sides_red;
      }
      sides_o.push_back(side());
      auto& sd0 = sides_o[sides_o.size() - 1];
      auto& cv0 = (*pforward_sds)[id_range[id]]._cv;
      for (auto& pt : cv0) {
        sd0.append_point(pt);
      }
      int ix = id_range[id];
      for (; ix < id_range[id + 1] - 1; ix++) {
        sides_o.push_back(side());
        auto& sd = sides_o[sides_o.size() - 1];
        auto& sd_f = (*pforward_sds)[ix + 1];
        auto& sd_a = (*pafterward_sds)[ix];
        combine_2_sides(sd_f, sd_a, sd);
      }
      sides_o.push_back(side());
      auto& sd_end = sides_o[sides_o.size() - 1];
      auto& cv_end = (*pafterward_sds)[ix]._cv;
      for (auto& pt : cv_end) {
        sd_end.append_point(pt);
      }
    }
  }
}
void line_points(Point& p0, Point& p1, std::vector<Point>& pt_list) {
  cv::LineIterator it(p0, p1);
  pt_list.resize(it.count);
  for (int i = 0; i < it.count; ++i, ++it) {
    pt_list[i] = it.pos();
  }
}

void get_curve_from_side_by_middle_point(curve& cv_l, side& sd_s, bool end = false, bool vertical = false) {
  auto& cv_sd_s = sd_s._cv;
  auto cv_sd_s_sz = cv_sd_s.size();
  auto cv_l_sz = cv_l.size();
  auto mid_id = cv_sd_s_sz / 2 - 1;//get index of middle point of side sd_s
  auto& pt_mid = cv_sd_s[mid_id];
  if (cv_l_sz == 0) {
    if (vertical) {// if sd_s is vertical,we get rhe middle x of the side as x of point will be appened
      for (int ix = 0; ix < mid_id; ix++) {
        Point pt = { pt_mid.x, cv_sd_s[ix].y };
        cv_l.emplace_back(pt);
      }
    } else {
      for (int ix = 0; ix < mid_id; ix++) {
        Point pt = { cv_sd_s[ix].x,pt_mid.y };
        cv_l.emplace_back(pt);
      }
    }
  }
  else {
    auto& pt_end = cv_l[cv_l_sz - 1];
    auto& pt_mid = cv_sd_s[mid_id];
    curve cv_g;
    line_points(pt_end, pt_mid, cv_g);
    for (auto& pt : cv_g) {
      cv_l.push_back(pt);
    }
    if (end) {
      if (vertical) {
        for (int ix = mid_id + 1; ix < cv_sd_s_sz; ix++) {
          Point pt = { pt_mid.x,cv_sd_s[ix].y };
          cv_l.emplace_back(pt);
        }
      } else {
        for (int ix = mid_id + 1; ix < cv_sd_s_sz; ix++) {
          Point pt = { cv_sd_s[ix].x,pt_mid.y };
          cv_l.emplace_back(pt);
        }
      }
    }
  }
}

enum smooth_id {
  en_3,
  en_5,
  en_5_3,
};
void smooth_curve(curve& cv, smooth_id sid, bool vertical = true) {
  auto icv_sz = cv.size();
  std::vector<int>  ut(icv_sz);
  if (vertical) {
    for (int ix = 0; ix < icv_sz; ix++) {
      ut[ix] = cv[ix].x;
    }
    switch (sid)
    {
    case en_3:
      cv[0].x = (5.0 * ut[0] + 2.0 * ut[1] - ut[2]) / 6;
      for (int ix = 1; ix < icv_sz - 1; ix++) {
        cv[ix].x = (ut[ix - 1] + ut[ix] + ut[ix + 1]) * 1.0 / 3;
      }
      cv[icv_sz - 1].x = (5.0 * ut[icv_sz - 1] + 2.0 * ut[icv_sz - 2] - ut[icv_sz - 3]) * 1.0 / 6;
      break;
    case en_5:
      cv[0].x = (3.0 * ut[0] + 2.0 * ut[1] + ut[2] - ut[4]) / 5;
      cv[1].x = (4.0 * ut[0] + 3.0 * ut[1] + 2.0 * ut[2] + ut[3]) / 10;
      for (int ix = 2; ix < icv_sz - 2; ix++) {
        cv[ix].x = (ut[ix - 2] + ut[ix - 1] + ut[ix] + ut[ix + 1] + ut[ix + 2]) * 1.0 / 5;
      }
      cv[icv_sz - 2].x = (ut[icv_sz - 4] + 2.0 * ut[icv_sz - 3] + 3.0 * ut[icv_sz - 2] + 4.0 * ut[icv_sz - 1]) / 10;
      cv[icv_sz - 1].x = (ut[icv_sz - 3] - ut[icv_sz - 5] + 2.0 * ut[icv_sz - 2] + 3.0 * ut[icv_sz - 1]) / 5;
      break;
    case en_5_3:
      cv[0].x = (69.0 * ut[0] + 4.0 * ut[1] - 6.0 * ut[2] + 4.0 * ut[3] - ut[4]) / 70;
      cv[1].x = (2.0 * ut[0] + 27.0 * ut[1] + 12.0 * ut[2] - 8.0 * ut[3] + 2.0 * ut[4]) / 35;
      for (int i = 2; i < icv_sz - 2; i++)
      {
        cv[i].x = (12.0 * (ut[i - 1] + ut[i + 1]) - 3.0 * (ut[i - 2] + ut[i + 2]) + 17.0 * ut[i]) / 35;
      }
      cv[icv_sz - 2].x = (2.0 * ut[icv_sz - 5] - 8.0 * ut[icv_sz - 4] + 12.0 * ut[icv_sz - 3] + 27.0 * ut[icv_sz - 2] + 2 * ut[icv_sz - 1]) / 35;
      cv[icv_sz - 1].x = (-ut[icv_sz - 5] + 4.0 * ut[icv_sz - 4] - 6.0 * ut[icv_sz - 3] + 4.0 * ut[icv_sz - 2] + 69.0 * ut[icv_sz - 1]) / 70;
      break;
    }
  }
  else {
    for (int ix = 0; ix < icv_sz; ix++) {
      ut[ix] = cv[ix].y;
    }
    switch (sid)
    {
    case en_3:
      cv[0].y = (5.0 * ut[0] + 2.0 * ut[1] - ut[2]) / 6;
      for (int ix = 1; ix < icv_sz - 1; ix++) {
        cv[ix].y = (ut[ix - 1] + ut[ix] + ut[ix + 1]) * 1.0 / 3;
      }
      cv[icv_sz - 1].y = (5.0 * ut[icv_sz - 1] + 2.0 * ut[icv_sz - 2] - ut[icv_sz - 3]) * 1.0 / 6;
      break;
    case en_5:
      cv[0].y = (3.0 * ut[0] + 2.0 * ut[1] + ut[2] - ut[4]) / 5;
      cv[1].y = (4.0 * ut[0] + 3.0 * ut[1] + 2.0 * ut[2] + ut[3]) / 10;
      for (int ix = 2; ix < icv_sz - 2; ix++) {
        cv[ix].y = (ut[ix - 2] + ut[ix - 1] + ut[ix] + ut[ix + 1] + ut[ix + 2]) * 1.0 / 5;
      }
      cv[icv_sz - 2].y = (ut[icv_sz - 4] + 2.0 * ut[icv_sz - 3] + 3.0 * ut[icv_sz - 2] + 4.0 * ut[icv_sz - 1]) / 10;
      cv[icv_sz - 1].y = (ut[icv_sz - 3] - ut[icv_sz - 5] + 2.0 * ut[icv_sz - 2] + 3.0 * ut[icv_sz - 1]) / 5;
      break;
    case en_5_3:
      cv[0].y = (69.0 * ut[0] + 4.0 * ut[1] - 6.0 * ut[2] + 4.0 * ut[3] - ut[4]) / 70;
      cv[1].y = (2.0 * ut[0] + 27.0 * ut[1] + 12.0 * ut[2] - 8.0 * ut[3] + 2.0 * ut[4]) / 35;
      for (int i = 2; i < icv_sz - 2; i++)
      {
        cv[i].y = (12.0 * (ut[i - 1] + ut[i + 1]) - 3.0 * (ut[i - 2] + ut[i + 2]) + 17.0 * ut[i]) / 35;
      }
      cv[icv_sz - 2].y = (2.0 * ut[icv_sz - 5] - 8.0 * ut[icv_sz - 4] + 12.0 * ut[icv_sz - 3] + 27.0 * ut[icv_sz - 2] + 2 * ut[icv_sz - 1]) / 35;
      cv[icv_sz - 1].y = (-ut[icv_sz - 5] + 4.0 * ut[icv_sz - 4] - 6.0 * ut[icv_sz - 3] + 4.0 * ut[icv_sz - 2] + 69.0 * ut[icv_sz - 1]) / 70;
      break;
    }
  }
}

void offset_curve(int of, curve& cv, bool h = true) {
  if (h) {
    for (auto& pt : cv) {
      pt.x += of;
    }
  } else {
    for (auto& pt : cv) {
      pt.y += of;
    }
  }
}
void extend_curve(int of, curve& cv, bool h = true) {
  if (h) {
    auto pt0 = cv[0];
    for (int ix = 1; ix <= of; ix++) {
      Point pt = { pt0.x - ix,pt0.y };
      cv.insert(cv.begin(), pt);
    }
    auto pte = cv[cv.size() - 1];
    for (int ix = 1; ix < of; ix++) {
      cv.push_back({ pte.x + ix,pte.y });
    }
  } else {
    auto pt0 = cv[0];
    for (int iy = 1; iy <= of; iy++) {
      Point pt = { pt0.x,pt0.y - iy };
      cv.insert(cv.begin(), pt);
    }
    auto pte = cv[cv.size() - 1];
    for (int iy = 1; iy < of; iy++) {
      cv.push_back({ pte.x,pte.y + iy });
    }
  }
}

//fill points to gap between two seperate points
void fill_points_2_gap(curve& cv, curve& cv_o) {

  auto pt0 = cv[0];
  cv_o.emplace_back(pt0);
  auto cv_sz = cv.size();
  for (int ix = 1; ix < cv_sz; ix++) {
    auto& pt = cv[ix];
    if (be_neighboring(pt0, pt)) {
      cv_o.emplace_back(pt);
    }
    else {
      curve mid_cv;
      line_points(pt0, pt, mid_cv);
      auto msz = mid_cv.size();
      for (int i = 1; i < msz; i++) {
        cv_o.emplace_back(mid_cv[i]);
      }

    }
    pt0 = pt;
  }
}
void get_lines_by_fill_points_2_gap(std::vector<curve>& lines_in, std::vector<curve>& lines_o) {
  for (int ix = 0; ix < lines_in.size(); ix++) {
    fill_points_2_gap(lines_in[ix], lines_o[ix]);
  }
}
//find intersection of 2 curves,and cut curve to sides and blocks
void find_intersection_2_curve_cutted_2_sides(curve& cv_h, int& hid, side& sd_h,
  curve& cv_v, int& vid, side& sd_v) {
    while (true) {
      auto& pth = cv_h[hid];
      auto& ptv = cv_v[vid];
      if (pth == ptv) {
        return;
      }

      auto dy = pth.y - ptv.y;
      auto dx = ptv.x - pth.x;
      if (dy > dx) {
        sd_v.append_point(ptv);
        vid++;
      }
      else if (dy < dx) {
        sd_h.append_point(pth);
        hid++;
      }
      else {//
        sd_v.append_point(ptv);
        sd_h.append_point(pth);
        vid++;
        hid++;
      }
    }
}


}
