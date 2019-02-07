//
// Created by Martin Hermant on 24/10/2018.
//

#pragma once
//
//
//////////////////
//// ParamIterators
////////////////////
//
//namespace ParamIterators{
//    struct Up_Group{
//        Up_Group(ParameterGroup* _p):p(_p){}
//        ParameterGroup * p;
//        bool next(){bool res= (p!=nullptr);if(res)p=p->parentTree;return res;}
//    };
//    struct Up_Node{
//        Up_Node(ParameterNode* _p,ParameterNode* _toRoot=nullptr):p(_p),toRoot(_toRoot){}
//        ParameterNode * p;
//        ParameterNode* toRoot;
//        ParameterNode* next(){ParameterNode* cur= p;p=(!p || p==toRoot)?nullptr:p->parentTree;return cur;}
//    };
//    struct FromRoot{
//        FromRoot(ParameterNode* p,ParameterNode* toRoot=nullptr){
//            Up_Node it(p,toRoot);
//            while(auto insp =it.next()){path.push_back(insp);}
//            idx = path.size()-1;
//        }
//        std::vector<ParameterNode *> path;
//        int idx;
//        ParameterNode* next(){ParameterNode* cur(nullptr);if(idx>=0){cur= path[idx];idx--;};return cur;}
//    };
//
//    struct Trav_Group{
//    public:
//        Trav_Group(ParameterGroup* _p,int _maxDepth=-1):origin(_p),current(_p),maxDepth(_maxDepth){poses.reserve(8);}
//        ParameterGroup * next(){
//            if(current==nullptr){return nullptr;}
//            if(current->trees.size() && !hasReachedMaxDepth()){
//                //go deeper leftmost while we can
//                poses.push_back(0);
//                current = current->trees.begin().second;
//                return current;
//            }
//            else{
//                // go up until unvisited
//                while(poses.size() && current->parentTree){
//                    int nextI = poses.back()+1;
//                    auto curList = current->parentTree->parentTree;
//                    if(nextI<curList.size()){
//                        poses.back()+=1;
//                        current = curList[nextI];
//                        return current;
//                    }
//                    poses.pop_back();
//                    current = current->parentTree;
//
//                }
//                current = nullptr;
//                return nullptr;
//            }
//        }
//
//        bool hasReachedMaxDepth(){
//            bool hasReached = (maxDepth>=0) && poses.size()>=maxDepth;
//#if DO_DBG
//            if(hasReached &&current->paramGroups.size() )DBG.println("stopping iterator , has reached maxDepth");
//#endif
//            return hasReached;}
//
//    private:
//        ParameterGroup * origin;
//        ParameterGroup* current;
//        std::vector<int> poses;
//        int maxDepth;
//
//    };
//
//    struct Trav_Param:public AbstractParameterIterator{
//        Trav_Param(ParameterGroup * pg,int maxDepth=-1):it(pg,maxDepth),count(0),current(pg){if(maxDepth==0)current=nullptr;}
//
//        AbstractParameter * next()override{
//            if(current==nullptr)return nullptr;
//            if(count<current->params.size()){
//                count++;
//                return current->params[count-1];
//            }
//            else{
//                count = 0;
//                while((current = it.next())){
//                    if(auto p = next()){return p;}
//                }
//                current=nullptr;
//                return nullptr;
//            }
//
//        }
//
//    private:
//        Trav_Group it;
//        ParameterGroup* current;
//        int count;
//    };
//
//    struct Trav_Node{
//        Trav_Node(ParameterGroup * pg,int maxDepth=-1):it(pg,maxDepth),count(0),current(pg){if(maxDepth==0)current=nullptr;}
//
//        ParameterNode * next(){
//            if(current==nullptr)return nullptr;
//            if(count<current->params.size()){
//                count++;
//                return current->params[count-1];
//            }
//            else{
//                count = 0;
//                current = it.next();
//                return current;
//            }
//
//        }
//
//    private:
//        Trav_Group it;
//        ParameterGroup* current;
//        int count;
//    };
//    struct NullParamIterator:public AbstractParameterIterator{
//        AbstractParameter * next(){return nullptr;};
//    };
//
//    struct UniqueParamIterator:public AbstractParameterIterator{
//        UniqueParamIterator(AbstractParameter  * _start):cur(_start){}
//        AbstractParameter * next(){auto res = cur; if(res)cur=nullptr;return res;};
//        AbstractParameter  * cur;
//    };
//
//    struct VectorParamIterator:public AbstractParameterIterator{
//        VectorParamIterator(const std::vector<AbstractParameter*> & all):v(all),count(-1){}
//        AbstractParameter * next(){count++;if(count>=v.size())return nullptr;return v[count];};
//        std::vector<AbstractParameter*> v;
//        int count;
//    };
//
//
//    template<typename T>
//    struct TypedPListIterator{
//        TypedPListIterator(TypedParameterList<T>* _pl):it(_pl){};
//        Parameter<T>* next(){return ParamCasting::to<T> (it.next());}
//        ParameterList::PListIterator it;
//    };
//
//
//    template<typename T>
//    struct TypedGroupIterator{
//        TypedGroupIterator(ParameterGroup * _pg,int _i=0,T * _v=nullptr):pg(_pg),i(_i),value(_v){}
//        TypedGroupIterator & begin(){
//            i=0;
//            value = nullptr;
//            getNext();
//            return *this;
//
//        }
//        TypedGroupIterator & end(){
//            static auto endIt = TypedGroupIterator(nullptr,i,nullptr);
//            return endIt;
//        }
//
//        void getNext(){
//            value = nullptr;
//            while(i<pg->paramGroups.size()){
//                if(auto d = dynamic_cast<T*>(pg->paramGroups[i])){
//                    value = d;
//                    break;
//                }
//                i++;
//            }
//        }
//        TypedGroupIterator & operator++(){
//            i++;
//            getNext();
//            return *this;
//        }
//        friend bool operator!= (TypedGroupIterator & l,TypedGroupIterator & r){
//            return l.value!=nullptr;
//        }
//        friend T& operator* (TypedGroupIterator & it){
//            return *it.value;
//        }
//        ParameterGroup * pg;
//        T * value;
//        int i ;
//    };
//
//}//namespace ParamIterators
//


