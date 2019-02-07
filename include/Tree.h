//
// Created by Martin Hermant on 24/10/2018.
//

#pragma once

template <class TreeClass,class LeafClass>
class AbstractTreeListener:public Weak<AbstractTreeListener<TreeClass,LeafClass>>{
public:

    AbstractTreeListener():Weak<AbstractTreeListener<TreeClass,LeafClass>>(this){}
    virtual ~AbstractTreeListener(){}
    virtual void leafAdded(TreeClass *caller,LeafClass *){};
    virtual void leafRemoved(TreeClass *caller,LeafClass *){};
    virtual void treeAdded(TreeClass *caller,TreeClass *){};
    virtual void treeRemoved(TreeClass *caller,TreeClass *){};

};

//template <class TreeClass,class LeafClass>
//class AbstractLeafListener{
//public:
//    virtual void leafChanged(LeafClass*) = 0;
//};



template<class Identifier>
class DefaultTreeNode{
public:
    DefaultTreeNode(Identifier n):name(n){};
    Identifier name;
};

template<class T>
class DummyPtr{
public:
    DummyPtr():ptr(nullptr){};
    DummyPtr(T* p):ptr(p){};
    T* get(){return ptr;}
    operator bool(){return ptr!=nullptr;}
    T* operator ->()const{return ptr;}
    T* ptr;
};

template <class Leaf,class Tree,class Identifier=std::string,class NodeType=DefaultTreeNode<Identifier>>
class TreeTempl : public NodeType,public AbstractTreeListener<Tree,Leaf>{
protected:
    template<typename T>
    struct PtrContainerHelper{
//        typedef std::unique_ptr<T> OPtr;
        typedef std::shared_ptr<T> OPtr;
        typedef std::weak_ptr<T> WPtr;

        typedef DummyPtr<T> DPtr;
//        typedef T* UPtr;
//        typedef T* WPtr;
        typedef std::vector<WPtr> RefArray ;
        typedef std::vector<OPtr> OwnedArray ;
        typedef std::unordered_map<Identifier,OPtr> OwnedMap ;
        typedef std::unordered_map<Identifier,WPtr> RefMap ;
    };
public:
    typedef typename PtrContainerHelper<Tree>::OPtr TreePtr;
    typedef typename PtrContainerHelper<Leaf>::OPtr LeafPtr;
    typedef AbstractTreeListener<Tree,Leaf> TreeListenerType;
    typedef typename PtrContainerHelper<Leaf>::OwnedMap LeafContainer;
    typedef typename PtrContainerHelper<Tree>::OwnedMap TreeContainer;
public:
    TreeTempl(Identifier &s):NodeType(s),TreeListenerType(),parentTree(nullptr){

    }
    virtual ~TreeTempl(){

    }

    LeafPtr getLeaf(Identifier & i) {return leafs.find(i)!=leafs.end()?leafs[i]:nullptr;}
    TreePtr getTree(Identifier & i) {return trees.find(i)!=trees.end()?trees[i]:nullptr;}

    typedef typename LeafContainer::iterator LeafIterator ;
    typedef typename TreeContainer::iterator TreeIterator;
    LeafContainer& getLeafs() {return leafs;};
    TreeContainer& getTrees() {return trees;};

    template<class T,class ...Args>
    typename PtrContainerHelper<T>::OPtr addTree(Args ...a){
        auto t =std::make_shared<T>(a...);

//        if(t->parentTree){
//             t->parentTree->removeTree(_t);
//        }
        t->parentTree=(Tree*)this;
        trees[t->name]=t;

        notifyListeners(&TreeListenerType::treeAdded,(Tree*)this,t.get());
        return t;
    };
    virtual TreePtr removeTree(TreePtr t){
        for(auto i:getTrees()){
            if(i.second==t){ trees.erase(i.first); break;}
        }
        notifyListeners(&TreeListenerType::treeRemoved,(Tree*)this,t.get());
        t->parentTree= nullptr;
        return t;
    };

    template<class T, class ...Args>
    typename PtrContainerHelper<T>::OPtr addLeaf(Args ... a) {
        auto l =std::make_shared<T>(a...);
        if(auto t = l->parentTree){ t->removeLeaf(l); }
        l->parentTree=(Tree*)this;
        leafs[l->name] = l;
        notifyListeners(&TreeListenerType::leafAdded,(Tree*)this,l.get());
        return l;
    };

    virtual LeafPtr removeLeaf(LeafPtr l) {
        for(auto i:getLeafs()){
            if(i.second==l){ leafs.erase(i.first); break;}
        }
        notifyListeners(&TreeListenerType::leafRemoved,(Tree*)this,l.get());
        l->parentTree= nullptr;
        return l;
    }

    //Listeners
    void addTreeListener(TreeListenerType * l){
        if(indexOfListener(l)==-1)treeListeners.push_back(l->getWeakRef());
    }
    void removeTreeListener(TreeListenerType * l) {
        if(int i = indexOfListener(l)){
            treeListeners.erase(i);
        }
    }
    int indexOfListener(TreeListenerType * l){
        int idx = 0;
        for(auto ll:treeListeners) {
            if(l==*ll){ return idx; }
            idx++;
        }
        return -1;
    }





    virtual void  leafAdded(Tree*,Leaf *)override{};
    virtual void  leafRemoved(Tree*,Leaf *)override{};
    virtual void  treeAdded(Tree*,Tree *)override{};
    virtual void  treeRemoved(Tree*,Tree *) override{};


    typename PtrContainerHelper<Tree>::DPtr parentTree;
    typename PtrContainerHelper<Leaf>::OwnedMap leafs;
    typename PtrContainerHelper<Tree>::OwnedMap trees;
protected:

    template<class F,class ...Args>
    void notifyListeners(F f,Tree* notifier,Args... a){
        (*this.*f)(notifier,a...);
        for(auto & l:treeListeners){
            if(auto lk = l.get()) {
                (**lk.*f)(notifier,a...);
            }
        }
        auto insp = this->parentTree.get();
        while(insp){
            (insp->notifyListeners)(f,notifier,a...);
            insp = insp->parentTree.get();
        }


    }


private:



    typename PtrContainerHelper<TreeListenerType*>::OwnedArray treeListeners;

};




