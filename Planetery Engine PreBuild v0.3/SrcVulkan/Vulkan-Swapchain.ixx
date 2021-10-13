export module Vulkan: Swapchain;
export import: Declaration;
import: Lifetime;
import: Enum;
import: Sync;
import std.core;
import Define;
import Util;
import "VulkanExtModule.h";

// Swapchain class:
export namespace vk {

	/// @addtogroup vkSwapchain Swapchain
	/// @ingroup vulkan
	/// A group of vulkan classes that is related to swapchain. All class in
	/// here is provided by default-included SwapchainKHR extension
	/// @{

	/// <summary>
	/// Swapchain object
	/// </summary>
	/// An object that holds and manages SwapchainImage. Swapchain may become
	/// outdated if the native OSRenderSurface's properties changed, such as
	/// window resizing. See OSRenderSurface on how to manage Swapchain
	/// lifetime.
	class Swapchain: public ComplexObject
	{
		friend class SwapchainImage;
		friend class OSRenderSurface;
		friend class Queue;
		void _make() noexcept(false);

		// Call via OSRenderSurface
	  public:
		/// Set a SwapchainCallback
		static void setCallback(SwapchainCallback callback);
		/// Set a FrameCallback
		static void setCallback(FrameCallback callback);
		// Lifetime - This obj should only be created via OSRenderSurface
		/// @todo move this to private
		Swapchain(LogicalDevice& device, OSRenderSurface& surface);
		Swapchain(const Swapchain&) = delete;
		Swapchain(Swapchain&&) = delete;
		/// @brief Rebuild the Swapchain \b (blocking)
		///
		/// @p This call will rebuild the Swapchain inplace. All SwapchainImage
		/// will be destructed even when rebuilds of the Swapchain fails, but
		/// not when exception is thrown.
		///
		/// @p Call to \c SwapchainCallback::onDestroy happens after the
		/// previous Swapchain becomes invalid, and all previous SwapchainImage
		/// has been destructed, which will \b block and wait until deallocation
		/// and destruction is completed.
		///
		/// @p Call to \c SwapchainCallback::onCreate happens only when the
		/// creation of the new Swapchain is successful, and after
		/// SwapchainCallback::onDestroy is called.
		///
		/// @exception SurfaceMinimizedException: Surface minimized.
		/// @note Call to SwapchainCallback::onSurfaceMinimized happens before
		/// the exception is thrown.
		void rebuild() noexcept(false);
		/// @brief Destructor \b (blocking)
		///
		/// @p This will first destruct all SwapchainImage, which will \b block
		/// and wait until deallocation and destruction is completed.
		///
		/// @p Call to SwapchainCallback::onDestroy happens after all
		/// SwapchainImage has been destructed.
		~Swapchain() final;

		// Actions
		/// @brief Aquire next SwapchainImage <b> (timed blocking) </b>
		///
		/// Try to aquire next SwapchainImage within the provided timeout. If
		/// timeout is 0, then this function is non-blocking. May set the
		/// outdated flag
		/// @param timeout The maximum nanoseconds the function can wait, or 0
		/// if non-blocking
		/// @return A valid SwapchainImage pointer, or nullptr if function fails
		/// to aquire the image in time
		SwapchainImage* getNextImage(ulint timeout = ulint(-1)) noexcept(false);
		// Return false if image failed to render this time, and requires
		// querying swapchain again
		/// @brief Aquire and render next SwapchainImage <b>(timed blocking)</b>
		///
		/// Try to aquire next SwapchainImage within the provided timeout. If
		/// successful, call the render function. May set the \c 'outdated' flag
		/// @param timeout The maximum nanoseconds the function can wait when
		/// trying to aquire the image, or 0 if non-blocking
		/// @return Whether a frame has been rendered successfully
		bool renderNextImage(ulint timeout = ulint(-1));
		/// Invalidate all SwapchainImages and reset them \b (blocking)
		/// @todo Add implementation
		void invalidateAllImages();	 // TODO: invalidateAllImages()
		/// Invalidate the Swapchain and set the \c 'outdated' flag
		void invalidateSwapchain();

		// Getters
		/// Get the related LogicalDevice
		LogicalDevice& getLogicalDevice() const { return d; }
		/// Get the related OSRenderSurface
		OSRenderSurface& getRenderSurface() const { return sf; }
		/// Get the LifetimeLanager for allocating objects whose lifetime tie to
		/// the Swapchain
		MonotonicLifetimeManager& getPerSwapchainResource() {
			return perSwapchainResource;
		}
		/// Get the SwapchainImage pixel size
		uvec2 getPixelSize() const { return pixelSize; }
		/// Check if Swapchain is valid and not outdated
		bool isValid() const { return sc != nullptr && !outdated; }
		/// Get the count of SwapchainImage
		uint getImageCount() const { return imgs.size(); }
		/// Get the format of SwapchainImage
		VkFormat getImageFormat() const { return surfaceFormat; }
		/// @bug Repeated func
		LogicalDevice& getDevice() const { return d; }

	  private:
		LogicalDevice& d;
		OSRenderSurface& sf;
		VkSwapchainKHR sc;
		VkFormat surfaceFormat;
		uvec2 pixelSize;
		bool outdated = false;

		MonotonicLifetimeManager perSwapchainResource;

		std::vector<util::OptionalUniquePtr<SwapchainImage>> scImgs{};
		std::vector<VkImage> imgs{};
	};

	/// @brief An object repesenting OS-specific surface
	///
	/// This is the display-part of a window if in windows mode, or the display
	/// if in full screen mode. It contains imformation about the supported
	/// display settings
	///
	/// @todo Seperate GLFW away from this class
	///
	class OSRenderSurface
	{
		friend class QueuePoolLayout;
		friend class Swapchain;
		friend class PhysicalDevice;

	  public:
		/// @brief Support class of OSRenderSurface
		///
		/// This class is a cache of the support of an OSRenderSurface. It
		/// provides access to get/calculate the supported display settings of
		/// an OSRenderSurface. calculate*() will get a property that is best
		/// matching to the \c 'perferred' input.
		class Support
		{
			friend class Swapchain;
			friend class PhysicalDevice;

		  public:
			/// Create a OSRenderSurface supports cache based on PhysicalDevice
			/// @param pd The PhysicalDevice that presentation commend will be
			/// submitted on
			/// @param surface The OSRenderSurface to get a support from
			Support(const PhysicalDevice& pd, const OSRenderSurface& surface);

			// Getter
			/// Calculate a best-matching image size based on the inputted
			/// \c 'preferredSize'
			/// @param preferredSize The preferred size of the SwapchainImage,
			/// or \c uvec2(-1) for getting the largest possible size
			/// @return A supported SwapchainImage size.
			/// @note It may return a \c uvec2(0). In that case, it means that
			/// no SwapchainImage should be made, and that making a valid
			/// Swapchain is not possible. An example is when the target window
			/// is minimized on Windows OS.
			uvec2 calculateImageSize(uvec2 preferredSize) const;
			/// Calculate a best-matching image count based on the inputted
			/// \c 'preferredCount'
			/// @param preferredCount The preferred numbers of SwapchainImage in
			/// a swapchain.
			/// @return A supported numbers of SwpachainImage.
			uint calculateImageCount(uint preferredCount) const;
			// TODO: make VKPresentModeKHR a enum class
			/// Calculate a present mode for creating a Swapchain
			VkPresentModeKHR calculatePresentMode() const;
			/// Get the format of the surface's SwapchainImage
			VkFormat getFormat() const;

		  private:
			VkSurfaceCapabilitiesKHR capabilities{};
			// TODO: make VKSurfaceFormat mapping to VkFormat
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		// Lifetime
		/// Make an OSRenderSurface
		/// @todo Add callbacks for getting a native surface, and put that to
		/// user-side
		OSRenderSurface();
		OSRenderSurface(const OSRenderSurface&) = delete;
		OSRenderSurface(OSRenderSurface&&) = delete;
		/// Destructor
		~OSRenderSurface();

		// Public values
		/// When making a Swapchain, what the preferred image size is.
		uvec2 preferredImageSize = uvec2(-1);
		/// When making a Swapchain, what the preferred image count is.
		uint preferredImageCount = 4;
		/// When making a Swapchain, what the @link SurfaceTransparentAction
		/// preferred transparency action @endlink is.
		SurfaceTransparentAction preferredTransparencyAction =
		  SurfaceTransparentAction::RemoveAlpha;

		// Action
		/// Query an OSRenderSurface support for this surface. See
		/// OSRenderSurface::Support::Support()
		Support querySupport(PhysicalDevice& pd) { return Support(pd, *this); }

		// May throw SurfaceMinimizedException
		/// @brief Query a Swapchain that is not outdated
		///
		/// If the current Swapchain is outdated, Rebuild the Swapchain and
		/// return it. If the current Swapchain is not outdated, return it.
		///
		/// @param device What LogicalDevice the Swapchain should be built on
		///
		/// @exception SurfaceMinimizedException: If the current Swapchain is
		/// outdated, and on rebuilding the Swapchain, the OSRenderSurface is
		/// detected to be minimized, this exception will be thrown to signal
		/// that the Surface had been minimized when it is rebuilding the
		/// Swapchain, in which the Swapchain will be set to be outdated.\n
		/// Call to SwapchainCallback::onSurfaceMinimized happens before the
		/// exception is thrown.
		Swapchain& querySwapchain(LogicalDevice& device);
		// May block until all Images are not in use
		/// Release and destroy the current Swapchain. (Blocking)
		void releaseSwapchain();

		// Getter
		// Does not change status or remake Swapchain.
		// Returns nullptr if Swapchain does not exist yet.

		/// @brief Get the current Swapchain
		///
		/// It returns the Swapchain directly, and does not update/remake the
		/// Swapchain.
		/// @return A pointer to the surface's valid Swapchain, or nullptr if a
		/// Swapchain does not exist yet.
		Swapchain* getSwapchain() {
			return sc.has_value() ? &sc.value() : nullptr;
		}

	  private:
		Optional<Swapchain> sc;
		VkSurfaceKHR surface = nullptr;
	};

	/// A signal that notifies the Swapchain is outdated. The Swapchain
	/// should be notified of the event by calling
	/// Swapchain::invalidateSwapchain() before throwing this exception.
	struct OutdatedSwapchainException {};

	/// A signal that notifies the SwapchainImage is outdated.
	/// @todo Add the functionality for invalidating a SwpachainImage
	struct OutdatedFrameException {};

	/// @brief An image of a Swapcain
	///
	///
	class SwapchainImage: public ComplexObject
	{
		friend class Swapchain;
		friend class Queue;
		void setImageAquired(Semaphore&& sp);

	  public:
		// Lifetime
		/// @todo Move to private, call by Swapchain
		SwapchainImage(Swapchain& sc, uint imgId, VkImage img, Semaphore&& sp);
		SwapchainImage(const SwapchainImage&) = delete;
		SwapchainImage(SwapchainImage&&) = delete;
		~SwapchainImage();

		// Action
		// Note: present() function is moved to queue object.
		// Note: Remember to set completion fence!

		// return false for timeout

		/// @brief Wait until the frame can be cleanup within the timeout.
		///
		/// If it timeout from waiting, return false.
		/// If the frame hasn't been sent, return false immidiatly.
		bool waitForCompletion(ulint timeout = -1) const;
		/// Invalidate the image, to trigger a reset.
		/// @todo Add this
		void invalidateImage();	 // TODO: add invalidateImage()
		/// Make an ImageView from this image
		ImageView makeImageView() const;
		/// Make an IamgeView using LM as the LifetimeMangager from this image
		template<typename LM>
		requires std::derived_from<LM, LifetimeManager> ImageView&
		  makeImageView(LM& lm) const;

		// Getter
		/// Check if it has aquired the image back yet
		bool hasAquiredImage() const {
			return std::holds_alternative<Semaphore*>(imgAquireSpOrCompleteFc);
		}
		/// Get the underlying Swapchain
		Swapchain& getSwapchain() const { return sc; }
		/// Get the underlying LogicalDevice
		LogicalDevice& getDevice() const { return sc.getDevice(); }
		/// Get the Semaphore that will be signaled when the image is recieved
		/// and is usable
		Semaphore& getImageRecievedSemaphore() const {
			return *std::get<Semaphore*>(imgAquireSpOrCompleteFc);
		}
		/// Get the Per-Frame LifetimeManager, which resets per frame.
		MonotonicLifetimeManager& getPerFrameResource() {
			return perFrameResource;
		}
		/// Get the Per-Image LifetimeManager, which resets when the
		/// SwapchainImage resets.
		MonotonicLifetimeManager& getPerImageResource() {
			return perImageResource;
		}
		/// Get the format of the SwapchainImage
		VkFormat getFormat() const { return sc.getImageFormat(); }
		/// Get the SwapchainImage index for its underlying Swapchain
		uint getImageIndex() const { return imgId; }
		/// Internal use
		/// @todo move to private
		VkImageViewCreateInfo _getImgViewInfo() const;

	  private:
		Variant<Semaphore*, Fence*> imgAquireSpOrCompleteFc;
		uint imgId;
		VkImage img;
		Swapchain& sc;
		MonotonicLifetimeManager perFrameResource{};
		MonotonicLifetimeManager perImageResource{};
	};


	// template func impl
	template<typename LM>
	requires std::derived_from<LM, LifetimeManager> ImageView&
	  SwapchainImage::makeImageView(LM& lm) const {
		return lm.make<ImageView>(sc.getDevice(), _getImgViewInfo());
	}

	/// @}
}