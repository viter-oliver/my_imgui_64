#ifndef TOPIC_DEF_H
#define TOPIC_DEF_H
#include <string>
#include <vector>
struct base_topic {
  std::string name;
  std::string content;
  std::vector<std::string> answers;
};

#endif