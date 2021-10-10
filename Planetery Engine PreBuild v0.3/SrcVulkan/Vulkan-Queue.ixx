export module Vulkan: Queue;
export import: Declaration;
import: Enum;
import std.core;
import Define;
import Util;
import "VulkanExtModule.h";

// Queue class:
export namespace vk {

	/// @addtogroup vkQueue Queue
	/// @ingroup vulkan
	/// A group of vulkan classes that is related to device queue.
	/// See also LogicalDevice.
	/// @{

	/// <summary>
	/// Hint struct for queues usage
	/// </summary>
	struct QueueHint {
		uint memoryIndex = uint(-1);
		uint presentIndex = uint(-1);
		uint renderIndex = uint(-1);
	};

	/// <summary>
	/// Hint enum
	/// </summary>
	enum HintUsage {
		Memory,	  ///< Memory Copy Operations
		Present,  ///< Presentation Operations
		Render,	  ///< Render Operations
	};

	/// <summary>
	/// QueuePool layout
	/// </summary>
	/// A layout object for setting up the desired QueuePool layout. On
	/// construction, this loads the constraint of the layout, which then can be
	/// queried via its const function. Call the non-const function to setup the
	/// desire layout. This object should then be provided to
	/// LogicalDevice::LogicalDeivce(PhysicalDeivce&, const QueuePoolLayout&)
	/// ctor. The LogicalDevice will then construct a QueuePool following this
	/// QueuePoolLayout.
	class QueuePoolLayout
	{
		friend class QueuePool;
		friend class LogicalDevice;
		// Called by LogicalDevice on creation.
		std::vector<VkDeviceQueueCreateInfo> getQueueCreateInfos() const;

	  public:
		/// Queue family support/constraint
		struct FamilySupport {
			/// Allowed queue operations
			Flags<QueueType> allowedUsages;
			/// Max allowed queue count
			uint maxCount;
			// TODO: Add Img Transfer min size here
		};
		// Note that there is no futher access to PhysicalDevice after Ctor

		/// <summary>
		/// Construct a QueuePool layout constrainted to the physical device
		/// </summary>
		/// <param name="pd">A physical device that the layout should be
		/// compatible with</param>
		/// @note There are no futher access to pd after the constructer returns
		QueuePoolLayout(const PhysicalDevice& pd);
		/// Get the allowed queue operations of a queue family
		/// <param name="familyIndex">The queue family index</param>
		/// <returns>The allowed queue opertions</returns>
		Flags<QueueType> getFamilySupportType(uint familyIndex) const;
		/// Get the max allowed queue count of a queue family
		/// <param name="familyIndex">The queue family index</param>
		/// <returns>The max allowed queue count</returns>
		uint getFamilyMaxQueueCount(uint familyIndex) const;
		/// Find a queue family filtered by its allowed queue operations
		/// <param name="mustSupport">The queue operations it must
		/// support</param>
		/// <param name="mustNotSupport">The queue operations it
		/// must not support</param>
		/// <returns>The queue family index</returns>
		/// @todo This will be changed to use Iteratiors or Ranges for better
		/// filtering
		uint findFamilyBySupportType(Flags<QueueType> mustSupport,
		  Flags<QueueType> mustNotSupport = Flags<QueueType>()) const;
		/// Check if a queue family supports presenting a SwapchainImage to the
		/// specified OSRenderSurface
		/// <param name="familyIndex">The queue family index</param>
		/// <param name="surface">The surface to check against</param>
		/// <returns>A boolean where \c true means it supports the
		/// surface</returns>
		bool checkQueuePresentSupport(
		  uint familyIndex, const OSRenderSurface& surface) const;
		/// Add a queue group to the layout
		/// <param name="familyIndex">The family index that the queue group
		/// should use</param> <param name="count">The max queue count of the
		/// queue group</param>
		/// @warning The family index must be unique currently as each queue
		/// families must only be linked to at most one queue group.
		/// @todo This behaviour may change later.
		void addQueueGroup(uint familyIndex, uint count);
		/// Provide a hint for how to use a group
		/// <param name="usage">The hinted usage</param>
		/// <param name="familyIndex">The group's familyIndex</param>
		void hintGroupUsage(HintUsage usage, uint familyIndex);

	  private:
		std::vector<std::pair<uint, uint>> queueGroups{};
		std::vector<FamilySupport> queueFamilyProperties{};
		QueueHint hint;
		VkPhysicalDevice pd;
	};

	/// <summary>
	/// Queue pool
	/// </summary>
	/// An object owned by a LogicalDevice. It manages the lifetime and usages
	/// of Queues.
	/// @warning This object should NEVER be constructed manually. Instead, this
	/// object is automatically constructed when constructing a LogicalDevice.
	///
	/// @p @anchor vkInUseQueue
	/// In-Use queues mean a queue that have been queried before using
	/// queryQueueNotInUse(), and that have yet called markQueueNotUnUse().
	///
	/// @todo Fix the In-Use queue machanic
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
		/// @todo Move this to private
		QueuePool(const QueuePoolLayout& layout, LogicalDevice& device);
		/// Destructer
		~QueuePool() = default;

		// TODO: Better search function via giving out Iterators (ranges?)
		/// Find a queue group filtered by its allowed queue operations
		/// <param name="mustSupport">The queue operations it must
		/// support</param>
		/// <param name="mustNotSupport">The queue operations it
		/// must not support</param>
		/// <returns>The queue family index of the group</returns>
		/// @todo This will be changed to use Iteratiors or Ranges for better
		/// filtering
		uint findGroupBySupportType(Flags<QueueType> mustSupport,
		  Flags<QueueType> mustNotSupport = Flags<QueueType>()) const;
		/// Query a Queue object that is \ref vkInUseQueue "in-use"
		/// <param name="groupId">The group index</param>
		/// <returns>A valid queue pointer, or \c nullptr if no queue is in
		/// use</returns>
		Queue* queryQueueInUse(uint groupId);
		/// Query a Queue object that is not \ref vkInUseQueue "in-use"
		/// <param name="groupId">The group index</param>
		/// <returns>A valid queue pointer, or \c nullptr if no queue is in
		/// use</returns>
		/// @note Concider calling markQueueNotInUse() when the queue is not
		/// needed anymore. This is only a suggestion as the \ref
		/// vkInUseQueue in-use thing is not really fully functional.
		Queue* queryQueueNotInUse(uint groupId);
		/// Mark a Queue object as not \ref vkInUseQueue "in-use"
		/// <param name="queue">The target Queue object</param>
		void markQueueNotInUse(Queue& queue);

		/// Get a Queue group using the hint provided when setting up the
		/// QueuePoolLayout
		/// <param name="usage">The type of usage</param>
		/// <returns>The Queue group index, or \c uint(-1) if the hint of the
		/// usage is not set</returns>
		/// @note This is faster than using findGroupBySupportType() as it uses
		/// constant time, independant of how many groups there are.
		uint getGroupByHint(HintUsage usage) const;
		/// Get the related LogicalDevice
		LogicalDevice& getDevice() const { return d; }

		// CommendPool& getCommendPool(Flags<CommendPoolType> type);
		// CommendBuffer getSingleUseCommendBuffer();

		/// <summary>
		/// Query a CommendPool for a Queue group
		/// </summary>
		/// Query a CommendPool that is compatible with the queue group. If one
		/// with the same CommendPoolType and in the same queue group is not
		/// present, it will make a new one.
		/// <param name="groupId">The queue group it sould be compatible
		/// to</param> <param name="type">The CommendPool type</param>
		/// <returns>A CommendPool</returns>
		/// @note The queried CommendPool's CommendBuffer should be submitable,
		/// and \b only submitable to a Queue in the targeted queue group. See
		/// also Queue::Submit()
		CommendPool& queryCommendPool(
		  uint groupId, Flags<CommendPoolType> type);

		// TODO: add priority for queues
		/// Get an express queue for Memory copy operations
		/// @todo Actually add priority for queues
		Queue& getExpressMemoryCopyQueue();
		/// Get an express queue for Render operations
		/// @todo Actually add priority for queues
		Queue& getExpressRenderQueue();

	  private:
		std::unordered_map<uint, Group> groups{};
		QueueHint hint;
	};

	/// <summary>
	/// A queue object for submiting commends
	/// </summary>
	/// @p This repesent a queue for the underlying LogicalDevice. It recieves
	/// commends via collecting CommendBuffer. See Queue::submit()
	///
	/// @p This should only be constructed via
	/// \link QueuePool::queryQueueNotInUse() QueuePool::query*() \endlink
	/// function.
	class Queue: public ComplexObject
	{
		friend class LogicalDevice;
		QueuePool& qp;
		VkQueue q;

	  public:
		/// Read-only value of the Queue's family index
		const uint familyIndex;
		/// Read-only value of the Queue index in a group
		const uint queueIndex;
		/// @todo move to private
		Queue(QueuePool& queuePool, uint familyIndex, uint queueIndex);
		Queue(const Queue&) = delete;
		Queue(Queue&&) = delete;
		/// <summary>
		/// Queue destructor
		/// </summary>
		/// Its underlying object's lifetime is linked to LogicalDevice and
		/// therefore no special destructor is needed
		~Queue() final = default;  // Lifetime linked to LogicalDevices
		/// Get the underlying LogicalDevice
		LogicalDevice& getDevice() { return qp.getDevice(); }
		/// Get the QueuePool that the Queue is in
		QueuePool& getQueuePool() { return qp; }

		// TODO: Add back multi-commend submitions

		/// <summary>
		/// Submit a CommendBuffer for execution
		/// </summary>
		/// The queue collects CommendBuffer to a multithread-safe queue, and
		/// after an undetermined delay, will start executing the
		/// CommendBuffers' commend in order and in parrellal.
		///
		/// <param name="cb">The commend buffer to submit</param>
		///
		/// <param name="waitFor">For cross queue syncing.\n
		/// What Semaphore object to wait for, or \c nullptr if no waiting is
		/// needed</param>
		///
		/// <param name="waitType">For same queue syncing.\n
		/// What PipelineStage of the previous commend to wait for</param>
		///
		/// <param name="signalToSemaphore">For cross queue syncing.\n
		/// What Semaphore object to signal to, or \c nullptr if no signaling is
		/// needed</param>
		///
		/// <param name="signalToFence">For syncing to user process.\n
		/// What Fence object to signal to, or \c nullptr if no signaling is
		/// needed</param>
		///
		/// @warning While CommendBuffer is executed in order,
		/// CommendBuffers dependent on each other submitted to the same queue
		/// still requires syncing via pipeline barriers or execution barriers
		/// as multiple commends (even across CommendBuffer).\n CommendBuffers
		/// dependent on each other submitted to different queues requires
		/// syncing via synchronization object. (see \link vkSync Sync \endlink)
		///
		/// @todo Add back multi-commend submitions in a single commend
		///
		/// @todo Add back TimelineSemaphore support
		void submit(CommendBuffer& cb, Semaphore* waitFor,
		  PipelineStage waitType, Semaphore* signalToSemaphore,
		  Fence* signalToFence);
		/// <summary>
		/// Presents a SwapchainImage to the Swapchain
		/// </summary>
		/// This presents a previously aquired SwapchainImage to the Swapchain.
		/// The execution order obeys the same rule as normal CommendBuffer
		/// submit(), except that it must be synchronized using Semaphore
		///
		/// <param name="scImg">The target SwapchainImage</param>
		///
		/// <param name="presentSemaphore">The Semaphore to wait for
		/// presentation</param>
		///
		/// <param name="cleanupFence">The Fence that is signaled when all
		/// previous commends that uses the Swapchain's LifetimeManager's
		/// allocated object has completed, and that cleanups can be
		/// called</param>
		/// @exception OutdatedSwapchainException Signal that Swapchain is
		/// outdated when trying to present the image
		void presentImage(SwapchainImage& scImg, Semaphore& presentSemaphore,
		  Fence& cleanupFence);
	};

	/// @}
}