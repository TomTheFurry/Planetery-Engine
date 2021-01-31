#pragma once

#include "ModelBase.h"
#include "RenderThread.h"

class Bridge {
public:
	static void _callbackRender(void* const o, ulint ns) {
		((ModelBase*)o)->renderUpdate(ns);
		((ModelBase*)o)->glRender();
	}
	static RenderFunction _callbackInit(void* const o) {
		((ModelBase*)o)->glInit();
		return _callbackRender;
	}
	static void _callbackDest(void* const o) {
		delete ((ModelBase*)o);
	}
};


//TODO: add single-call, multi-ModelBase functions
template <class ModelType, class ...Tn>
inline ModelType* addModel(Tn&&... args) {
	ModelType* modelObj = new ModelType(std::forward<Tn>(args)...);
	RenderThread::addInitQueue(InitCallback(Bridge::_callbackInit, modelObj));
	return modelObj;
}

template <class ModelType>
inline ModelType* delModel(ModelType* const modelObj) {
	(static_cast<ModelBase* const>(modelObj))->destruct();
	RenderThread::addDestQueue(DestCallback(Bridge::_callbackDest, modelObj));
	return modelObj;
}