#pragma once

class NonAssignable {
private:
  NonAssignable(NonAssignable const&);
  NonAssignable& operator=(NonAssignable const&);
public:
  NonAssignable() {}
};

class ParameterGroup;

class ParamModifier{};

class AbstractParameterListener;


class ParameterNode:public NonAssignable{
public:
  ParameterNode(Identifier _name):name(_name),parentTree(nullptr){};
  virtual ~ParameterNode(){};
  Identifier name;
  std::string getNameStr(){return name.c_str();}
  virtual std::string getTypeStr() const = 0;
  virtual std::type_index getTypeId() const =0;
  ParameterGroup* parentTree;
  bool isChildOf(ParameterNode* p){
    ParameterNode * insp = this;
    while(insp = (ParameterNode*)insp->parentTree){
      if(insp==p){return true;}
    }
    return false;
  }

  
};

class AbstractParameter:public ParameterNode,public ParamModifier{
public:
  AbstractParameter(Identifier n):ParameterNode(n){};
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
//  ParamModifier * lockedModifier;
  
};

class AbstractParameterListener:public ParamModifier,public Weak<AbstractParameterListener> {
public:

    AbstractParameterListener():Weak(this){};
    virtual ~AbstractParameterListener(){};
    virtual void paramChanged(ParamModifier * caller,AbstractParameter* source){};


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
  std::string getValueAsString()override{
    return std::to_string(value);
  }
  
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

template <> std::string Parameter<std::string>::getValueAsString(){return value;}
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






