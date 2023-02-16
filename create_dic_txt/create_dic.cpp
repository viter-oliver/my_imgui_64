#include "create_dic.h"
#include <functional>
using namespace std;
using namespace raw_image;
template<class T>
bool approximate(T value, T center, T range=10) {
	auto difference = value - center;
	return difference >= -range && difference <= range;
}
void create_dic(raw_image::img_raw& input_image, txt_dic& output_txt_dic) {
	const int height = input_image.size();
	const int width = input_image[0].size();
	const BYTE ref_b = 90;
	//detect ideal area
	int col_left = -1, col_right = -1, row_top = -1, row_bottom = -1;
	{
		auto col_is_within_area = [&](int ix) {
			for (int ii = 0; ii < height; ++ii) {
				if (!approximate(input_image[ii][ix].b, ref_b)) {
					return false;
				}
			}
			return true;
		};
		for (int ix = 0; ix < width; ix++) {
			if (col_left == -1 && col_is_within_area(ix)) {
				col_left = ix;
			}
			else if (col_right == -1 && !col_is_within_area(ix)) {
				col_right = ix - 1;
			}
			else {
				break;
			}
		}

		auto row_is_within_area = [&](int iy) {
			for (int ii = 0; ii < width; ++ii) {
				if (!approximate(input_image[iy][ii].b, ref_b)) {
					return false;
				}
			}
			return true;
		};

		for (int iy = 0; iy < height; iy++) {
			if (row_top == -1 && row_is_within_area(iy)) {
				row_top = iy;
			}
			else if (row_bottom == -1 && !row_is_within_area(iy)) {
				row_bottom = iy - 1;
			}
			else {
				break;
			}
		}
	}
	/*
	detect blocks of image
action:
	¨I	
	¡û   left move    
	¨L	 


	¨I	¡ü  ¨J
	   up move

		       ¨J
	right move  ¡ú
	           ¨K

    down move
	¨L	¡ý  ¨K
	

	*/

	enum move_unit_op {
		en_right_up,
		en_right,
		en_right_down,
		en_down_left,
		en_down,
		en_move_cnt
	};
	using point = txt_u;

	struct block {
		vector<BYTE> h_boder_ops;
		vector<BYTE> v_boder_ops;
		point o_point,rigt_top,left_bottom;
		pixel color_value;
	};

	using block_row = vector<block>;
	using block_dic = vector<block_row>;
	auto pixel_equal = [](pixel& px0, pixel& px1) ->bool {
		return approximate(px0.b, px1.b)
			&& approximate(px0.g, px1.g)
			&& approximate(px0.r, px1.r);
	};
    auto point_pixel_equal = [&](point pt0, point &pt1) {
          auto &px0 = input_image[pt0.iv][pt0.ih];
          auto &px1 = input_image[pt1.iv][pt1.ih];
          return pixel_equal(px0, px1);
    };
	
	block_dic _block_dic;
	const WORD unit = 20;
    const WORD block_width = width / unit;
    const WORD block_height = height / unit;
    _block_dic.resize(block_height, block_row(block_width));
	{

		
		// find orignal point;
		WORD ix = 0, iy = 0;
		while (!approximate(input_image[iy][ix].b, ref_b)) {
			if (iy == 0) {
				iy = ix + 1;
				ix = 0;

			} else {
				ix++;
				iy--;
			}
		}
                
		_block_dic[0][0].o_point = { iy,ix };

auto is_top_border = [&](point& pt) {
	auto iy = pt.iv;
	if (iy == 0) {
		return true;
	}
	auto ix = pt.ih;
	return !pixel_equal(input_image[iy][ix], input_image[iy - 1][ix]);
};
auto is_bottom_boder = [&](point& pt) {
	auto iy = pt.iv;
	if (iy == height) {
		return true;
	}
	auto ix = pt.ih;
	return !pixel_equal(input_image[iy][ix], input_image[iy + 1][ix]);
};
auto is_right_border = [&](point& pt) {
	auto ix = pt.ih;
	if (ix == width) {
		return true;
	}
	auto iy = pt.iv;
	return !pixel_equal(input_image[iy][ix], input_image[iy][ix + 1]);
};
auto is_left_border = [&](point& pt) {
	auto ix = pt.ih;
	if (ix == 0) {
		return true;
	}
	auto iy = pt.iv;
	return !pixel_equal(input_image[iy][ix], input_image[iy][ix - 1]);
};

//_block_dic[0][0].color_value = input_image[iy][ix];
auto move_by_top_boder = [&](point& pt_n) ->move_unit_op {
	point pt = pt_n;
	pt_n.ih += 1;
	if (point_pixel_equal(pt, pt_n)) {
		if (is_top_border(pt_n)) {
			return en_right;
		}
		else {
			pt_n.iv -= 1;
			return en_right_up;
		}
	}
	else {
		return en_right_down;
	}
};
auto move_by_left_boder = [&](point& pt_n) ->move_unit_op {
	point pt = pt_n;
	pt_n.iv += 1;
	if (point_pixel_equal(pt, pt_n)) {
		if (is_left_border(pt_n)) {
			return en_down;
		}
		else {
			pt_n.ih -= 1;
			return en_down_left;
		}
	}
	else {
		return en_right_down;
	}
};

for (int iv = 0; iv < block_height; iv++) {
	for (int ih = 0; ih < block_width; ih++) {
		block& bk_cur = _block_dic[iv][ih];
		if (ih > 0) {
			block& bk_pre = _block_dic[iv][ih - 1];
			bk_cur.o_point = bk_pre.rigt_top;
			bk_cur.o_point.ih++;
		}
		point pt0 = bk_cur.o_point;
		bk_cur.color_value = input_image[pt0.iv][pt0.ih];
		while (!is_right_border(pt0)) {
			auto op_type = move_by_top_boder(pt0);
			bk_cur.h_boder_ops.push_back(op_type);
		}
		bk_cur.rigt_top = pt0;
		pt0 = bk_cur.o_point;
		while (!is_bottom_boder(pt0)) {
			auto op_type = move_by_left_boder(pt0);
			bk_cur.v_boder_ops.push_back(op_type);
		}
		bk_cur.left_bottom = pt0;
	}
	if (iv > 0) {
		block& bk_head_prev = _block_dic[iv - 1][0];
		block& bk_head = _block_dic[iv][0];
		bk_head.o_point = bk_head_prev.left_bottom;
		bk_head.o_point.iv++;
	}
}

	}
	//iterate ideal area,find origanl coord of pixel of ideal area
	{
		function<void(point & pt)> move_point[en_move_cnt] = {
		//en_right_up
        [&](point& pt) {pt.ih++;pt.iv--;}, 
		//en_right, 
        [&](point& pt) {pt.ih++;}, 
		//en_right_down,
        [&](point& pt) {pt.ih++;pt.iv++;}, 
		//en_down_left,     
        [&](point& pt) {pt.ih--;pt.iv++;}, 
		//en_down,  
        [&](point& pt) {pt.iv++;}, 
		};
        auto find_coord = [&](point& coord_in, point& coord_out) {
            auto& px = input_image[coord_in.iv][coord_out.ih];
            for (WORD ix = 0; ix < block_height; ix++) {
              for (WORD iy = 0; iy < block_width; iy++) {
                auto& tar_bk = _block_dic[ix][iy];
                if (pixel_equal(px, tar_bk.color_value)) {
                  auto vopoint = tar_bk.o_point;  
                  auto& hops = tar_bk.h_boder_ops;
                  auto& vops = tar_bk.v_boder_ops;
                  WORD i = 0, j= 0;
                  for (i = 0; i < hops.size(); i++) {
                    auto hpoint = vopoint;
                    for (j = 0; j < vops.size(); j++) {
					    if (hpoint == coord_in) {
							coord_out = {(WORD)(iy * unit + j+1),
							(WORD)(ix * unit + i+1)};
							return;
						}
                        move_point[vops[j]](hpoint);
					}
                    if (hpoint.iv >= coord_in.iv) {
						coord_out = {
						(WORD)(iy * unit + j + 1),
						(WORD)(ix * unit + i + 1)};
						return;
                    }
                    move_point[hops[i]](vopoint);
				  }
				  //last row
                  auto hpoint = vopoint;
                  for (j = 0; j < vops.size(); j++) {
                    if (hpoint.ih == coord_in.ih) {
                      coord_out = {(WORD)(iy * unit + j + 1),
                                   (WORD)(ix * unit + i + 1)};
                      return;
                    }
                    move_point[vops[j]](hpoint);
                  }
				  //right down corner
				  coord_out = {(WORD)(iy * unit + j + 1),
							(WORD)(ix * unit + i + 1)};
				  return;

				}
              }
            }
		  
		};
        const int ideal_w = col_right - col_left + 1;
        const int ideal_h = row_bottom - row_top + 1;
		for (int iv = row_top; iv <= row_bottom; iv++) {
            for (int ih = col_left; ih <= col_right; ih++) {
              point cd_in = {iv, ih};
              point cd_out;
              find_coord(cd_in, cd_out);
			  
              auto ch = ih - col_left+1;
              float cdr_u = (float)ch / ideal_w;
              WORD oh = cdr_u * width;

              auto cv = iv - row_top+1;
              float cdr_v = (float)cv / ideal_h;
              WORD ov = cdr_v * height;

			  output_txt_dic[cd_out.iv][cd_out.ih] = {ov, oh};
            }
		}
    }
        
}