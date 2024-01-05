#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <functional>
#include <chrono>
#include <fstream>
using namespace cv;
using namespace std;
using namespace chrono;
struct dic_uint {
  uint16_t x, y;
};
int txt_w = 1920, txt_h = 720;
int bk_x_cnt = 24, bk_y_cnt = 9,o_unit_bk_sz=80;
const uint uimax = -1;
namespace block_detect {
  using namespace std;
  using curve = vector<Point>;

  /*

  direction of moving:
  ↖  ↑  ↗
  ←      →
  ↙  ↓  ↘

  */
  enum move_action {
    left_move,
    left_up_move,
    up_move,
    right_up_move,
    right_move,
    right_down_move,
    down_move,
    left_down_move,
    move_count
  };
  move_action rev_actions[move_count] = {
    right_move,
    right_up_move,
    down_move,
    left_down_move,
    left_move,
    left_up_move,
    up_move,
    right_up_move,
  };
  using move_point = std::function<void(Point& pt)>;
  move_point _move_fn[move_count] = {
    [](Point& pt) { pt.x--; },
    [](Point& pt) { pt.x--; pt.y--; },
    [](Point& pt) { pt.y--; },
    [](Point& pt) { pt.x++; pt.y--; },
    [](Point& pt) { pt.x++; },
    [](Point& pt) { pt.x++; pt.y++; },
    [](Point& pt) { pt.y++; },
    [](Point& pt) { pt.x--; pt.y++; },
  };
  using action_list = std::vector<move_action>;
  struct side {
    curve _cv;
    action_list _alist;
    uint x_min{ uimax }, x_max{0};
    uint y_min{ uimax }, y_max{0};
    void reset() {
      _cv.clear();
      _alist.clear();
      x_min = y_min = uimax;
      x_max = y_max = 0;
    }
    void add_point(Point& pt) {
      if (pt.x < x_min) {
        x_min = pt.x;
      }
      if (pt.x > x_max) {
        x_max = pt.x;
      }
      if (pt.y < y_min) {
        y_min = pt.y;
      }
      if (pt.y > y_max) {
        y_max = pt.y;
      }
      _cv.insert(_cv.begin(), pt);
      if (_alist.size() > 0) {
        auto& pt1 = _cv[1];
        if (pt1.x > pt.x) {//right...
          if (pt1.y < pt.y) {
            _alist.insert(_alist.begin(), right_up_move);
          } else if (pt1.y > pt.y) {
            _alist.insert(_alist.begin(), right_down_move);
          } else {
            _alist.insert(_alist.begin(), right_move);
          }
        } else if (pt1.x < pt.y) {//left...
          if (pt1.y < pt.y) {
            _alist.insert(_alist.begin(), left_up_move);
          } else if (pt1.y > pt.y) {
            _alist.insert(_alist.begin(), left_down_move);
          } else {
            _alist.insert(_alist.begin(), left_move);
          }
        } else {
          if (pt1.y < pt.y) {
            _alist.insert(_alist.begin(), up_move);
          } else {
            _alist.insert(_alist.begin(), down_move);
          }
        }
      }

    }
    void append_point(Point& pt){
        if (pt.x < x_min) {
          x_min = pt.x;
        }
        if (pt.x > x_max) {
          x_max = pt.x;
        }
        if (pt.y < y_min) {
          y_min = pt.y;
        }
        if (pt.y > y_max) {
          y_max = pt.y;
        }   
        auto rm_sz = _cv.size();
        if(rm_sz >0) {
          auto& end_pt = _cv[rm_sz - 1];
          if (pt.x > end_pt.x) {//right...
            if (pt.y < end_pt.y) {
              _alist.emplace_back(right_up_move);
            } else if (pt.y > end_pt.y) {
              _alist.emplace_back(right_down_move);
            } else {
              _alist.emplace_back(right_move);
            }
          } else if (pt.x < end_pt.y) {//left...
            if (pt.y < end_pt.y) {
              _alist.emplace_back(left_up_move);
            } else if (pt.y > end_pt.y) {
              _alist.emplace_back(left_down_move);
            } else {
              _alist.emplace_back(left_move);
            }
          } else {
            if (pt.y < end_pt.y) {
              _alist.emplace_back(up_move);
            } else {
              _alist.emplace_back(down_move);
            }
          }
        }
        _cv.emplace_back(pt);
    }
    uint width() {
      return x_max - x_min;
    }
    uint height() {
      return y_max - y_min;
    }
    bool vertical() {
      return height() > width();
    }
    void sure_direct_right_or_down() {
      auto eid = _cv.size() - 1;
      if (vertical()&&_cv[0].y>_cv[eid].y|| !vertical() && _cv[0].x > _cv[eid].x) {
        std::reverse(_cv.begin(), _cv.end());
        for (auto& m : _alist) {
          m = rev_actions[m];
        }
        std::reverse(_alist.begin(), _alist.end());
      }
    }
  };
  using segments = vector<side>;
  struct block{
    segments& _sides;
    Point ori_0;
    //index of block_sides;
    uint left_side_id{0}, top_side_id{0}, right_side_id{0}, bottom_side_id{0};
#define DECLARE_SIDE(sd)\
    side& sd##_side(){return _sides[sd##_side_id];}
    DECLARE_SIDE(left)
    DECLARE_SIDE(top)
    DECLARE_SIDE(right)
    DECLARE_SIDE(bottom)

#define DECLARE_RANGE_FUN(sd,dir)\
    uint sd##_side_##dir##_min() {\
      return  sd##_side().##dir##_min;\
    }\
    uint sd##_side_##dir##_max() {\
      return  sd##_side().##dir##_max;\
    }
    DECLARE_RANGE_FUN(left,x)
    DECLARE_RANGE_FUN(right, x)
    DECLARE_RANGE_FUN(top, y)
    DECLARE_RANGE_FUN(bottom, y)
  };
  segments _ref_sides;
  vector<block> _blocks;
  void o_coordinate(int block_id,Point& pt) {
    int bk_row_id = block_id / bk_x_cnt;
    int bk_col_id = block_id % bk_x_cnt;
    pt.y = bk_row_id * o_unit_bk_sz;
    pt.x = bk_col_id * o_unit_bk_sz;
  }
  int within(int x, int y) {
    for (int id = 0; id < _blocks.size(); id++) {
      auto& bk = _blocks[id];
      if (x < bk.left_side_x_min()
        || x >= bk.right_side_x_max()
        || y < bk.top_side_y_min()
        || y >= bk.bottom_side_y_max()) {
        continue;
      }
      if (x >= bk.left_side_x_min() &&
        x <= bk.left_side_x_max()) {
        for (auto& pt : bk.left_side()._cv) {
          if (pt.y == y) {
            if(x>=pt.x) return id;
            break;
          }
        }
        auto& ori_pt = bk.left_side()._cv[0];
        if (y< ori_pt.y&& ori_pt.y < bk.top_side_y_min()) {
          for (auto& pt : bk.top_side()._cv) {
            if (pt.y == y||pt.x > bk.left_side_x_min()) {
              if(x>=pt.x )return id;
              break;
            }
          }
        } 
        auto& bottom_start_pt = bk.bottom_side()._cv[0];
        if (y >= bottom_start_pt.y && bottom_start_pt.y > bk.bottom_side_y_max()) {
          for (auto& pt : bk.bottom_side()._cv) {
            if (pt.y == y|| pt.x > bk.left_side_x_min()) {
              if(x>pt.x)  return id;
              break;
            }
          }
        }
      }
      if (x >= bk.right_side_x_min() &&
        x < bk.right_side_x_max()) {
        for (auto& pt : bk.right_side()._cv) {
          if (pt.y == y) {
            if (x < pt.x) return id;
            break;
          }
        }
        auto& right_start_pt = bk.right_side()._cv[0];
        if (y < right_start_pt.y && right_start_pt.y < bk.top_side_y_min()) {
          auto& tside_cv = bk.top_side()._cv;
          for (auto ipt = tside_cv.rbegin(); ipt != tside_cv.rend(); ipt++) {
            auto& pt = *ipt;
            if (pt.y == y|| pt.x < bk.right_side_x_min()) {
              if (x <= pt.x) return id;
              break;
            }
          }
        }
        auto& bside = bk.bottom_side()._cv;
        auto clen = bside.size();
        auto& end_pt = bside[clen - 1];
        if (y >= end_pt.y && end_pt.y > bk.bottom_side_y_max()) {
          for (auto ipt = bside.rbegin(); ipt != bside.rend(); ipt++) {
            auto& pt = *ipt;
            if (pt.y == y|| pt.x < bk.right_side_x_min()) {
              if (x < pt.x) return id;
              break;
            }
          }
        }
      }
      for (auto& pt : bk.top_side()._cv) {
        if (x == pt.x&&y < pt.y) {
            continue;
        }
      }
      for (auto& pt : bk.bottom_side()._cv) {
        if (x == pt.x && y >= pt.y) {
          continue;
        }
      }
      return id;
    }
    return -1;
  }
};

uint get_averange_gray(Mat &image) {
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
            uchar *current_row = dst.ptr<uchar>(row);
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

void get_img_binary(Mat &image, Mat &b_image) {

    Mat gray = image.clone();
    uint w, h;
    w = gray.size().width;
    h = gray.size().height;

    //Crop Gray image
    const uint DIV_NUM = w*0.003;
    Mat cropped_gray_image, cropped_bin_image, resultImg;
    uint thresh;
    resultImg = Mat(h, w, CV_8UC1, Scalar::all(0));
    for (int cnt = 0; cnt < DIV_NUM; cnt++) {
        uint c_w, c_w_n;
        c_w = w / DIV_NUM * cnt;
        c_w_n = w / DIV_NUM * (cnt + 1);
        cropped_gray_image = gray(Range(0, h), Range(c_w, c_w_n));
        //GaussianBlur(cropped_gray_image, cropped_gray_image, Size(5, 5), 0, 0);
        thresh = get_averange_gray(cropped_gray_image);
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
int main(int argc, char** argv) {
  auto img = cv::imread(argv[1]);
  if (argc > 3) {
    txt_w = atoi(argv[2]);
    txt_h = atoi(argv[3]);
    if (argc > 5) {
      bk_x_cnt = atoi(argv[4]);
      bk_y_cnt = atoi(argv[5]);
    }
  }
  auto& ori = img;
  Mat ori_red;
  img.copyTo(ori_red);
  Mat ori_green;
  img.copyTo(ori_green);
  Mat ori_curves;
  img.copyTo(ori_curves);
  Mat ori_cv;
  img.copyTo(ori_cv);

  cv::Mat ele = getStructuringElement(MORPH_RECT, { 5,5 });
  vector<Mat> channels;
  split(img, channels);
  auto& blue = channels[0];
  auto& green = channels[1];
  auto& red = channels[2];

  Mat green_th;
  get_img_binary(green, green_th);
  Mat erode_green_th;
  cv::erode(green_th, erode_green_th, ele, { -1,-1 }, 3);
  Mat red_th;
  get_img_binary(red, red_th);
  Mat erode_red_th;
  cv::erode(red_th, erode_red_th, ele, { -1,-1 }, 2);

  using contour_list = std::vector<std::vector<cv::Point>>;
  contour_list contours_red,contours_green;

  cv::findContours(erode_red_th, contours_red,  cv::RETR_LIST,cv::CHAIN_APPROX_NONE);
  cv::findContours(erode_green_th, contours_green,  cv::RETR_LIST,cv::CHAIN_APPROX_NONE);
  using namespace block_detect;
  segments sides_red, sides_green, tmp_sides;
 // cv::drawContours(img_bk, contours, -1, CV_RGB(255,0,0));
  struct {
    uint8_t r = 255, g = 0, b = 0;
    void next(uint8_t delta=10) {
      uint8_t iv= b +delta;
      if (iv < b) {//overflow
        b = iv;
        uint8_t iv2 = g + delta;
        if (iv2 < g) {//overflow
          r += delta;
        }
        g = iv2;
      }
      b = iv;
    }
    cv::Scalar operator () () {
      return cv::Scalar((b), (g), (r), 0);
    }
  } col_d,col_t;
  struct circle_iterable_curve{
    curve& _cv;
    int id{0};
    void next_id() {
      id++;
      auto cv_len= _cv.size();
      id %= cv_len;
    }
    Point& cur_pt() {
        return _cv[id];
    }
    Point& pre_pt() {
      auto pre_id = id - 1;
      if (pre_id == -1) {
        pre_id = _cv.size() - 1;
      }
      return _cv[pre_id];
    }
  };
  auto be_neighboring = [](Point& pt0, Point& pt1) {
    auto dx = pt1.x - pt0.x;
    auto dy = pt1.y - pt0.y;
    bool ben = dx == 0 && (dy == 1 || dy == -1)
      || dy == 0 && (dx == 1 || dx == -1)
      || dx == 1 && dy == 1
      || dx == -1 && dy == -1
      || dx == 1 && dy == -1
      || dx == -1 && dy == 1;
    return ben;
  };
  auto cos_a_of_3_points = [](Point& pre_pt,Point& cur_pt,Point& next_pt) {
    auto v_chord_pre = cur_pt - pre_pt;
    auto v_chord = next_pt - cur_pt;
    auto sq_v_chord_pre = v_chord_pre.x * v_chord_pre.x + v_chord_pre.y * v_chord_pre.y;
    auto len_v_chord_pre = pow(sq_v_chord_pre, 0.5);
    auto sq_v_chord = v_chord.x * v_chord.x + v_chord.y * v_chord.y;
    auto len_v_chord = pow(sq_v_chord, 0.5);
    auto sq_chord = len_v_chord_pre * len_v_chord;
    auto cos_a = (v_chord_pre.x * v_chord.x + v_chord_pre.y * v_chord.y) / sq_chord;
    return cos_a;
  };
  auto cut_contour_2_sides = [&](contour_list& cnt_list, segments& smt_list) {
    for (int ix = 0; ix < cnt_list.size(); ix++) {
      auto& contour = cnt_list[ix];
      if (contour.size() < 40) continue;
      circle_iterable_curve tcv_contour = { contour, 0 };
      curve approx;
      cv::approxPolyDP(contour, approx, 8, true);
      circle_iterable_curve tcv_approx = { approx ,0 };
      int icnt_consume = 0;
      int sid = -1,first_sid=-1;
      while (icnt_consume!= contour.size()) {
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
              if(be_neighboring(pt0, pt1)) {
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
              first_sid=smt_sz - 1;
            }
            sid = smt_sz - 1;
          }
          smt_list[sid].append_point(cur_pt_contour);
          icnt_consume++;
        } else if(sid>-1) {
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
  };
  cut_contour_2_sides(contours_red, sides_red);
  cut_contour_2_sides(contours_green, sides_green);

  auto sort_sides_by_mid = [&](segments& sides) {
    std::sort(sides.begin(), sides.end(), [](side& s1, side& s2) {
      if (!s1.vertical() && !s2.vertical()|| s1.vertical() && s2.vertical()) {
        auto mid1 = s1.y_max + s1.y_min;
        auto mid2 = s2.y_max + s2.y_min; 
        return mid1 < mid2; 
      }  else  {
        return !s1.vertical() && s2.vertical();
      }
    });
  };
  sort_sides_by_mid(sides_red);
  sort_sides_by_mid(sides_green);
  
  std::vector<uint> lines_id_range =
  {  0, 12, 36, 60, 84, 108,132,156,180,204,//rows
    216,240,264,288,312,336,360,384,408,432};//cols
  auto sort_by_xmin = [&](segments& smt_list) {
    for (int id = 0; id < lines_id_range.size()-1; id++) {
      std::sort(smt_list.begin() + lines_id_range[id], smt_list.begin() + lines_id_range[id + 1], [](side& s1, side& s2) {
      return s1.x_min < s2.x_min;
    });
    }
  };
  sort_by_xmin(sides_red);
  sort_by_xmin(sides_green);
  auto combine_2_sides = [](side& sd0, side& sd1, side& sd_o) {
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
      pick_pt_from_side pk0 = { 0,sd0 }, pk1 = {0,sd1};
      side* psd_top = nullptr, *psd_low = nullptr;
      pick_pt_from_side* ppick_pt = nullptr;
      if (sd0.y_min < sd1.y_min) {
        psd_top = &sd0;
        psd_low = &sd1;
        ppick_pt = &pk1;
      } else {
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
      Point pt_mid = {(int)((pt_mid0.x + cv[ix].x) * 0.5),pt_mid0.y };
      while (pt0 || pt1) {
        if (pt0 && pt1) {
          pt_mid = { (int)((pt0->x + pt1->x) * 0.5),pt0->y };
          sd_o.append_point(pt_mid);
        } else {
          Point* p_pt = pt0 ? pt0 : pt1;
          pt_mid.y = p_pt->y;
          sd_o.append_point(pt_mid);
        }
        pk0.next_id();
        pk1.next_id();
        pt0 = pk0.pick_pt();
        pt1 = pk1.pick_pt();
      }
    } else {
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
      Point pt_mid={ pt_mid0.x ,(int)((pt_mid0.y+cv[ix].y)*0.5)};
      while (pt0 || pt1) {
        if (pt0 && pt1) {
          pt_mid = { pt0->x,(int)((pt0->y + pt1->y) * 0.5) };
          sd_o.append_point(pt_mid);
        } else {
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
  };
  {
    for (int id = 0; id < lines_id_range.size() - 1; id++) {
      if (0==lines_id_range[id]|| 204 == lines_id_range[id]) {
        for (int ix = lines_id_range[id]; ix < lines_id_range[id + 1]; ix++) {
          tmp_sides.push_back(side());
          auto& sd_red_part = tmp_sides[tmp_sides.size() - 1];
          for (auto& pt : sides_red[ix]._cv) {
            sd_red_part.append_point(pt);
          }
          tmp_sides.push_back(side());
          auto& sd_green_part = tmp_sides[tmp_sides.size() - 1];
          for (auto& pt : sides_green[ix]._cv) {
            sd_green_part.append_point(pt);
          }
        }
      }
      else if (lines_id_range[id]<204) {
        for (int ix = lines_id_range[id]; ix < lines_id_range[id + 1]; ix++) {
          tmp_sides.push_back(side());
          auto& sd_o = tmp_sides[tmp_sides.size() - 1];
          auto& sd_red_part = sides_red[ix], &sd_green_part = sides_green[ix];
          combine_2_sides(sd_red_part, sd_green_part, sd_o);
        }
      } 
      else {
        auto rem = (lines_id_range[id] - 216) / 24;
        auto odd_num = rem % 2;
        segments* pforward_sds=nullptr, * pafterward_sds = nullptr;
        if (odd_num == 0) { //red first
          pforward_sds = &sides_red;
          pafterward_sds = &sides_green;
        } else {//green first
          pforward_sds = &sides_green;
          pafterward_sds = &sides_red;
        }
        tmp_sides.push_back(side());
        auto& sd0 = tmp_sides[tmp_sides.size() - 1];
        auto& cv0 = (*pforward_sds)[lines_id_range[id]]._cv; 
        for (auto& pt : cv0) {
          sd0.append_point(pt);
        }
        int ix = lines_id_range[id];
        for (; ix < lines_id_range[id + 1] - 1; ix++) {
          tmp_sides.push_back(side());
          auto& sd = tmp_sides[tmp_sides.size() - 1];
          auto& sd_f = (*pforward_sds)[ix + 1];
          auto& sd_a = (*pafterward_sds)[ix];
          combine_2_sides(sd_f, sd_a, sd);
        }
        tmp_sides.push_back(side());
        auto& sd_end = tmp_sides[tmp_sides.size() - 1];
        auto& cv_end = (*pafterward_sds)[ix]._cv;
        for (auto& pt : cv_end) {
          sd_end.append_point(pt);
        }
      }
    }
  }
  auto line_points = [](Point& p0, Point& p1, std::vector<Point>& pt_list) {
    cv::LineIterator it(p0, p1);
    pt_list.resize(it.count);
    for (int i = 0; i < it.count; ++i, ++it) {
      pt_list[i] = it.pos();
    }
  };
  auto combine_side_2_curve = [&](curve& cv_l, side & sd_s) {
    auto& cv_s = sd_s._cv;
    auto cv_l_sz = cv_l.size();
    int sid = 0;
    if (sd_s.vertical()) {
      if (cv_l_sz > 0){
        if (cv_l[cv_l_sz - 1].y >= sd_s.y_min) {
          for (auto rit = cv_l.rbegin(); rit != cv_l.rend(); rit++) {
            if ((*rit).y == cv_s[0].y) {
              auto it = rit.base() - 1;
              for (; it != cv_l.end(); it++, sid++) {
                auto& pt = *it;
                pt.x = (pt.x + cv_s[sid].x) / 2;
              }
              break;
            }
          }
        } else {
          curve cv_mid;
          line_points(cv_l[cv_l_sz - 1], cv_s[0],cv_mid);
          for (int i = 1; i < cv_mid.size() - 1; i++) {
            cv_l.emplace_back(cv_mid[i]);
          }
        }
      }
      for (; sid < cv_s.size(); sid++) {
        cv_l.emplace_back(cv_s[sid]);
      }
    } else {
      if (cv_l_sz > 0) {
        if (cv_l[cv_l_sz - 1].x >= sd_s.x_min) {
          for (auto rit = cv_l.rbegin(); rit != cv_l.rend(); rit++) {
            if ((*rit).x == cv_s[0].x) {
              auto it = rit.base() - 1;
              for (; it != cv_l.end(); it++, sid++) {
                auto& pt = *it;
                pt.y = (pt.y + cv_s[sid].y) / 2;
              }
              break;
            }
          }
        }
        else {
          curve cv_mid;
          line_points(cv_l[cv_l_sz - 1], cv_s[0], cv_mid);
          for (int i = 1; i < cv_mid.size() - 1; i++) {
            cv_l.emplace_back(cv_mid[i]);
          }
        }
      }
      for (; sid < cv_s.size(); sid++) {
        cv_l.emplace_back(cv_s[sid]);
      }
    }
  };
  auto combine_midpt_of_side_2_curve = [&](curve& cv_l, side& sd_s,bool end=false,bool vertical=false) {
    auto& cv_s = sd_s._cv;
    auto cv_s_sz = cv_s.size();
    auto cv_l_sz = cv_l.size();
    auto mid_id = cv_s_sz / 2 - 1;
    auto& pt_mid = cv_s[mid_id];
    if (cv_l_sz == 0) {
      if (vertical) {
        for (int ix = 0; ix < mid_id; ix++) {
          Point pt = { pt_mid.x, cv_s[ix].y};
          cv_l.emplace_back(pt);
        }
      }
      else {
        for (int ix = 0; ix < mid_id; ix++) {
          Point pt = { cv_s[ix].x,pt_mid.y };
           cv_l.emplace_back(pt);
        }
      } 
    } else {
      auto& pt_end = cv_l[cv_l_sz - 1];
      auto& pt_mid = cv_s[mid_id];
      curve cv_g;
      line_points(pt_end, pt_mid, cv_g);
      for (auto& pt : cv_g) {
        cv_l.push_back(pt);
      }
      if (end) {
        if (vertical) {
          for (int ix = mid_id+1; ix < cv_s_sz; ix++) {
            Point pt = { pt_mid.x,cv_s[ix].y };
            cv_l.emplace_back(pt);
          }
        }
        else {
          for (int ix = mid_id + 1; ix < cv_s_sz; ix++) {
            Point pt = { cv_s[ix].x,pt_mid.y };
            cv_l.emplace_back(pt);
          }
        }
        
      }
    }
  };
  vector<curve> h_lines(10),v_lines(25);
  for (int ix = 0; ix < 10; ix++) {
    auto col_start = ix * 24,col_end=(ix+1)*24;
    auto& hL = h_lines[ix];
    for (int icol = col_start; icol < col_end; icol++) {
      //combine_side_2_curve(hL, tmp_sides[icol]);
      auto end = icol == col_end - 1;
      combine_midpt_of_side_2_curve(hL, tmp_sides[icol],end,false);
    }
  }
  for (int ix = 0; ix < 25; ix++) {
    //auto row_start = 240 + ix;
    auto& vL = v_lines[ix];
    for (int irow = 0; irow < 9; irow++) {
      auto sid = 240 + ix + irow * 25;
      //combine_side_2_curve(vL, tmp_sides[sid]);
      auto end = irow == 8;
      combine_midpt_of_side_2_curve(vL, tmp_sides[sid],end,true);
    }
  }
  enum smooth_id {
    en_3,
    en_5,
    en_5_3,
  };
  auto smooth_curve = [&](curve& cv, smooth_id sid,bool vertical=true) {
    auto icv_sz = cv.size();
    vector<int>  ut(icv_sz);
    if (vertical) {
      for (int ix = 0; ix < icv_sz;ix++) {
        ut[ix] = cv[ix].x;
      }
      switch (sid)
      {
      case en_3:
        cv[0].x = (5.0 * ut[0] + 2.0 * ut[1] - ut[2]) / 6;
        for (int ix = 1; ix < icv_sz - 1; ix++) {
          cv[ix].x = (ut[ix - 1] + ut[ix] + ut[ix + 1])*1.0 / 3;
        }
        cv[icv_sz - 1].x = (5.0 * ut[icv_sz - 1] + 2.0 * ut[icv_sz - 2] - ut[icv_sz - 3])*1.0 / 6;
        break;
      case en_5:
        cv[0].x = (3.0 * ut[0] + 2.0 * ut[1] + ut[2] - ut[4]) / 5;
        cv[1].x= (4.0 * ut[0] + 3.0 * ut[1] + 2.0 * ut[2] + ut[3]) / 10;
        for (int ix = 2; ix < icv_sz - 2; ix++) {
          cv[ix].x = (ut[ix - 2] + ut[ix - 1] + ut[ix] + ut[ix + 1] + ut[ix + 2])*1.0 / 5;
        }
        cv[icv_sz - 2].x= (ut[icv_sz - 4] + 2.0 * ut[icv_sz - 3] + 3.0 * ut[icv_sz - 2] + 4.0 * ut[icv_sz - 1]) / 10;
        cv[icv_sz - 1].x= (ut[icv_sz - 3] - ut[icv_sz - 5] + 2.0 * ut[icv_sz - 2] + 3.0 * ut[icv_sz - 1]) / 5;
        break;
      case en_5_3:
        cv[0].x= (69.0* ut[0] + 4.0 * ut[1] - 6.0*ut[2] + 4.0*ut[3] - ut[4]) / 70;
        cv[1].x = (2.0 * ut[0] + 27.0 * ut[1] + 12.0 * ut[2] - 8.0 * ut[3] + 2.0 * ut[4]) / 35;
        for (int i = 2; i < icv_sz - 2; i++)
        {
          cv[i].x = (12.0 * (ut[i - 1] + ut[i + 1])-3.0 * (ut[i - 2] + ut[i + 2])  + 17.0 * ut[i]) / 35;
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
  };
  auto draw_curves_2_image = [&](vector<curve>& vcv, Mat& image,cv::Scalar col) {
    int ix = 0;
    for (auto& cv : vcv) {
      auto sid = std::to_string(ix++);
      auto& pt0 = cv[0];
      cv::putText(image, sid, { pt0.x,pt0.y - 5 }, FONT_HERSHEY_SIMPLEX, 0.4, col);
      cv::polylines(image, cv, false, col);
    }
  };

  draw_curves_2_image(h_lines, ori_cv, { 0,255,255 });
  draw_curves_2_image(v_lines, ori_cv, { 130, 0, 75 });
  imshow("ori_cv", ori_cv);
  const int hlines_cnt = 10, vlines_cnt = 25;
  vector<curve> ah_lines(hlines_cnt), av_lines(vlines_cnt);
  double epilon = 7;

  for (int ix = 0; ix < ah_lines.size();ix++) {
    smooth_curve(h_lines[ix], en_5, false);
    cv::approxPolyDP(h_lines[ix], ah_lines[ix], epilon, false);
  }
  for (int ix = 0; ix < av_lines.size(); ix++) {
    smooth_curve(v_lines[ix], en_5, true);
    cv::approxPolyDP(v_lines[ix], av_lines[ix], epilon, false);
  }
  //XXX expand the range
  auto offset_curve=[&](int of, curve& cv, bool h=true) {
    if (h) {
      for (auto& pt : cv) {
        pt.x += of;
      }
    }
    else {
      for (auto& pt : cv) {
        pt.y += of;
      }
    }
  };
  auto extend_curve = [&](int of, curve& cv, bool h = true) {
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
    }
    else {
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
  };

  offset_curve(-5, av_lines[0]);
  offset_curve(5, av_lines[av_lines.size() - 1]);
  for (auto& cv : av_lines) {
    extend_curve(20, cv, false);
  }
  offset_curve(-5, ah_lines[0],false);
  offset_curve(5, ah_lines[ah_lines.size() - 1],false);
  for (auto& cv : ah_lines) {
    extend_curve(20, cv);
  }
  draw_curves_2_image(ah_lines, ori_curves, { 130, 0, 75 });
  draw_curves_2_image(av_lines, ori_curves, { 130, 0, 75 });
  cv::imwrite("ori_curves.png", ori_curves);
  //fill points to gap between two seperate points
  auto fill_points_2_gap = [&](curve& cv,curve& cv_o) {
    
    auto pt0 = cv[0];
    cv_o.emplace_back(pt0);
    auto cv_sz = cv.size();
    for (int ix = 1; ix < cv_sz;ix++) {
      auto& pt = cv[ix];
      if (be_neighboring(pt0, pt)) {
        cv_o.emplace_back(pt);
      } else {
        curve mid_cv;
        line_points(pt0, pt, mid_cv);
        auto msz = mid_cv.size();
        for (int i = 1; i < msz; i++) {
          cv_o.emplace_back(mid_cv[i]);
        } 
      }
      pt0 = pt;
    }
  };

  vector<curve> bh_lines(hlines_cnt), bv_lines(vlines_cnt);
  for (int ix = 0; ix<ah_lines.size(); ix++) {
    fill_points_2_gap(ah_lines[ix], bh_lines[ix]);
  }
  for (int ix = 0; ix<av_lines.size(); ix++) {
    fill_points_2_gap(av_lines[ix], bv_lines[ix]);
  }

  //find intersection of 2 curves,at the same time cut curve to sides and blocks
   auto f_its = []
   (curve& cv_h, int& hid, side& sd_h,
    curve& cv_v, int& vid,side& sd_v) {
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
      } else if (dy < dx) {
        sd_h.append_point(pth);
        hid++;
      } else {//
        sd_v.append_point(ptv);
        sd_h.append_point(pth);
        vid++;
        hid++;
      }
    }
  };
  vector<int> hlines_ids(hlines_cnt, 0), vlines_ids(vlines_cnt, 0);
  side top_sd_left[hlines_cnt], left_sd_top[vlines_cnt];
  const auto sides_cnt = vlines_cnt * (hlines_cnt - 1) + hlines_cnt * (vlines_cnt-1);
  _ref_sides.resize(sides_cnt);
  const auto  blk_cnt = (hlines_cnt - 1) * (vlines_cnt - 1);
  _blocks.resize(blk_cnt, { _ref_sides });
  const auto vside_base_id = hlines_cnt * (vlines_cnt - 1);
  for (int hcv_id = 0; hcv_id < hlines_cnt; hcv_id++) {
    auto& hcurve = bh_lines[hcv_id];
    auto& cur_hid = hlines_ids[hcv_id];
    side* ptop_sd = top_sd_left+hcv_id;
    for (int vcv_id = 0; vcv_id < vlines_cnt; vcv_id++) {
      auto& vcurve = bv_lines[vcv_id];
      auto& cur_vid = vlines_ids[vcv_id];
      side* pleft_sd = left_sd_top + vcv_id;
      if (vcv_id > 0) {
        auto prev_top_id = hcv_id * (vlines_cnt-1)+ vcv_id-1;
        ptop_sd = &_ref_sides[prev_top_id];
      }
      if (hcv_id > 0) {
        auto prev_left_id = vside_base_id+ (hcv_id-1)* vlines_cnt+vcv_id;
        pleft_sd = &_ref_sides[prev_left_id];
      }
      f_its(hcurve, cur_hid, *ptop_sd, vcurve, cur_vid, *pleft_sd);
      if (hcv_id < hlines_cnt - 1 && vcv_id < vlines_cnt - 1) {
        auto bk_id = hcv_id * (vlines_cnt - 1) + vcv_id;
        auto& cur_bk = _blocks[bk_id];
        cur_bk.ori_0 = { vcv_id * o_unit_bk_sz,hcv_id * o_unit_bk_sz};
        cur_bk.top_side_id = hcv_id * (vlines_cnt - 1) + vcv_id;
        cur_bk.bottom_side_id = cur_bk.top_side_id + vlines_cnt - 1;
        cur_bk.left_side_id = vside_base_id + hcv_id * vlines_cnt + vcv_id;
        cur_bk.right_side_id = cur_bk.left_side_id + 1;
      }
    }
  }
  //find ideal rectangle
  uint top{0}, bottom{ uimax }, left{0}, right{ uimax };
  for (int ix = 0; ix < vlines_cnt-1; ix++) {
    auto& sd_top = _ref_sides[ix];
    if (sd_top.y_max > top) {
      top = sd_top.y_max;
    }
    auto bt_sd_id = blk_cnt + ix;
    auto& sd_bottom= _ref_sides[bt_sd_id];
    if (sd_bottom.y_min < bottom) {
      bottom = sd_bottom.y_min;
    }
  }

  for (int ix = 0; ix < hlines_cnt-1; ix++) {
    auto edge_id = vside_base_id + ix * vlines_cnt;
    auto& sd= _ref_sides[edge_id];
    if (sd.x_max > left) {
      left = sd.x_max;
    }
    auto r_edge_id= edge_id+ vlines_cnt-1;
    auto& sd_r= _ref_sides[r_edge_id];
    if (sd_r.x_min < right) {
      right = sd_r.x_min;
    }
  }
  auto tw = right - left;
  auto th = bottom - top;
  const float w_over_h_rate = (float)txt_w / (float)txt_h;
  auto itw = w_over_h_rate * th;
  if (tw > itw) { //too wide
    auto ad_w = tw - itw;
    auto half_ad_w = ad_w / 2;
    left += half_ad_w;
    right -= half_ad_w;
  }
  else { // too narrow
    const float h_over_w_rate = (float)txt_h / (float)txt_w;
    auto ith = tw * h_over_w_rate;
    auto ad_h = th - ith;
    auto half_ad_h = ad_h / 2;
    top += half_ad_h;
    bottom -= half_ad_h;
  }
  tw = right - left;
  th = bottom - top;
  //create txt_dic
  auto tb_sz = txt_h * txt_w;
  auto ideal_rect_sz = tw * th;
  float s_sz = tb_sz / (float)ideal_rect_sz;
  vector<dic_uint> txt_dic(tb_sz);
  auto tm_before_tb = steady_clock::now();
  auto getDis = [](Point& p0, Point& p1) {
    return cv::abs(p0.x + p1.x) + cv::abs(p0.y + p1.y);
  };
#if 0
  for (uint16_t iy = top; iy <= bottom; iy++) {
    for (uint16_t ix = left; ix <= right; ix++) {
      using namespace block_detect;
      auto id_block = within(ix, iy);
      Point o_start;
      o_coordinate(id_block, o_start);//求出该block在变形前的起点坐标
      block& bk = _blocks[id_block];
      //detect relative coordinate of sample
      Point pt{ ix,iy };

      auto& lcv = bk.left_side()._cv;
      auto& tal = bk.top_side()._alist;

      auto set_txt_tb = [&](float ux, float uy) {
        Point ptr={(int)(ux* o_unit_bk_sz) ,(int)(uy* o_unit_bk_sz)};
        Point pts = o_start + ptr;
        auto of_x = ix - left;
        auto off_x = of_x * txt_w / tw;
        auto of_y = iy - top;
        auto off_y = of_y * txt_h / th;
    
        int tar_id=  of_y * tb_sz / th + of_x * txt_w / tw;
        auto& tar_unit = txt_dic[tar_id];
        tar_unit = {(uint16_t)pts.x,(uint16_t)pts.y};
      };
      uint16_t min_dis = -1;
      Point pt_min;
      auto find_pt = [&] {
        for (int iiy = 0; iiy < lcv.size(); iiy++) {
          Point pti = lcv[iiy];
          if (pti == pt) {
            auto uy = iiy / (float)lcv.size();
            set_txt_tb(0, uy);
            return true;
          }
          for (int iix = 0; iix < tal.size(); iix++) {
            auto ac = tal[iix];
            _move_fn[ac](pti);
            if (pti == pt) {
              auto ux = (iix + 1) / (float)(tal.size()+1);
              auto uy = iiy / (float)lcv.size();
              set_txt_tb(ux, uy);
              return true;
            }
            auto dis = getDis(pti, pt);
            if (dis < min_dis) {
              dis = min_dis;
              pt_min = { iix + 1,iiy };
            }
          }

        }
        return false;
      };
      auto hitted = find_pt();
      if (!hitted) {
        auto ux = pt_min.x / (float)(tal.size()+1);
        auto uy = pt_min.y / (float)lcv.size();
        set_txt_tb(ux,uy);
      }
    }
  }
#else
  uint vtop{ uimax }, vbottom{ 0 }, vleft{ uimax }, vright{ 0 };
  auto out_irect = [&](Point& pt) {
    bool be_outside= pt.x < left||pt.x>right||pt.y<top||pt.y>bottom;
    return be_outside;
  };
   
  for (int bk_id = 0; bk_id < _blocks.size(); bk_id++) {
    auto& bk = _blocks[bk_id];
    bool bk_out =  bk.right_side_x_max()<left||bk.left_side_x_min() > right
      ||bk.bottom_side_y_max()<top
      ||bk.top_side_y_min()>bottom;
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
      uint16_t off_y = sid * o_unit_bk_sz / (float)lsz+0.5f;
      while (pre_off_y <= off_y){
        uint16_t pre_off_x = 0;
        auto trace_distorted_pt = [&](Point& pt) {
          if (!out_irect(pt)) {
            //计算源坐标，即table of dic_txt的目标索引
            uint16_t off_x = tsid * o_unit_bk_sz / (float)curve_len+0.5f;
            if (0==pre_off_x) {
              pre_off_x = off_x;
            }
            while (pre_off_x <= off_x){
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
              uint16_t o_off_x = txt_w * i_off_x / (float)tw + 0.5f;
              auto i_off_y = pt.y - top;
              uint16_t o_off_y = txt_h * i_off_y / (float)th + 0.5f;
              auto dic_id = ori_from.y * txt_w + ori_from.x;
              txt_dic[dic_id] = { o_off_x ,o_off_y };
              pre_off_x++;
            }
            
          }
        };
        do {
          trace_distorted_pt(pt);
          if (tsid < tal.size()) {
            auto ac = tal[tsid++];
            _move_fn[ac](pt);
          } else {
            pt.x++;
          }
          pid++;
        } while (pid<curve_len);
        pid = 0;
        pt = lcv[sid];
        tsid = 0;
        pre_off_y++;
      }
    }

  }
#endif
  //去除空点
  for (int iy = vtop; iy <= vbottom; iy++) {
    for (int ix = vleft; ix <= vright; ix++) {
      auto pre_dic_id = iy * txt_w + ix - 1;
      auto dic_id = iy * txt_w + ix;
      auto next_dic_id = iy * txt_w + ix + 1;
      auto& pre_bk = txt_dic[pre_dic_id];
      auto& bk = txt_dic[dic_id];
      auto& next_bk = txt_dic[next_dic_id];
      if (bk.x == 0 && bk.y == 0
        && pre_bk.x!=0&& pre_bk.y != 0
        && next_bk.x!=0&&next_bk.y != 0
        ) {
        bk.y = (pre_bk.y+next_bk.y)/2;
        bk.x = (pre_bk.x + next_bk.x) / 2;
      }
    }
  }
  auto tm_after_tb = steady_clock::now();
  int dur = duration_cast<duration<int, std::milli>>(tm_after_tb - tm_before_tb).count();
  imshow("ori_curves", ori_curves);
  //KalmanFilter kf(4, 2);

  auto draw_sides_2_image = [&](segments& smt_list,Mat& image) {
    col_t = { 0,0,0 };
    int sd_id = 0;
    for (auto& sd : smt_list) {
      auto& cv = sd._cv;
      circle(image, cv[0], 3, {0,0,0}, 1, 8);
      auto iend = cv.size() - 1;
      auto mid_id = iend * 0.4;
      String cv_nm_str(std::to_string(sd_id));
      cv::polylines(image, cv, false, col_t());
      cv::putText(image, cv_nm_str, { cv[mid_id].x - 5,cv[mid_id].y }, cv::FONT_HERSHEY_SIMPLEX, 0.4, { 0,0,0,0 });
      sd_id++;
      col_t.next(50);
    }
  };
  draw_sides_2_image(sides_green, ori_green);
  draw_sides_2_image(sides_red, ori_red);
  draw_sides_2_image(_ref_sides, ori);
  col_t = { 0,0,0 };
  cv::line(ori, { (int)left,(int)top }, { (int)right,(int)top }, col_t(),1, LINE_4);
  cv::line(ori, { (int)left,(int)bottom }, { (int)right,(int)bottom }, col_t(),1, LINE_4);
  cv::line(ori, { (int)left,(int)top }, { (int)left,(int)bottom }, col_t(),1, LINE_4);
  cv::line(ori, { (int)right,(int)top }, { (int)right,(int)bottom }, col_t(), 1, LINE_4);

  ofstream of_txt_dic;
  string fname = "txt_dic_";
  fname += std::to_string(txt_w);
  fname += "_";
  fname += std::to_string(txt_h);
  fname += ".bin";
  of_txt_dic.open(fname, ios::binary);
  of_txt_dic.write((const char*)&txt_dic[0], sizeof(dic_uint)* txt_dic.size());
  of_txt_dic.close();
  imshow("ori_green", ori_green);
  //cv::imwrite("ori_green.png", ori_green);
  imshow("ori_red", ori_red);
  //cv::imwrite("ori_red.png", ori_red);

  imshow("ori", ori);
  cv::imwrite("ori.png", ori);
  cv::waitKey(0);
  return 0;/**/
  



  return 0;
}