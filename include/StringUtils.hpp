#pragma once
#include <string>
static bool endsWith (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

static bool startsWith (std::string const &fullString, std::string const &start) {
    if (fullString.length() >= start.length()) {
        return (0 == fullString.compare (0, start.length(), start));
    } else {
        return false;
    }
}

static bool tryConsume(std::string &fullString, std::string const &start) {
    if (startsWith(fullString,start)) {
        fullString= fullString.substr(start.length());
        return true;
    } else {
        return false;
    }
}

static inline std::string enquote(std::string s,bool avoidDouble=true){
    if(!avoidDouble || s[0]!='"') { return "\"" + s + "\""; }
    return s;
}

static bool couldBeInt(const char * c){
  const char * i = c;
  while(isdigit(*(++i))){};
  return (i==0);
}
