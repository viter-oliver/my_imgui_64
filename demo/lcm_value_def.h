enum obstacle_type{
  unclassified,
  unknownsmall,
  unknownbig,
  pedestrian,
  bicycle,
  car,
  truck,
  underdriveable,
  point,
  us_object,
  motorbike,
  cone
};
enum line_color{
  undecided,
  white,
  yellow_orange_red,
  blue_green,
  lane_color_cnt
};
enum line_type{
  undecided_type,
  solid,
  dashed,
  botts,
  deceleration,
  hov_lane,
  roadedge,
  dlm_not_dlm=30,
  dlm_solid_dashed,
  dlm_dashed_solid,
  dlm_solid_solid,
  dlm_dashed_dashed,
  dlm_undecided
};
enum warnning_level{
  no_alarm,
  first_alarm,
  second_alarm,
  warning_level_cnt
};
enum lane_change_state{
  keep_lane,
  prepare_change_left,
  prepare_change_right,
  change_left,
  change_right,
  abort,
};
enum drive_mode{
  manual,
  pilot_ready,
  pilot,
  parking_ready,
  parking,
  ACC,
  drive_mode_cnt
};
enum  {
  en_inv,
  en_inv1,
  en_inv2,
  en_inv3,
  en_inv4,
  en_D,
  en_N,
  en_R,
  en_P,
  en_g_cnt
};
