export module Vulkan: Queue;
export import: Declaration;
import: Enum;
import std.core;
import Define;
import Util;
import "VulkanExtModule.h";

// Queue class:
export namespace vk {

	struct QueueHint {
		uint memoryIndex = uint(-1);
		uint presentIndex = uint(-1);
		uint renderIndex = uint(-1);
	};
	enum HintUsage {
		Memory,
		Present,
		Render,
	};

	class QueuePoolLayout
	{
		friend class QueuePool;
		friend class LogicalDevice;
		// Called by LogicalDevice on creation.
		std::vector<VkDeviceQueueCreateInfo> getQueueCreateInfos() const;

	  public:
		struct FamilySupport {
			Flags<QueueType> allowedUsages;
			uint maxCount;
			// TODO: Add Img Transfer min size here
		};
		// Note that there is no futher access to PhysicalDevice after Ctor
		QueuePoolLayout(const PhysicalDevice& pd);

		Flags<QueueType> getFamilySupportType(uint familyIndex) const;
		// TODO: Better search function via giving out Iterators (ranges?)
		uint findFamilyBySupportType(Flags<QueueType> mustSupport,
		  Flags<QueueType> mustNotSupport = Flags<QueueType>()) const;
		uint getFamilyMaxQueueCount(uint familyIndex) const;
		bool checkQueuePresentSupport(
		  uint familyIndex, const OSRenderSurface& surface) const;

		void addQueueGroup(uint familyIndex, uint count);
		void hintGroupUsage(HintUsage usage, uint familyIndex);

	  private:
		std::vector<std::pair<uint, uint>> queueGroups{};
		std::vector<FamilySupport> queueFamilyProperties{};
		QueueHint hint;
		VkPhysicalDevice pd;
	};

	class QueuePool
	{
		struct Group {
			Flags<QueueType> allowedUsages;
			std::vector<util::OptionalUniquePtr<Queue>> queues{};
			std::map<uint, CommendPool> commendPools{};
			// TODO: Add Img Transfer min size here
		};

		LogicalDevice& d;

	  public:
		// Called by LogicalDevice after native logicalDevice constructed.
		QueuePool(const QueuePoolLayout& layout, LogicalDevice& device);
		~QueuePool() = default;

		// TODO: Better search function via giving out Iterators (ranges?)
		uint findGroupBySupportType(Flags<QueueType> mustSupport,
		  Flags<QueueType> mustNotSupport = Flags<QueueType>()) const;
		Queue* queryQueueInUse(uint groupId);
		Queue* queryQueueNotInUse(uint groupId);
		void markQueueNotInUse(Queue& queue);

		uint getGroupByHint(HintUsage usage) const;
		LogicalDevice& getDevice() const { return d; }

		// CommendPool& getCommendPool(Flags<CommendPoolType> type);
		// CommendBuffer getSingleUseCommendBuffer();
		CommendPool& queryCommendPool(
		  uint groupId, Flags<CommendPoolType> type);

		// TODO: add priority for queues
		Queue& getExpressMemoryCopyQueue();
		Queue& getExpressRenderQueue();

	  private:
		std::unordered_map<uint, Group> groups{};
		QueueHint hint;
	};


	class Queue: public ComplexObject
	{
		friend class LogicalDevice;
		QueuePool& qp;
		VkQueue q;

	  public:
		const uint familyIndex;
		const uint queueIndex;
		Queue(QueuePool& queuePool, uint familyIndex, uint queueIndex);
		Queue(const Queue&) = delete;
		Queue(Queue&&) = delete;
		~Queue() final = default;  // Lifetime linked to LogicalDevices
		LogicalDevice& getDevice() { return qp.getDevice(); }
		QueuePool& getQueuePool() { return qp; }

		// TODO: Add back multi-commend submittions
		void submit(CommendBuffer& cb, Semaphore* waitFor,
		  PipelineStage waitType, Semaphore* signalToSemaphore,
		  Fence* signalToFence);
		// Note: Throws OutdatedSwapchainException
		void presentImage(SwapchainImage& scImg, Semaphore& presentSemaphore,
		  Fence& cleanupFence);
	};

}