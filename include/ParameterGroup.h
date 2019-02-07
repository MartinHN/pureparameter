//
// Created by Martin Hermant on 24/10/2018.
//

#pragma once


//////////////
// ParamGroup
///////////////

class ParameterGroup :public TreeTempl<AbstractParameter,ParameterGroup,std::string,ParameterNode>,public AbstractParameterListener{
public:
    typedef  TreeTempl<AbstractParameter,ParameterGroup,std::string,ParameterNode> TreeType;
    template<class T>
    struct TH{
        typedef typename TreeType::PtrContainerHelper<T>::OPtr ParameterPtr;
        typedef typename TreeType::PtrContainerHelper<T>::OPtr ParameterGroupPtr;
    };


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
            *rootI = new ParameterGroup("root","root");
        }
        return *rootI;
    }


    ParameterGroup(Identifier n,Identifier _type="group"):TreeType(n),type(_type){}
    virtual ~ParameterGroup(){};


    virtual Identifier getTypeStr() const override {return type;}
    std::type_index getTypeId() const override {return typeid(this);}

    template<class T,class ...Args>
    typename TH<T>::ParameterGroupPtr addParameterGroup(Args... args){
        return addTree<T>(args...);
    }

    void removeParameterGroup(TreePtr pg){ removeTree(pg);}

    void removeParameter(LeafPtr p){removeLeaf(p);}
    TreePtr getParameterGroup(Identifier i){return getTree(i);}
    TreePtr getParameterGroup(const char* n,int len){return getParameterGroup(Identifier(n,len));}
    LeafPtr getParameter(Identifier i){ return getLeaf(i); }
    LeafPtr getParameter(const char* n,int len){return getParameter(Identifier(n,len));}




    typename TH<Parameter<TriggerType>>::ParameterPtr   addTrigger(Identifier n){
        return addLeaf<Parameter<TriggerType> >(n,TriggerType());
    }
    template<typename F>
    typename TH<Parameter<TriggerType >>::ParameterPtr addTrigger(Identifier n,F f){
        auto t = addTrigger(n);
        t.get()->addRTFunctionListener(f);
        return  t;
    }

    template<typename T>
    typename TH<NumericParameter<T>>::ParameterPtr addParameter(Identifier n,T val,T min,T max){
        return addLeaf<NumericParameter<T>>(n,val,min,max);
    }

    template<typename T,class ...Args>
    typename TH<Parameter<T>>::ParameterPtr addParameter(Identifier n,Args ... args){
        return addLeaf<Parameter<T>>(n,args...);
    }



    void paramChanged(ParamModifier * caller,AbstractParameter* source)override{
        dispatch_feedback(caller,source);
    };


    typedef AbstractParameterListener ParamGroupListener;
    typedef std::shared_ptr<ParamGroupListener*> ParamGroupListenerPtrRef;
    typedef std::shared_ptr<ParamGroupListener> ParamGroupListenerPtr;

    template<class ListenerT>
    void addParamGroupListener( ListenerT* l){paramGroupListeners.push_back(l->getWeakRef());}
    template<class ListenerT>
    void removeParamGroupListener( ListenerT * l){paramGroupListeners.erase( std::remove_if( paramGroupListeners.begin(), paramGroupListeners.end(), [l](const ParamGroupListenerPtrRef  x){return x.get()==l;} ), paramGroupListeners.end() );}
    Identifier type;

protected:

    virtual void leafAdded(ParameterGroup * o,AbstractParameter * p) override{ if(o==this)p->addListener(this); }
    virtual void leafRemoved(ParameterGroup *o,AbstractParameter * p) override{ if(o==this)p->removeListener(this); }


    std::vector<ParamGroupListenerPtrRef> paramGroupListeners;



    virtual void dispatch_feedback(ParamModifier * caller,AbstractParameter *p){
        for(auto l:paramGroupListeners){
            if(l.get()) {
#warning TODO
                //l->paramChanged(caller, p);
            }
        }
        if(parentTree.get()){parentTree.get()->dispatch_feedback(caller,p);}
    }

};

//
//class ParameterList : public ParameterGroup{
//public:
//    ParameterList(Identifier n):ParameterGroup(n){}
//    void addParameterGroup(ParameterGroup* p){
//#if DO_DBG
//        DBG.println("can't add group to list");
//#endif
//    }
//    struct PListIterator:public AbstractParameterIterator{
//    public:
//        PListIterator(ParameterList* _pl):pl(_pl){
//            if(pl){size = pl->leafs.size();}
//            count=-1;
//        };
//        ~PListIterator(){};
//        AbstractParameter* next()override{
//            count++;
//            if(!pl || count>=size)return nullptr;
//            return pl->leafs[count];;
//        }
//        ParameterList * pl;
//        int count;
//        int size;
//    };
//
//
//    AbstractParameter * getAtPos(int pos){
//        if(pos<leafs.size()){return leafs[pos];}
//        return nullptr;
//    }
//
//    template<typename T>
//    bool commitAtPos(int pos,const T & v,ParamModifier *  setter=nullptr,bool force=false){
//        if(auto p =ParamCasting::to<T>(getAtPos(pos))){
//            p->commitValue(v,setter,force);
//            return true;
//        }
//        return false;
//    }
//
//    template<typename T>
//    void fetchAllOfType(){
//        PListIterator it(this);
//        bool hasAtLeastOneFetch = false;
//        while(auto p=it.next()){
//            if(auto pp = ParamCasting::to<T>(p)){
//                hasAtLeastOneFetch =true;
//                pp->fetchValue();
//            }
//        }
//        if(hasAtLeastOneFetch){
//
//        }
//    }
//
//
//};
//
//
//template<typename T>
//class TypedParameterList : public ParameterList{
//public:
//
//
//    Parameter<T>* getAtPos(int pos){
//        if(pos<params.size()){return ParamCasting::to<T>(params[pos]);}
//        return nullptr;
//    }
//
//
//};

