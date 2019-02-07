//
// Created by Martin Hermant on 24/10/2018.
//
#pragma once

#if 0

namespace AddressUtils{
    struct AddressIterator{
        AddressIterator(  const char * a):ad(a){
            int len = a?strlen(a):0;
            cur = len>0?(*a!='/'?nullptr:a):nullptr;
            nextc=cur;
            curlen=0;
        }
        int next(){
            if(cur==nullptr || *cur=='\0')return 0;
            cur=nextc+1;
            nextc = strchr (cur,'/');
            curlen=0;
            if(nextc==nullptr){curlen= strlen(cur);}
            else{curlen = nextc-cur;}
            return curlen;

        }
        bool isLast(){return nextc==nullptr;}
        Identifier getCur(){
            if(cur)
#ifdef ARDUINO
                return String(cur).substring(0,curlen);
#else
                return std::string(cur,curlen);
#endif
            else
                return "";
        }
        const char* getCurChrPtr(){
            return cur;
        }

    private:
        const char* cur;
        const char* ad;
        const char* nextc;
        int curlen;
    };


//    static std::string buildOSCAddress(ParameterNode * p,ParameterGroup *root){
//        if(!p->parentTree){return "";}
//        auto it = ParamIterators::FromRoot(p,root);
//        std::string res ;
//        it.next(); // jump root
//        while(auto pp= it.next()){
//            res+="/";res+=pp->getNameStr();
//        }
//        return res;
//
//    }

    ParameterNode * getNodeFromOSCAddress(ParameterGroup * root,const char * ad){
        if(!ad|| !root)return nullptr;
        AddressIterator it(ad);
        ParameterGroup * insp(root);
        AbstractParameter * res= nullptr;

        int len=it.next();

        if(len==0 && ( ad[0]!='/'||strlen(ad)==0)){return nullptr;}

        while(len&& insp) {
            if(it.isLast()){

                res = insp->getParameter((const char*)it.getCurChrPtr(),len);
                if(!res && couldBeInt(it.getCurChrPtr())){
                    // alow index based addresses
                    //if(auto pl = ParamCasting::toClass<ParameterList>(insp)){
                    char * p ;
                    int idx =atoi(it.getCurChrPtr()) ;
                    if(*p==0 && (idx<insp->leafs.size())){
                        res = insp->leafs[idx];
                    }
                    //}

                }
                if(res)
                    return res;
            }
            insp = insp->getGroup(it.getCur());
            if(it.isLast()){break;}
            len = it.next();
        }

        return insp;
    }

    AbstractParameterIterator::Ptr getParametersFromOSCAddress(ParameterGroup * root,const char * ad,int maxDepth = -1 ){
        ParameterNode * p = getNodeFromOSCAddress(root,ad);
        if(p){
#if DO_DBG
            if(debugParamsSetting)DBG.printf("getting Parameters from : %s (t:%s)",ad,p->getTypeStr().c_str());
#endif
            if(auto pp = ParamCasting::toClass<AbstractParameter>(p)){return std::unique_ptr<AbstractParameterIterator>{new ParamIterators::UniqueParamIterator(pp)};}
            else if(auto d = ParamCasting::toClass<ParameterList>(p)) {return std::unique_ptr<AbstractParameterIterator>{new  ParameterList::PListIterator(d)};}
            else if(auto d = ParamCasting::toClass<ParameterGroup>(p)) {return std::unique_ptr<AbstractParameterIterator>{new  ParamIterators::Trav_Param(d,maxDepth)};}
        }
#if DO_DBG
        DBG.printf("not found node %s",ad);
#endif
        return std::unique_ptr<AbstractParameterIterator>{new ParamIterators::NullParamIterator()};
    }

}//namespace  AdressUtils

#endif

