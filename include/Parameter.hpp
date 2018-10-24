#pragma once

class NonAssignable {
private:
  NonAssignable(NonAssignable const&);
  NonAssignable& operator=(NonAssignable const&);
public:
  NonAssignable() {}
};

class ParameterGroup;
class AbstractParameterListener;

class ParameterNode:public NonAssignable{
public:
  ParameterNode(Identifier _name):name(_name),parentGroup(nullptr){};
  virtual ~ParameterNode(){};
  Identifier name;
  std::string getNameStr(){return name.c_str();}
  virtual std::string getTypeStr() const = 0;
  virtual std::type_index getTypeId() const =0;
  ParameterGroup* parentGroup;
  bool isChildOf(ParameterNode* p){
    ParameterNode * insp = this;
    while(insp = (ParameterNode*)insp->parentGroup){
      if(insp==p){return true;}
    }
    return false;
  }

  
};

class ParamModifier{};

class AbstractParameter:public ParameterNode,public ParamModifier{
public:
  AbstractParameter(Identifier n):ParameterNode(n),lockedModifier(nullptr){};
  virtual ~AbstractParameter(){};
  #if USE_STR_CONV
  virtual String toString() = 0;
  virtual void setFromString(const String & in,AbstractParameterListener* from=nullptr,bool forceNotify=false) = 0;
  #endif
  virtual void addListener(AbstractParameterListener* l)=0;
  virtual void removeListener(AbstractParameterListener* l)=0;
  typedef std::unordered_map<std::type_index, std::string> tmap;
  static  tmap * getTmap(){static tmap* i(nullptr);if(!i){i = new tmap();}  return i;};
  virtual std::string getTypeNameStr()=0;
  virtual std::string getValueAsString()=0;
  //virtual std::type_index getType() =0;
  
private:
  ParamModifier * lockedModifier;
  
};


// static type map
//AbstractParameter::tmap AbstractParameter::parameter_tmap;

class AbstractParameterIterator{
public:
  virtual ~AbstractParameterIterator(){};
  virtual AbstractParameter* next()=0;
  //typedef ScopedPtr<AbstractParameterIterator> Ptr;
  typedef std::unique_ptr<AbstractParameterIterator> Ptr;

};


class AbstractParameterListener:public ParamModifier {
public:
  AbstractParameterListener(){};
  virtual ~AbstractParameterListener(){};
  virtual void paramChanged(ParamModifier * caller,AbstractParameter* source)=0;
  
};




template <typename T>
class Parameter : public AbstractParameter{

public:
  Parameter(Identifier n,T v):
          AbstractParameter(n),
          value(v),
          defaultValue(v),
          lastValue(v),
          isWaitingFetch(false){
                auto tid = getTypeId();
                auto entry = getTmap()->find(tid);
                if(entry == getTmap()->end())
                {
                  const std::string tname = getTypeNameStr();
                  getTmap()->operator[](tid) = tname;
                }

  }
  virtual ~Parameter(){};
  
  std::string getTypeNameStr()override{return type_to_string<T>();}
  std::string getValueAsString()override{return std::to_string(value);}
  
  void commitValue(const T& v,ParamModifier* from=nullptr,bool forceLaterFetch=false){
    setValue(v,from,false,false,true);
    if(forceLaterFetch)isWaitingFetch=true;
  }

  bool fetchValue(ParamModifier *from,bool forceNotify = false,bool silent=false){
    bool wasChanged = isWaitingFetch;
    if(!silent && (forceNotify || isWaitingFetch)){
      #if DO_DBG
      if(debugParamsSetting)DBG.printf("fetchingvalue :  %s from %p\n",name.c_str(),from);
      #endif
      notifyParameterChanged(from);
    }
    isWaitingFetch=false;
    return wasChanged;
  }
  
  void setValue(const T& v,ParamModifier* from,bool forceNotify=false,bool silentSet=false,bool onlyCommit=false){
    bool valueWantChange =  !isSame(v,value);
    #if DO_DBG
    if(debugParamsSetting &&!onlyCommit)DBG.printf("settingvalue :  %s :  from %p\n",name.c_str(),toString(),from);
    #endif
    lastValue = value;
    setValueInternal(v);
    bool valueHasChanged =  !isSame(value,lastValue);
    bool shouldNotify =  !silentSet && (forceNotify||valueWantChange);
    isWaitingFetch |= onlyCommit && valueWantChange;
    
    if(shouldNotify){
      
      
      bool hasBeenConstrained = valueWantChange!=valueHasChanged;
      ParamModifier * m = hasBeenConstrained?this:from; //allow feedback if parameter changed values
      
      notifyParameterChangedRT(m);
      if(!onlyCommit){notifyParameterChanged(m);}
      
      
    }
    
  }

  
  std::type_index getTypeId() const override {return typeid(value);}
  std::string getTypeStr() const override {return getTmap()->operator[](getTypeId());}

  typedef AbstractParameterListener Listener;
  template <typename F>
  class FunctionListener:public AbstractParameterListener{
  public:
    FunctionListener(F & f):function(f){};
    F function;
    virtual void paramChanged(ParamModifier * caller,AbstractParameter* source)override{
      if(caller!=this){function(((Parameter<T>*) source)->getValue());  }
    }
  };

template<typename F>
  void addFunctionListener(F f){listeners.push_back(new FunctionListener<F>(f));}
  void addListener(Listener * l)override{listeners.push_back(l);}
  void removeListener(Listener * l)override{listeners.erase( std::remove_if( listeners.begin(), listeners.end(), [l](Listener * x){return x==l;} ), listeners.end() );}

  template<typename F>
  void addRTFunctionListener(F f){rtListeners.push_back(new FunctionListener<F>(f));}
  void addRTListener(Listener * l){rtListeners.push_back(l);}
  void removeRTListener(Listener * l){rtListeners.erase( std::remove_if( rtListeners.begin(), rtListeners.end(), [l](Listener * x){return x==l;} ), rtListeners.end() );}


  void notifyParameterChanged(ParamModifier* from){for(auto l : listeners){l->paramChanged(from,this);}}
  void notifyParameterChangedRT(ParamModifier* from){for(auto l : rtListeners){l->paramChanged(from,this);}}
  const T& getValue()const{return value;}

protected:

  T value;
  T defaultValue;
  T lastValue;

  virtual void setValueInternal(const T & v){value=v;}
  virtual bool isSame(const T & v1,const T & v2){return v1==v2;}
#if USE_STR_CONV
  String toString() {        
    return String(value) ;
  }

  void setFromString(const String & in,AbstractParameterListener* from=nullptr,bool forceNotify=false);
#endif
private:
  std::vector<Listener *> listeners,rtListeners;
  bool isWaitingFetch;
  
};

class TriggerType{};
template<> bool Parameter<TriggerType>::isSame(const TriggerType & v1,const TriggerType & v2){return false;}
template <> std::string Parameter<TriggerType>::getTypeNameStr(){return "trigger";}
template <> std::string Parameter<TriggerType>::getValueAsString(){return "";}
template <> void Parameter<TriggerType>::setValueInternal(const TriggerType & v1){}

TriggerType TriggerImpl; // to use to pass fake member to setValue


#if USE_STR_CONV
template<> void Parameter<int>::setFromString(const String & in,AbstractParameterListener* from,bool forceNotify){ value  = in.toInt();}
template<> void Parameter<float>::setFromString(const String & in,AbstractParameterListener* from,bool forceNotify){ value  = in.toFloat();}
template<> void Parameter<String>::setFromString(const String & in,AbstractParameterListener* from,bool forceNotify){ value  = in;}
template<> void Parameter<bool>::setFromString(const String & in,AbstractParameterListener* from,bool forceNotify){ value  = in=="true";}
template<> void Parameter<TriggerType>::setFromString(const String & in,AbstractParameterListener* from,bool forceNotify){}
template<> String Parameter<TriggerType>::toString(){return "";}

#endif



#define HAS_DYNAMIC_CAST 1 // to implement for no_rtti
namespace ParamCasting{
  template <typename T>
  Parameter<T>* to(ParameterNode * n){
    #if HAS_DYNAMIC_CAST
    return dynamic_cast<Parameter<T> * >(n);
    #else
    // not implemen
    return nullptr;
    #endif
  }

    template <typename T>
  T* toClass(ParameterNode * n){
    #if HAS_DYNAMIC_CAST
    return dynamic_cast<T*>(n);
    #else
    // not implemented
    return nullptr;
    #endif
  }

}


constexpr int noMin = -9999;
constexpr int noMax = +9999;

template<typename T>
class NumericParameter : public Parameter<T>{
public:
  NumericParameter(Identifier n,T v,T _min=noMin,T _max=noMax):Parameter<T>(n,v),min(_min),max(_max){};
  
  void setValueInternal(const T & v)override{
    Parameter<T>::value = hasMax()?(v<max?v:max):v;
    Parameter<T>::value = hasMin()?(v>min?v:min):v;
  }
  bool hasMin()const {return min!=noMin;}
  bool hasMax()const {return max!=noMax;}
  
  T min;
  T max;
};



//////////////
// ParamGroup
///////////////

class ParameterGroup :public ParameterNode,public AbstractParameterListener{
public:


  static ParameterGroup * setRoot(ParameterGroup * r){
      ParameterGroup ** root = getRootI();
  #if DO_DBG
    if(*root){DBG.println("overriding root parameter");}
  #endif
    *root = r;
    return r;
  }
    static ParameterGroup ** getRootI(){
        static ParameterGroup ** rootI(nullptr);
        if(!rootI){
            rootI = new ParameterGroup*(nullptr);
        }
        return rootI;

  }
  static ParameterGroup * getRoot(){
      auto ** rootI = getRootI();
      if(!*rootI){
        *rootI = new ParameterGroup("root");
      }
      return *rootI;
  }

  
  ParameterGroup(Identifier n):ParameterNode(n){}
  virtual ~ParameterGroup(){};
  
  
  std::string getTypeStr() const override {static std::string tidstr ("group");return tidstr;}
  std::type_index getTypeId() const override {return typeid(this);}

    ParameterGroup* addParameterGroup(ParameterGroup* p){
    if( p->parentGroup){p->parentGroup->removeParameterGroup(p);}
    p->parentGroup = this;
    paramGroups.push_back(p);
    for(auto l:structListeners){l->groupAdded(this,p);}
      auto insp = this;
    while((insp = insp->parentGroup)){for(auto l:insp->structListeners){l->groupAdded(this,p);}}
    return p;
  }


  void addParameter(AbstractParameter * p){
   p->addListener(this);
   if(p->parentGroup){p->parentGroup->removeParameter(p); }
   p->parentGroup = this;
   params.push_back(p);
   
   for(auto l:structListeners){l->paramAdded(this,p);}
     auto insp = this;
   while((insp = insp->parentGroup)){for(auto l:insp->structListeners){l->paramAdded(this,p);}}
 }
 
 Parameter<TriggerType>*   addTrigger(Identifier n){
  auto np = new Parameter<TriggerType>(n,TriggerType());
  addParameter(np);
  
  return np;
}
  template<typename F>
Parameter<TriggerType >* addTrigger(Identifier n,F f){
  auto t = addTrigger(n);
  t->addRTFunctionListener(f);
  return  t;
}

  template<typename T>
Parameter<T>* addParameter(Identifier n,T val){
  auto np = new Parameter<T>(n,val);
  addParameter(np);
  return np;
}
    template<typename T>
NumericParameter<T>* addParameter(Identifier n,T val,T min,T max){
  auto np = new NumericParameter<T>(n,val,min,max);
  addParameter(np);
  return np;
}
void removeParameter(AbstractParameter *p){
  p->removeListener(this);
  for(auto l:structListeners){l->paramRemoved(this,p);}
   auto insp = this;
 while((insp = insp->parentGroup)){for(auto l:insp->structListeners){l->paramRemoved(this,p);}}
 for(auto pp = params.begin() ;pp!= params.end() ;++pp){
  if(*pp==p){params.erase(pp);break;}
}
p->parentGroup = nullptr;
}

void removeParameterGroup(ParameterGroup *pg){
  for(auto l:structListeners){l->groupRemoved(this,pg);}
   auto insp = this;
 while((insp = insp->parentGroup)){for(auto l:insp->structListeners){l->groupRemoved(this,pg);}}
 for(auto pp = paramGroups.begin() ;pp!= paramGroups.end() ;++pp){
  if(*pp==pg){paramGroups.erase(pp);break;}
}
pg->parentGroup = nullptr;

}

void paramChanged(ParamModifier * caller,AbstractParameter* source)override{
  dispatch_feedback(caller,source);
};


ParameterGroup * getGroup(Identifier i){
  auto it = find_if(paramGroups.begin(), paramGroups.end(), [i] (const ParameterGroup* pg) { return pg->name == i; } );
  if(it!=paramGroups.end()){return *it;}
  return nullptr;
}
ParameterGroup * getGroup(const char* n,int len){
  auto it = find_if(paramGroups.begin(), paramGroups.end(), 
    [n,len] (const ParameterGroup* pg) 
    { return strncmp(pg->name.c_str(),n,len)==0; } 
    );
  if(it!=paramGroups.end()){return *it;}
  return nullptr;
}

AbstractParameter * getParameter(Identifier i){
  auto it = find_if(params.begin(), params.end(), [i] (const AbstractParameter* p) { return p->name == i; } );
  if(it!=params.end()){return *it;}
  return nullptr;
}
AbstractParameter * getParameter(const char* n,int len){
  auto it = find_if(params.begin(), params.end(), [n,len] (const AbstractParameter* p) { return strncmp(p->name.c_str(),n,len)==0; } );
  if(it!=params.end()){return *it;}
  return nullptr;
}

class ParamGroupStructListener{
public:
  ParamGroupStructListener(){}
  virtual void paramAdded(ParameterGroup* caller,AbstractParameter* source)=0;
  virtual void paramRemoved(ParameterGroup* caller,AbstractParameter* source)=0;
  virtual void groupAdded(ParameterGroup* caller,ParameterGroup* source)=0;
  virtual void groupRemoved(ParameterGroup* caller,ParameterGroup* source)=0;

};

void addParamGroupStructListener( ParamGroupStructListener *l){structListeners.push_back(l);}
void removeParamGroupStructListener( ParamGroupStructListener *l){structListeners.erase( std::remove_if( structListeners.begin(), structListeners.end(), [l](const ParamGroupStructListener * x){return x==l;} ), structListeners.end() );}

typedef AbstractParameterListener ParamGroupListener;

void addParamGroupListener( ParamGroupListener *l){paramGroupListeners.push_back(l);}
void removeParamGroupListener( ParamGroupListener *l){paramGroupListeners.erase( std::remove_if( paramGroupListeners.begin(), paramGroupListeners.end(), [l](const ParamGroupListener * x){return x==l;} ), paramGroupListeners.end() );}

std::vector<AbstractParameter*> params;
std::vector<ParameterGroup*> paramGroups;
protected:
  std::vector<ParamGroupStructListener *> structListeners;
  std::vector<ParamGroupListener *> paramGroupListeners;



  virtual void dispatch_feedback(ParamModifier * caller,AbstractParameter *p){
    for(auto l:paramGroupListeners){l->paramChanged(caller,p);}
      if(parentGroup){parentGroup->dispatch_feedback(caller,p);}
  }
private:
  static ParameterGroup *rootParameter;
};


class ParameterList : public ParameterGroup{
public:
  ParameterList(Identifier n):ParameterGroup(n){}
  void addParameterGroup(ParameterGroup* p){
    #if DO_DBG
    DBG.println("can't add group to list");
    #endif
  }
  struct PListIterator:public AbstractParameterIterator{
  public:
    PListIterator(ParameterList* _pl):pl(_pl){
      if(pl){size = pl->params.size();}
      count=-1;
    };
    ~PListIterator(){};
    AbstractParameter* next()override{
      count++;
      if(!pl || count>=size)return nullptr;
      return pl->params[count];;
    }
    ParameterList * pl;
    int count;
    int size;
  };

  
  AbstractParameter * getAtPos(int pos){
    if(pos<params.size()){return params[pos];}
    return nullptr;
  }

template<typename T>
  bool commitAtPos(int pos,const T & v,ParamModifier *  setter=nullptr,bool force=false){
    if(auto p =ParamCasting::to<T>(getAtPos(pos))){
     p->commitValue(v,setter,force);
     return true;
   }
   return false;
 }

template<typename T>
 void fetchAllOfType(){
  PListIterator it(this);
  bool hasAtLeastOneFetch = false;
  while(auto p=it.next()){
    if(auto pp = ParamCasting::to<T>(p)){
      hasAtLeastOneFetch =true;
      pp->fetchValue();
    }
  }
  if(hasAtLeastOneFetch){
    
  }
}


};


template<typename T>
class TypedParameterList : public ParameterList{
public:


  Parameter<T>* getAtPos(int pos){
    if(pos<params.size()){return ParamCasting::to<T>(params[pos]);}
    return nullptr;
  }

  
};




