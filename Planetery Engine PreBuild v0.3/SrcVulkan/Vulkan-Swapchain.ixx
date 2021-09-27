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
	class Swapchain: public ComplexObject
	{
		friend class SwapchainImage;
		friend class OSRenderSurface;
		friend class Queue;
		void _make() noexcept(false);

		// Call via OSRenderSurface
	  public:
		static void setCallback(SwapchainCallback callback);
		static void setCallback(FrameCallback callback);
		// Lifetime - This obj should only be created via OSRenderSurface
		Swapchain(LogicalDevice& device, OSRenderSurface& surface);
		Swapchain(const Swapchain&) = delete;
		Swapchain(Swapchain&&) = delete;
		void rebuild() noexcept(false);
		~Swapchain() final;

		// Actions
		SwapchainImage* getNextImage(ulint timeout = ulint(-1)) noexcept(false);
		// Return false if image failed to render this time, and requires
		// querying swapchain again
		bool renderNextImage(ulint timeout = ulint(-1));
		void invalidateAllImages();	 // TODO
		void invalidateSwapchain();


		// Getters
		LogicalDevice& getLogicalDevice() const { return d; }
		OSRenderSurface& getRenderSurface() const { return sf; }
		MonotonicLifetimeManager& getPerSwapchainResource() {
			return perSwapchainResource;
		}
		uvec2 getPixelSize() const { return pixelSize; }
		bool isValid() const { return sc != nullptr && !outdated; }
		uint getImageCount() const { return imgs.size(); }
		VkFormat getImageFormat() const { return surfaceFormat.format; }
		LogicalDevice& getDevice() const { return d; }

	  private:
		LogicalDevice& d;
		OSRenderSurface& sf;
		VkSwapchainKHR sc;
		VkSurfaceFormatKHR surfaceFormat;
		uvec2 pixelSize;
		bool outdated = false;

		MonotonicLifetimeManager perSwapchainResource;

		std::vector<util::OptionalUniquePtr<SwapchainImage>> scImgs{};
		std::vector<VkImage> imgs{};
	};

	class OSRenderSurface
	{
		friend class QueuePoolLayout;
		friend class Swapchain;
		friend class PhysicalDevice;

	  public:
		class Support
		{
			friend class Swapchain;
			friend class PhysicalDevice;

		  public:
			Support(const PhysicalDevice& pd, const OSRenderSurface& surface);

			// Getter
			uvec2 calculateImageSize(uvec2 preferredSize) const;
			uint calculateImageCount(uint preferredCount) const;
			// TODO: make VKPresentModeKHR a enum class
			VkPresentModeKHR calculatePresentMode() const;
			VkSurfaceFormatKHR getFormat() const;

		  private:
			VkSurfaceCapabilitiesKHR capabilities{};
			// TODO: make VKSurfaceFormat mapping to VkFormat
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		// Lifetime
		OSRenderSurface();
		OSRenderSurface(const OSRenderSurface&) = delete;
		OSRenderSurface(OSRenderSurface&&) = delete;
		~OSRenderSurface();

		// Public values
		uvec2 preferredImageSize = uvec2(-1);
		uint preferredImageCount = 4;
		WindowTransparentType preferredTransparencyAction =
		  WindowTransparentType::RemoveAlpha;

		// Action

		Support querySupport(PhysicalDevice& pd) { return Support(pd, *this); }

		// May throw SurfaceMinimizedException
		Swapchain& querySwapchain(LogicalDevice& device);
		// May block until all Images are not in use
		void releaseSwapchain();

		// Getter
		// Does not change status or remake Swapchain.
		// Returns nullptr if Swapchain does not exist yet.
		Swapchain* getSwapchain() {
			return sc.has_value() ? &sc.value() : nullptr;
		}

	  private:
		Optional<Swapchain> sc;
		VkSurfaceKHR surface = nullptr;
	};

	struct OutdatedSwapchainException {};
	struct OutdatedFrameException {};
	class SwapchainImage: public ComplexObject
	{
		friend class Swapchain;
		friend class Queue;
		void setImageAquired(Semaphore&& sp);

	  public:
		// Lifetime
		SwapchainImage(Swapchain& sc, uint imgId, VkImage img, Semaphore&& sp);
		SwapchainImage(const SwapchainImage&) = delete;
		SwapchainImage(SwapchainImage&&) = delete;
		~SwapchainImage();

		// Action
		// Note: present() function is moved to queue object.
		// Note: Remember to set completion fence!

		// return false for timeout
		bool waitForCompletion(ulint timeout = -1) const;
		void invalidateImage();	 // TODO
		ImageView makeImageView() const;
		template<typename LM>
		requires std::derived_from<LM, LifetimeManager> ImageView&
		  makeImageView(LM& lm) const;

		// Getter
		bool hasAquiredImage() const {
			return std::holds_alternative<Semaphore*>(imgAquireSpOrCompleteFc);
		}
		Swapchain& getSwapchain() const { return sc; }
		LogicalDevice& getDevice() const { return sc.getDevice(); }
		Semaphore& getImageRecievedSemaphore() const {
			return *std::get<Semaphore*>(imgAquireSpOrCompleteFc);
		}
		MonotonicLifetimeManager& getPerFrameResource() {
			return perFrameResource;
		}
		MonotonicLifetimeManager& getPerImageResource() {
			return perImageResource;
		}
		VkFormat getFormat() const { return sc.getImageFormat(); }
		uint getImageIndex() const { return imgId; }

		VkImageViewCreateInfo _getImgViewInfo() const;

	  private:
		Variant<Semaphore*, Fence*> imgAquireSpOrCompleteFc;
		uint imgId;
		VkImage img;
		Swapchain& sc;
		MonotonicLifetimeManager perFrameResource{};
		MonotonicLifetimeManager perImageResource{};
	};

	template<typename LM>
	requires std::derived_from<LM, LifetimeManager> ImageView&
	  SwapchainImage::makeImageView(LM& lm) const {
		return lm.make<ImageView>(sc.getDevice(), _getImgViewInfo());
	}
}