#pragma once
#include <string>
static bool endsWith (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}



static bool couldBeInt(const char * c){
  const char * i = c;
  while(isdigit(*(++i))){};
  return (i==0);
}
