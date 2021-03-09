#pragma once

#include <vector>
#include <memory>

#include "Define.h"
#include "DefineMath.h"
#include "DefineUtil.h"


/*
namespace gui {

	extern void init();

	class Base
	{
	  public:
		Base() = default;
		virtual ~Base() = 0;
	};

	template<typename obj> class Container
	{
	  public:
		~Container() = default;
	  protected:
		std::vector<std::unique_ptr<obj>> childs;
	};

	class AnchorPoint: protected Base, protected Container<AnchorPoint>
	{
	  public:
		virtual const vec2& operator*() const;
		virtual vec2& operator*();
		virtual void check();
		virtual void update();
		util::Trap<vec2> pos;
	};

	class AnchorPointOffset: protected AnchorPoint
	{
	  public:
		virtual const vec2& operator*() const;
		virtual vec2& operator*();
		virtual void check();
		virtual void update();
		util::Trap<vec2> pos;
	};

	class UpdateBase
	{
	  public:
		virtual void update(){};
		std::list<UpdateBase*> other;
		void connect(UpdateBase* v) {
			other.push_back(v);
			v->other.push_back(this);
		}
		void remove(UpdateBase* v) { other.remove(v); }
		void clear() {
			for (auto& p : other) p->remove(this);
			other.clear();
		}
		~UpdateBase() {
			for (auto& p : other) p->remove(this);
		}
	};
	class Point: UpdateBase
	{
	  public:
		vec2 pos;
		void valChanged() {
			for (auto& n : other) { n->update(); };
		}
		virtual void update() {}
	};

	template<typename T> class PointLink: UpdateBase
	{
	  public:
		T& t;
		PointLink(T& _t): UpdateBase(), t(_t) {}
		PointLink(T& _t, Point& _p): UpdateBase(), t(_t) { *this = _p; }
		PointLink& operator=(Point& p) {
			p.clear();
			p.connect(this);
		}
		virtual void update() { t.update(); }
	};

	class Rect
	{
		PointLink<Rect> bl;
		void check();
		void update();
	};

}
*/