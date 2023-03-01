#include<map>
namespace auto_future{
  template<class Key,class T> bool find_from_mp(T& receiver,Key& key,std::map<Key,T>& map_){
    auto& it_=map_.find(key);
    if(it_!=map_.end()){
      receiver=it_->second;
      return true;
    } else{ return false;}
  }
}