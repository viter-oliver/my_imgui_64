#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <functional>
using namespace cv;
struct dic_uint {
  uint16_t x{ 0 }, y{ 0 };
};
int txt_w = 1920, txt_h = 720;
int bk_x_cnt = 24, bk_y_cnt = 9,o_bk_size=80;
const uint uimax = -1;
namespace block_detect {
  using namespace std;
  using curve = vector<Point>;
  /*
  
  direction of moving:
  ¨I  ¡ü  ¨J
  ¡û      ¡ú
  ¨L  ¡ý  ¨K

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
          if (pt1.y > pt.y) {
            _alist.insert(_alist.begin(), right_up_move);
          } else if (pt1.y > pt.y) {
            _alist.insert(_alist.begin(), right_down_move);
          } else {
            _alist.insert(_alist.begin(), right_move);
          }
        } else if (pt1.x < pt.y) {//left...
          if (pt1.y > pt.y) {
            _alist.insert(_alist.begin(), left_up_move);
          } else if (pt1.y > pt.y) {
            _alist.insert(_alist.begin(), left_down_move);
          } else {
            _alist.insert(_alist.begin(), left_move);
          }
        } else {
          if (pt1.y > pt.y) {
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
            if (pt.y > end_pt.y) {
              _alist.emplace_back(right_up_move);
            } else if (pt.y > end_pt.y) {
              _alist.emplace_back(right_down_move);
            } else {
              _alist.emplace_back(right_move);
            }
          } else if (pt.x < end_pt.y) {//left...
            if (pt.y > end_pt.y) {
              _alist.emplace_back(left_up_move);
            } else if (pt.y > end_pt.y) {
              _alist.emplace_back(left_down_move);
            } else {
              _alist.emplace_back(left_move);
            }
          } else {
            if (pt.y > end_pt.y) {
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
  segments _sides;
  struct block{
    //index of block_sides;
    Point ori;
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
  vector<block> _blocks;
  void o_coordinate(int block_id,Point& pt) {
    int bk_row_id = block_id / bk_x_cnt;
    int bk_col_id = block_id % bk_x_cnt;
    pt.y = bk_row_id * o_bk_size;
    pt.x = bk_col_id * o_bk_size;
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
        if (y<bk.ori.y&& bk.ori.y < bk.top_side_y_min()) {
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
                if (*current_row > 50) {
                    Sumpv += *current_row;
                    cnt_t++;
                }
            }
        }
        averane = Sumpv / cnt_t;
        //printf("value is %d\n", averane);
        cv::waitKey(0);
        return averane;
    }
}

void get_img_binary(Mat &image, Mat &b_image) {

    Mat gray = image.clone();
    uint w, h;
    w = gray.size().width;
    h = gray.size().height;

    //Crop Gray image
    const uint DIV_NUM = w;
    Mat cropped_gray_image, cropped_bin_image, resultImg;
    uint thresh;
    resultImg = Mat(h, w, CV_8UC1, Scalar::all(0));
    for (int cnt = 0; cnt < DIV_NUM; cnt++) {
        uint c_w, c_w_n;
        c_w = w / DIV_NUM * cnt;
        c_w_n = w / DIV_NUM * (cnt + 1);
        cropped_gray_image = gray(Range(0, h), Range(c_w, c_w_n));
        GaussianBlur(cropped_gray_image, cropped_gray_image, Size(5, 5), 0, 0);
        thresh = get_averange_gray(cropped_gray_image);
        //String thr_png = "cropped_gry_image_" + toString(cnt) + "-" + toString(thresh) + ".png";
        //imwrite(thr_png, cropped_gray_image);

        cv::threshold(cropped_gray_image, cropped_bin_image, thresh, 255, THRESH_BINARY);
        //thr_png = "cropped_bin_image_"+ toString(cnt) + "-" + toString(thresh) + ".png";
        //imwrite(thr_png, cropped_bin_image);
        Mat ROU = resultImg(Rect(c_w, 0, w / DIV_NUM, h));
        cropped_bin_image.copyTo(ROU);
    }
    cv::imshow("binimg", resultImg);
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
  cv::Mat img_bk;
  img.copyTo(img_bk);
  cv::Mat img_bin;
  cv::cvtColor(img_bk, img_bin, cv::COLOR_BGR2GRAY);
  cv::Mat img_blur;
  cv::blur(img_bin, img_blur, { 7,7 },{-1,-1});
  cv::Mat mask;
  cv::threshold(img_blur,mask,130, 255, cv::THRESH_BINARY);
  cv::Mat close_mask;
  //±ÕÔËËã£º³ýÈ¥Ð¡ºÚ¶´
  cv::Mat ele = getStructuringElement(MORPH_RECT, { 5,5 });
  cv::morphologyEx(mask, close_mask, MORPH_CLOSE, ele);
  cv::imshow("mask", mask);
  cv::imshow("close_mask", close_mask);
  cv::Mat dilate_close_mask;
  cv::dilate(close_mask, dilate_close_mask, ele, { -1,-1 }, 1);
  cv::Mat erode_dilate_close_mask;
  cv::erode(dilate_close_mask, erode_dilate_close_mask, ele, { -1,-1 }, 2);
  cv::imshow("erode_dilate_close_mask", erode_dilate_close_mask);

  std::vector<std::vector<cv::Point>> contours;

  cv::findContours(erode_dilate_close_mask, contours,  cv::RETR_LIST,cv::CHAIN_APPROX_NONE);
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
  std::vector<std::vector<cv::Point>> approxs(contours.size());
  using namespace block_detect;
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
  for (int ix = 0; ix < contours.size(); ix++) {
    auto& contour = contours[ix];
    if (contour.size() < 40) continue;
    //cv::drawContours(img_bk, contours, ix, col_d());
    //col_d.next();
    //auto epsilon = 0.1* cv::arcLength(contour, true);
    circle_iterable_curve tcv_contour = { contour, 0 };
    auto& approx= approxs[ix];
    cv::approxPolyDP(contour, approx, 8, true);
    circle_iterable_curve tcv_approx = { approx ,0 };
    int icnt_consume = 0;
    int sid = -1;

    while (icnt_consume!= contour.size()) {
      auto& cur_pt_contour = tcv_contour.cur_pt();
      auto& pre_pt_approx= tcv_approx.pre_pt();
      auto& cur_pt_approx = tcv_approx.cur_pt();
      if (cur_pt_contour == cur_pt_approx) {
        tcv_approx.next_id();
        auto& n_pt_approx= tcv_approx.cur_pt();
        auto v_chord_pre = cur_pt_approx - pre_pt_approx;
        auto v_chord = n_pt_approx - cur_pt_approx;
        auto sq_v_chord_pre = v_chord_pre.x * v_chord_pre.x + v_chord_pre.y * v_chord_pre.y;
        auto len_v_chord_pre = pow(sq_v_chord_pre, 0.5);
        auto sq_v_chord = v_chord.x * v_chord.x + v_chord.y * v_chord.y;
        auto len_v_chord = pow(sq_v_chord, 0.5);
        auto sq_chord = len_v_chord_pre * len_v_chord;
        auto cos_a = (v_chord_pre.x * v_chord.x + v_chord_pre.y * v_chord.y) / sq_chord;
        if (cos_a < 0.7) {
          if (sid > -1) {
            _sides[sid].sure_direct_right_or_down();
          }
          _sides.push_back(side());
          sid = _sides.size() - 1;
        }
        _sides[sid].append_point(cur_pt_contour);
        icnt_consume++;
        
        auto& cur_pt_ap= tcv_approx.cur_pt();
      } else if(sid>-1) {
        _sides[sid].append_point(cur_pt_contour);
        icnt_consume++;
      }
      tcv_contour.next_id();
    }
    _sides[sid].sure_direct_right_or_down();
    /*
    col_t = { 0,0,0 };
    for (int id = 0; id < approx.size(); id++) {
      circle(img_bk, approx[id], 4, col_t(), 1, 8);
      col_t.next(50);
    }
   */
  }

  std::sort(_sides.begin(), _sides.end(), [](side& s1, side& s2) {
    if (!s1.vertical() && !s2.vertical()|| s1.vertical() && s2.vertical()) {
      auto mid1 = s1.y_max + s1.y_min;
      auto mid2 = s2.y_max + s2.y_min; 
      return mid1 < mid2; 
    }  else  {
      return !s1.vertical() && s2.vertical();
    }
  });
  auto sort_by_xmin = [](side& s1, side& s2) {
    return s1.x_min < s2.x_min;
  };
  std::vector<uint> lines_id_range =
  { 0,12,36,60,84,108,132,156,180,204,216,
    240,264,288,312,336,360,384,408,431};
 
  for (int id = 0; id < lines_id_range.size()-1; id++) {
    std::sort(_sides.begin() + lines_id_range[id], _sides.begin() + lines_id_range[id + 1], sort_by_xmin);
  }
  
  //std::move(_sides.begin() + 216, _sides.begin() + 240, _sides.begin() + 12);
/**  std::sort(_sides.begin(), _sides.begin() + 12, sort_by_xmin);
  std::sort(_sides.begin()+12, _sides.begin() + 36, sort_by_xmin);
  std::sort(_sides.begin() + 36, _sides.begin() + 60, sort_by_xmin);
  
  std::sort(_sides.begin() + 60, _sides.begin() + 84, sort_by_xmin);
  std::sort(_sides.begin() + 84, _sides.begin() + 108, sort_by_xmin);
  std::sort(_sides.begin() + 108, _sides.begin() + 132, sort_by_xmin);
  std::sort(_sides.begin() + 132, _sides.begin() + 156, sort_by_xmin);
  std::sort(_sides.begin() + 156, _sides.begin() + 180, sort_by_xmin);
  std::sort(_sides.begin() + 180, _sides.begin() + 204, sort_by_xmin);
  std::sort(_sides.begin() + 204, _sides.begin() + 216, sort_by_xmin);
  std::sort(_sides.begin() + 216, _sides.begin() + 240, sort_by_xmin);
  std::sort(_sides.begin() + 240, _sides.begin() + 264, sort_by_xmin);
  std::sort(_sides.begin() + 264, _sides.begin() + 288, sort_by_xmin);
  std::sort(_sides.begin() + 288, _sides.begin() + 312, sort_by_xmin);
  std::sort(_sides.begin() + 312, _sides.begin() + 336, sort_by_xmin);
  std::sort(_sides.begin() + 336, _sides.begin() + 360, sort_by_xmin);
  std::sort(_sides.begin() + 360, _sides.begin() + 384, sort_by_xmin);
  std::sort(_sides.begin() + 384, _sides.begin() + 408, sort_by_xmin);
  std::sort(_sides.begin() + 408, _sides.begin() + 431, sort_by_xmin);

 //complement the block
   */
  auto line_points = [](Point& p0, Point& p1, std::vector<Point>& pt_list) {
    cv::LineIterator it(p0, p1);
    pt_list.resize(it.count);
    for (int i = 0; i < it.count; ++i, ++it) {
      pt_list[i] = it.pos();
    }
  };
  col_t = { 0,0,0 };
  int sd_id = 0;
  for (auto& sd : _sides) {
    auto& cv = sd._cv;
    circle(img_bk, cv[0], 1, col_t(), 1, 8);
    col_t.next(10);
    auto iend = cv.size() - 1;
    auto mid_id = iend * 0.2;
    String cv_nm_str(std::to_string(sd_id));
    //circle(img_bk, cv[iend], 4, col_t(), 1, 8);
    //col_t.next(50);
    //cv::drawContours(img_bk, cv, -1, col_t());
    cv::polylines(img_bk, cv, false, col_t());
    col_t.next(50);
    cv::putText(img_bk, cv_nm_str, cv[mid_id], cv::FONT_HERSHEY_SIMPLEX, 0.4, {0,0,0,0});
    sd_id++;
  }
#if 1
  std::vector<Point> corners;
  cv::goodFeaturesToTrack(erode_dilate_close_mask, corners, 800, 0.01, 50, Mat(), 6, false);
  std::sort(corners.begin(), corners.end(), [](Point& p1, Point& p2) {
    return p1.y< p2.y;
    });
  col_d = { 0,0,0 };
  std::vector<std::vector<Point>> hlines;
  std::vector<Point> hline;
  Point pt0=corners[0];
  for (auto& corner : corners) {
    auto delta_y = corner.y - pt0.y;
    if (abs(delta_y) > 20) {
        std::sort(hline.begin(), hline.end(), [](Point& p1, Point& p2) {
        return p1.x < p2.x;
        });
      hlines.emplace_back(hline);
      hline.clear();
      hline.emplace_back(corner);
    }
    else {
      hline.emplace_back(corner);
    }
    pt0 = corner;
  }
  std::sort(hline.begin(), hline.end(), [](Point& p1, Point& p2) {
    return p1.x < p2.x;
    });
  hlines.emplace_back(hline);
  int ix = 0;
  for (auto& hline : hlines) {
    for (auto& corner : hline) {
      circle(img, corner, 2, CV_RGB(col_d.r, col_d.g, col_d.b), 1, 8);
      auto num_str = std::to_string(ix);
      String cv_nm_str(num_str);
      cv::putText(img, cv_nm_str, corner, cv::FONT_HERSHEY_SIMPLEX, 0.4, CV_RGB(col_d.r, col_d.g, col_d.b));
      col_d.next(20);
      ix++;
    }
  }
#endif
  //create txt_dic
  if (0) {
  //find ideal rectangle
    auto& top_line = hlines[0];
    uint top{0}, bottom{ uimax }, left{0}, right{ uimax };
    for (auto& pt : top_line) {
      if (pt.y > top) {
        top = pt.y;
      }
    }
    auto v_cnt = hlines.size();
    auto& bottom_line = hlines[v_cnt - 1];
    for (auto& pt : bottom_line) {
      if (pt.y < bottom) {
        bottom = pt.y;
      }
    }
    for (int ix = 0; ix < v_cnt; ix++) {
      if (hlines[ix][0].x > left) {
        left = hlines[ix][0].x;
      }
    }
    auto h_cnt = hlines[0].size();
    for (int ix = 0; ix < v_cnt; ix++) {
      if (hlines[ix][h_cnt - 1].x < right) {
        right = hlines[ix][h_cnt - 1].x;
      }
    }
    auto tw = right - left;
    auto th = bottom - top;
    const float w_over_h_rate = (float) txt_w / (float)txt_h;
    auto itw = w_over_h_rate * th;
    if (tw > itw) { //too wide
      auto ad_w = tw - itw;
      right -= ad_w;
    } else { // too narrow
      const float h_over_w_rate = (float)txt_h / (float)txt_w;
      auto ith = tw * h_over_w_rate;
      auto ad_h = th - ith;
      bottom -= ad_h;
    }
    dic_uint** txt_dic=new dic_uint* [txt_h];
    for (int id = 0; id < txt_h; id++) {
      txt_dic[id] = new dic_uint[txt_w];
    }
    
    auto getDis = [](Point& p0, Point& p1) {
      return cv::abs(p0.x + p1.x) + cv::abs(p0.y + p1.y);
    };
    for (uint16_t iy = top; iy <= bottom; iy++) {
      for (uint16_t ix = left; ix <= right; ix++) {
        using namespace block_detect;
        auto id_block = within(ix, iy);
        Point o_start;
        o_coordinate(id_block, o_start);
        block& bk = _blocks[id_block];
        //detect relative coordinate of sample
        Point pt{ ix,iy };
        
        auto& lcv = bk.left_side()._cv;
        auto& tal = bk.top_side()._alist;
        auto cal_rel_coor = [lcv,tal](int x, int y,Point& pt) {
          pt.y = y * o_bk_size / lcv.size();
          pt.x = x * o_bk_size / (tal.size() + 1);
        };
        auto set_txt_tb = [cal_rel_coor, o_start, 
          txt_dic,ix,iy,left,top,tw,th](int x, int y) {
          Point ptr;
          cal_rel_coor(x, y, ptr);
          Point pts=o_start + ptr; 
          uint16_t xv = (ix - left) * txt_w / tw,yv=(iy - top) * txt_h / th;
          txt_dic[pts.y][pts.x] = { xv ,yv };
        };
        uint16_t min_dis = -1;
        Point pt_min;
        auto find_pt = [&] {
          for(int iiy=0;iiy<lcv.size();iiy++){
            Point pti = lcv[iiy];
            if (pti == pt) {
              set_txt_tb(0, iiy);
              return true;
            }
            for (int iix = 0; iix < tal.size(); iix++) {
              auto ac = tal[iix];
              _move_fn[ac](pti);
              if (pti == pt) {
                set_txt_tb(iix+1, iiy);
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
          set_txt_tb(pt_min.x, pt_min.y);
        }
      }
    }

    for (int id = 0; id < txt_h; id++) {
      delete txt_dic[id];
    }

    delete[] txt_dic;
  }
  /**
  for (int i = 0; i < corners.size(); ++i) {
    circle(img, corners[i], 8, CV_RGB(col_d.r, col_d.g, col_d.b), 1, 8);
    auto num_str = std::to_string(i);
    String cv_nm_str(num_str);
    cv::putText(img, cv_nm_str, corners[i], cv::FONT_HERSHEY_SIMPLEX, 0.4, CV_RGB(col_d.r, col_d.g, col_d.b));
    col_d.next(20);
  }
  */
 
  /**
  cv::Mat img_bin_r;
  cv::bitwise_not(img_bin, img_bin_r);
  cv::Mat mask_r;
  cv::threshold(img_bin_r, mask_r, 127, 255, cv::THRESH_BINARY);
  std::vector<std::vector<cv::Point>> contours1;
  cv::findContours(mask_r, contours1, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
  for (int ix = 0; ix < contours1.size(); ix++) {
    cv::drawContours(img_bk, contours1, ix, CV_RGB(col_d.r, col_d.g, col_d.b));
    col_d.next();
  }
  cv::imshow("mask_r", mask_r);
  */
  //cv::imshow("ori", img);
  //namedWindow("contours", cv::WINDOW_NORMAL);
  cv::imshow("contours", img_bk);
  cv::imwrite("contours.png", img_bk);
  cv::waitKey(0);
  return 0;
}