//
// Created by Martin Hermant on 24/10/2018.
//

#pragma once

#include <stdlib.h>
template<class T>
class Weak{
public:
    typedef T* MasterRefType;
    typedef std::shared_ptr<MasterRefType> MasterPtr;
    typedef std::shared_ptr<MasterRefType> WeakPtr;

    Weak(T* obj){masterRef = std::make_shared<MasterRefType>(obj);}
    virtual ~Weak(){*masterRef=nullptr;}

    MasterPtr getMasterRef(){return masterRef;}
    WeakPtr getWeakRef(){return masterRef;}
private:
    MasterPtr masterRef;
};

