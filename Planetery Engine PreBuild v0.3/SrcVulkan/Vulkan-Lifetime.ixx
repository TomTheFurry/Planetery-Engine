export module Vulkan: Lifetime;
export import: Declaration;
import std.core;
import Define;
import Util;

//Lifetime class:
export namespace vk {
	class LifetimeManager
	{
	  public:
		LifetimeManager() = default;
		virtual pmr::MemoryResource* getResource() = 0;
		virtual void recordNewObject(ComplexObject* obj) = 0;
		virtual void removeObject(ComplexObject* obj) = 0;
		virtual void reset() = 0;
		template<class Obj, class... Args>
		requires ComplexObjects<Obj> && std::is_constructible_v<Obj,
		  pmr::MemoryResource*, Args...>
		inline Obj* make(Args&&... args) {
			pmr::MemoryResource* mr = getResource();
			Obj* obj = pmr::make(mr, mr, std::forward<Args>(args)...);
			recordNewObject(obj);
			return obj;
		}
		template<class Obj, class... Args>
		requires ComplexObjects<Obj> && std::is_constructible_v<Obj, Args...>
		inline Obj* make(Args&&... args) {
			pmr::MemoryResource* mr = getResource();
			Obj* obj = pmr::make(mr, std::forward<Args>(args)...);
			recordNewObject(obj);
			return obj;
		}
	};

	class MonotonicLifetimeManager: public LifetimeManager
	{
	  public:
		MonotonicLifetimeManager() { _objs.make(&mr); }
		MonotonicLifetimeManager(MonotonicLifetimeManager&&) = delete;
		MonotonicLifetimeManager(const MonotonicLifetimeManager&) = delete;
		virtual pmr::MonotonicResource* getResource() final override {
			return &mr;
		}
		virtual void recordNewObject(ComplexObject* obj) final override {
			_objs->push_front(obj);
		}
		virtual void removeObject(ComplexObject* obj) final override {}
		virtual void reset() final override {
			for (auto& ptr : *_objs) { ptr->~ComplexObject(); }
			mr.release();
			// recall constructor without destructing as memory will be reset
			// via MemoryResource
			_objs.make(&mr);
		}
		template<class Obj, class... Args>
		requires ComplexObjects<Obj> && std::is_constructible_v<Obj,
		  pmr::MonotonicResource*, Args...>
		inline Obj* make(Args&&... args) {
			pmr::MonotonicResource* mr = getResource();
			Obj* obj = pmr::make<Obj>(mr, mr, std::forward<Args>(args)...);
			recordNewObject(obj);
			return obj;
		}
		template<class Obj, class... Args>
		requires ComplexObjects<Obj> && std::is_constructible_v<Obj, Args...>
		inline Obj* make(Args&&... args) {
			pmr::MonotonicResource* mr = getResource();
			Obj* obj = pmr::make<Obj>(mr, std::forward<Args>(args)...);
			recordNewObject(obj);
			return obj;
		}

	  private:
		pmr::MonotonicResource mr{};
		util::ManualLifetime<
		  pmr::ForwardListMR<ComplexObject*, pmr::MonotonicResource>>
		  _objs;
	};
}
