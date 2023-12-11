#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
int main(int argc, char** argv) {
  if (argc < 2) {
    return 1;
  }
  auto img = cv::imread(argv[1],cv::IMREAD_GRAYSCALE);
  
  cv::Mat img_cpy;
  cv::bitwise_not(img, img_cpy);
  cv::Mat gray;
  img.copyTo(gray);
  //cv::cvtColor(img,gray, cv::COLOR_BGR2GRAY);
  auto kernel= cv::getStructuringElement(cv::MORPH_RECT, { 3, 3 });
  auto kernel1= cv::getStructuringElement(cv::MORPH_RECT, { 6, 3 });
  cv::Mat er_gray;
  cv::erode(gray, er_gray, kernel);
  cv::Mat dilated_er_gray;
  cv::dilate(er_gray, dilated_er_gray, kernel);
  cv::imshow("erode", er_gray);
  cv::imshow("erode_dilate", dilated_er_gray);
  cv::imshow("gray", gray);
  cv::Mat mask;
  cv::threshold(gray, mask, 130, 255, cv::THRESH_BINARY);
  /*std::vector<std::vector<cv::Point>> contours;
  cv::findContours(gray, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
  for (int ix = 0; ix < contours.size();ix++) {
    cv::drawContours(gray, contours, ix, (255,255, 255),1);
  }*/
  //find ideal rectangle

  cv::Rect rt_ideal_rectangle;
  auto img_sz = mask.size();
  //find left side and right side
  int col_left = 0, col_right = 0,row_top=0,row_bottom=0, max_col = 0;
  int limit = 1000;
  for (int col = 0; col < mask.cols; col++) {
    uint sum_row_clr = 0;
    for (int row = 0; row < mask.rows; row++){
      auto col_v= mask.at<uchar>(row, col);
      sum_row_clr += col_v;
    }
    auto dta = sum_row_clr - max_col;
    if (sum_row_clr>100&&dta < limit) {
      col_left = col;
      max_col = 0;
      break;
    }
    else if (sum_row_clr > max_col) {
      max_col = sum_row_clr;
      printf("%d,%d\n", sum_row_clr,dta);
    }
  }
  for (int col = mask.cols-1; col >=0 ; col--) {
    uint sum_row_clr = 0;
    for (int row = 0; row < mask.rows; row++) {
      auto col_v = mask.at<uchar>(row, col);
      sum_row_clr += col_v;
    }
    auto dta = sum_row_clr - max_col;
    if (sum_row_clr > 100 && dta < limit) {
      col_right = col;
      max_col = 0;
      break;
    }
    else if (sum_row_clr > max_col) {
      max_col = sum_row_clr;
      printf("~~~%d,%d\n", sum_row_clr, dta);
    }
  }
  limit = 280;
  for (int row = 0; row < mask.rows; row++) {
    uint sum_row_clr = 0;
    for (int col = 0; col < mask.cols; col++) {
      auto col_v = mask.at<uchar>(row, col);
      sum_row_clr += col_v;
    }
    auto dta = sum_row_clr - max_col;
    if (sum_row_clr > 160000 && dta < limit) {
      row_top = row;
      max_col = 0;
      break;
    }
    else if (sum_row_clr > max_col) {
      max_col = sum_row_clr;
      printf("__%d,%d\n", sum_row_clr, dta);
    }
  }
  limit = 500;
  for (int row = mask.rows-1; row >=0 ; row--) {
    uint sum_row_clr = 0;
    for (int col = 0; col < mask.cols; col++) {
      auto col_v = mask.at<uchar>(row, col);
      sum_row_clr += col_v;
    }
    auto dta = sum_row_clr - max_col;
    if (sum_row_clr > 100 && dta < limit) {
      row_bottom = row;
      max_col = 0;
      break;
    }
    else if (sum_row_clr > max_col) {
      max_col = sum_row_clr;
      printf("&&&%d,%d\n", sum_row_clr, dta);
    }
  }
  
  cv::line(img, { col_left, 0 }, { col_left, img_sz.height }, CV_RGB(255,0,0));
  cv::line(img, { col_right, 0 }, {col_right, img_sz.height }, CV_RGB(0, 255, 0));
  cv::line(img, { 0, row_top }, { img_sz.width, row_top }, CV_RGB(0, 0, 255));
  cv::line(img, { 0, row_bottom }, { img_sz.width, row_bottom }, CV_RGB(255, 0, 255));
  cv::String res = "ori";
  cv::imshow(res, img);

  //find top and bottom
  cv::imshow("mask", mask);
  cv::waitKey(0);


}