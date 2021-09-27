export module Vulkan: Lifetime;
export import: Declaration;
import std.core;
import Define;
import Util;

// Lifetime class:
export namespace vk {
	template<class Obj, class... CtorArgs>
	concept MemoryResourceAware =
	  std::is_constructible_v<Obj, pmr::MemoryResource*, CtorArgs...>;
	template<class Obj, class... CtorArgs>
	concept MemoryResourceNotAware = std::is_constructible_v<Obj,
	  CtorArgs...> && !MemoryResourceAware<Obj, CtorArgs...>;

	class LifetimeManager
	{
	  public:
		LifetimeManager() = default;
		virtual ~LifetimeManager() = default;
		LifetimeManager(LifetimeManager&&) = delete;
		LifetimeManager(const LifetimeManager&) = delete;
		virtual pmr::MemoryResource* getResource() = 0;
		virtual void _recordNewObject(ComplexObject* obj) = 0;
		virtual void _recordDeleteObject(ComplexObject* obj) = 0;
		virtual void reset() = 0;
		template<class Obj, class... Args>
		requires MemoryResourceNotAware<Obj, Args...>
		inline Obj& make(Args&&... args) {
			pmr::MemoryResource* mr = getResource();
			Obj* obj = pmr::make<Obj>(mr, std::forward<Args>(args)...);
			_recordNewObject(obj);
			return *obj;
		}
		template<class Obj, class... Args>
		requires MemoryResourceAware<Obj, Args...>
		inline Obj& make(Args&&... args) {
			pmr::MemoryResource* mr = getResource();
			Obj* obj = pmr::make<Obj>(mr, mr, std::forward<Args>(args)...);
			_recordNewObject(obj);
			return *obj;
		}
		template<class Obj> inline void free(Obj& obj) {
			pmr::MemoryResource* mr = getResource();
			_recordDeleteObject(&obj);
			pmr::free(mr, &obj);
		}
	};

	class MonotonicLifetimeManager: public LifetimeManager
	{
	  public:
		MonotonicLifetimeManager() { _objs.make(&mr); }
		virtual ~MonotonicLifetimeManager() final override { reset(); }
		virtual pmr::MonotonicResource* getResource() final override {
			return &mr;
		}
		virtual void _recordNewObject(ComplexObject* obj) final override {
			_objs->push_front(obj);
		}
		virtual void _recordDeleteObject(ComplexObject* obj) final override {}
		virtual void reset() final override {
			for (auto& ptr : *_objs) { ptr->~ComplexObject(); }
			mr.release();
			// recall constructor without destructing as memory will be reset
			// via MemoryResource
			_objs.make(&mr);
		}
		template<class Obj, class... Args>
		requires MemoryResourceAware<Obj, Args...>
		inline Obj& make(Args&&... args) {
			pmr::MonotonicResource* mr = getResource();
			Obj* obj = pmr::make<Obj>(mr, mr, std::forward<Args>(args)...);
			_recordNewObject(obj);
			return *obj;
		}
		template<class Obj, class... Args>
		requires MemoryResourceNotAware<Obj, Args...>
		inline Obj& make(Args&&... args) {
			pmr::MonotonicResource* mr = getResource();
			Obj* obj = pmr::make<Obj>(mr, std::forward<Args>(args)...);
			_recordNewObject(obj);
			return *obj;
		}
		template<class Obj> inline void free(Obj& obj) {
			// No early free allowed in monotonic buffer. All free deferred to
			// buffer reset().
		}

	  private:
		pmr::MonotonicResource mr{};
		util::ManualLifetime<
		  pmr::ForwardListMR<ComplexObject*, pmr::MonotonicResource>>
		  _objs;
	};

	class HeapLifetimeManager: public LifetimeManager
	{
	  public:
		HeapLifetimeManager() {}
		virtual ~HeapLifetimeManager() final override {}
		HeapLifetimeManager(HeapLifetimeManager&&) = delete;
		HeapLifetimeManager(const HeapLifetimeManager&) = delete;
		// FIXME: It is supposed to use new delete resource but the linking to
		// said func is broken due to c++2y standard lib module feature
		virtual pmr::MemoryResource* getResource() final override {
			return std::pmr::get_default_resource();
		}
		// TODO: Add checks for memory leaks
		virtual void _recordNewObject(ComplexObject* obj) final override {}
		// TODO: Add checks for memory leaks
		virtual void _recordDeleteObject(ComplexObject* obj) final override {}
		// Note: Reset does nothing and does not free previous objs.
		virtual void reset() final override {}

		// Note: Because this use newDeleteResource, it calls
		// nonMemoryResourceAware version ctor and does not pass in memory
		// resource ptr, as it is implied that the nonMemoryResourceAware
		// version ctor uses defaut alloc.
		template<class Obj, class... Args>
		requires std::is_constructible_v<Obj, Args...>
		inline Obj& make(Args&&... args) {
			return new Obj(std::forward<Args>(args)...);
		}
		template<class Obj> inline void free(Obj& obj) { delete &obj; }

	  private:
		// Nothing here ;)
	};
	HeapLifetimeManager defaultLifetimeManager;
}
