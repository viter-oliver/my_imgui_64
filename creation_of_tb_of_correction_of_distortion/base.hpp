#include <vector>
#include <opencv2/opencv.hpp>
namespace txt_dic{
  using namespace cv;
  struct dic_uint {
    uint16_t x, y;
  };
  const uint uimax = -1;
  using curve=std::vector<Point>;
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
    uint x_min{ uimax }, x_max{ 0 };
    uint y_min{ uimax }, y_max{ 0 };
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
          }
          else if (pt1.y > pt.y) {
            _alist.insert(_alist.begin(), right_down_move);
          }
          else {
            _alist.insert(_alist.begin(), right_move);
          }
        }
        else if (pt1.x < pt.y) {//left...
          if (pt1.y < pt.y) {
            _alist.insert(_alist.begin(), left_up_move);
          }
          else if (pt1.y > pt.y) {
            _alist.insert(_alist.begin(), left_down_move);
          }
          else {
            _alist.insert(_alist.begin(), left_move);
          }
        }
        else {
          if (pt1.y < pt.y) {
            _alist.insert(_alist.begin(), up_move);
          }
          else {
            _alist.insert(_alist.begin(), down_move);
          }
        }
      }

    }
    void append_point(Point& pt) {
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
      if (rm_sz > 0) {
        auto& end_pt = _cv[rm_sz - 1];
        if (pt.x > end_pt.x) {//right...
          if (pt.y < end_pt.y) {
            _alist.emplace_back(right_up_move);
          }
          else if (pt.y > end_pt.y) {
            _alist.emplace_back(right_down_move);
          }
          else {
            _alist.emplace_back(right_move);
          }
        }
        else if (pt.x < end_pt.y) {//left...
          if (pt.y < end_pt.y) {
            _alist.emplace_back(left_up_move);
          }
          else if (pt.y > end_pt.y) {
            _alist.emplace_back(left_down_move);
          }
          else {
            _alist.emplace_back(left_move);
          }
        }
        else {
          if (pt.y < end_pt.y) {
            _alist.emplace_back(up_move);
          }
          else {
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
      if (vertical() && _cv[0].y > _cv[eid].y || !vertical() && _cv[0].x > _cv[eid].x) {
        std::reverse(_cv.begin(), _cv.end());
        for (auto& m : _alist) {
          m = rev_actions[m];
        }
        std::reverse(_alist.begin(), _alist.end());
      }
    }
  };
  using segments = std::vector<side>;

  struct block {
    segments& _sides;
    Point ori_0;
    //index of block_sides;
    uint left_side_id{ 0 }, top_side_id{ 0 }, right_side_id{ 0 }, bottom_side_id{ 0 };
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
    DECLARE_RANGE_FUN(left, x)
    DECLARE_RANGE_FUN(right, x)
    DECLARE_RANGE_FUN(top, y)
    DECLARE_RANGE_FUN(bottom, y)
  };
  struct circle_iterable_curve {
    curve& _cv;
    int id{ 0 };
    void next_id() {
      id++;
      auto cv_len = _cv.size();
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
  using contour_list = std::vector<std::vector<cv::Point>>;
}
