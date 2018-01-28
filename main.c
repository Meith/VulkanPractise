#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#define  WIDTH 800
#define  HEIGHT 600

#ifdef NDEBUG
	const int8_t enable_layers = 0;
#else
	const int8_t enable_layers = 1;
#endif

#define LAYER_COUNT 1
char *requested_layers[LAYER_COUNT] = { "VK_LAYER_LUNARG_standard_validation" };
char *available_layers[LAYER_COUNT] = { "" };
int available_layer_count = 0;

#define DEVICE_EXTENSION_COUNT 1
char *requested_device_extensions[DEVICE_EXTENSION_COUNT] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
char *available_device_extensions[DEVICE_EXTENSION_COUNT] = { "" };
int available_device_extension_count = 0;

uint32_t glfw_extension_count = 0;
const char **glfw_extensions;

VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *create_info, const VkAllocationCallbacks *allocator, VkDebugReportCallbackEXT *callback)
{
	PFN_vkCreateDebugReportCallbackEXT func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func == NULL)
		return VK_ERROR_EXTENSION_NOT_PRESENT;

	return func(instance, create_info, allocator, callback);
}

void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks *allocator)
{
	PFN_vkDestroyDebugReportCallbackEXT func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != NULL)
		func(instance, callback, allocator);
}

GLFWwindow *Create_Window();
VkInstance Create_Vulkan_Instance();
void Get_Available_Layers();
VkDebugReportCallbackEXT Setup_Debug_Callback(VkInstance instance);
static VKAPI_ATTR VkBool32 VKAPI_CALL Debug_Callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT obj_type, uint64_t obj, size_t location, int32_t code, const char *layer_prefix, const char *msg, void *user_data);
VkSurfaceKHR Create_Surface(VkInstance instance, GLFWwindow *window);
VkPhysicalDevice Select_Physical_Device(VkInstance instance);
uint8_t Check_Physical_Device_Suitability(VkPhysicalDevice physical_device);
int Get_Queue_Family_Index(VkPhysicalDevice physical_device, VkQueueFlagBits flag_bits);
VkDevice Create_Logical_Device(VkSurfaceKHR surface, VkPhysicalDevice physical_device, int *graphics_queue_family_index, VkQueue *graphics_queue);
void Get_Available_Device_Extensions(VkPhysicalDevice physical_device);
VkSwapchainKHR Create_Swapchain(VkSurfaceKHR surface, VkPhysicalDevice physical_device, VkDevice device, VkImage **swapchain_images, uint32_t *swapchain_image_count, VkFormat *swapchain_format, VkExtent2D *swapchain_extent);
VkSurfaceFormatKHR Choose_Surface_Format(VkSurfaceKHR surface, VkPhysicalDevice physical_device);
VkPresentModeKHR Choose_Present_Mode(VkSurfaceKHR surface, VkPhysicalDevice physical_device);
VkSurfaceCapabilitiesKHR Get_Surface_Capabilities(VkSurfaceKHR surface, VkPhysicalDevice physical_device);
VkExtent2D Get_Extent(VkSurfaceCapabilitiesKHR surface_capabilities);
uint32_t Create_Image_Views(VkDevice device, VkFormat swapchain_format, VkImage *swapchain_images, uint32_t swapchain_image_count, VkImageView **swapchain_image_views);
VkRenderPass Create_Render_Pass(VkDevice device, VkFormat swapchain_format);
VkPipeline Create_GraphicsPipeline(VkDevice device, VkExtent2D swapchain_extent, VkRenderPass render_pass, VkPipelineLayout *pipeline_out);
int Read_File(const char *file_name, char **file_contents);
VkShaderModule Create_Shader_Module(VkDevice device, char *shader_code, int chader_code_length);
int Create_Framebuffer(VkDevice device, VkExtent2D swapchain_extent, VkImageView *swapchain_image_views, uint32_t swapchain_image_view_count, VkRenderPass render_pass, VkFramebuffer **swapchain_framebuffer);
VkCommandPool Create_Command_Pool(VkDevice deivce, int graphics_queue_family_index);
int Create_Command_Buffers(VkDevice device, VkExtent2D swapchain_extent, VkRenderPass render_pass, VkPipeline graphics_pipeline, int swapchain_framebuffer_count, VkFramebuffer *swapchain_framebuffer, VkCommandPool command_pool, VkCommandBuffer **command_buffers);
VkSemaphore Create_Semaphore(VkDevice device);
void Render_Frame(VkDevice device, VkQueue graphics_queue, VkSwapchainKHR swapchain, VkCommandBuffer *command_buffers, VkSemaphore image_available_semaphore, VkSemaphore render_finished_semaphore);

int main(int argc, char *argv[])
{
	GLFWwindow *window = Create_Window();

	VkInstance instance = Create_Vulkan_Instance();
	VkDebugReportCallbackEXT callback = Setup_Debug_Callback(instance);
	VkSurfaceKHR surface = Create_Surface(instance, window);
	VkPhysicalDevice physical_device = Select_Physical_Device(instance);
	int graphics_queue_family_index;
	VkQueue graphics_queue;
	VkDevice device = Create_Logical_Device(surface, physical_device, &graphics_queue_family_index, &graphics_queue);
	VkImage *swapchain_images;
	uint32_t swapchain_image_count;
	VkFormat swapchain_format;
	VkExtent2D swapchain_extent;
	VkSwapchainKHR swapchain = Create_Swapchain(surface, physical_device, device, &swapchain_images, &swapchain_image_count, &swapchain_format, &swapchain_extent);
	VkImageView *swapchain_image_views;
	uint32_t swapchain_image_view_count = Create_Image_Views(device, swapchain_format, swapchain_images, swapchain_image_count, &swapchain_image_views);
	VkRenderPass render_pass = Create_Render_Pass(device, swapchain_format);
	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline = Create_GraphicsPipeline(device, swapchain_extent, render_pass, &pipeline_layout);
	VkFramebuffer *swapchain_framebuffer;
	int swapchain_framebuffer_count = Create_Framebuffer(device, swapchain_extent, swapchain_image_views, swapchain_image_view_count, render_pass, &swapchain_framebuffer);
	VkCommandPool command_pool = Create_Command_Pool(device, graphics_queue_family_index);
	int command_buffer_count;
	VkCommandBuffer *command_buffers;
	command_buffer_count = Create_Command_Buffers(device, swapchain_extent, render_pass, graphics_pipeline, swapchain_framebuffer_count, swapchain_framebuffer, command_pool, &command_buffers);	
	printf("Command buffer count: %d\n", command_buffer_count);
	VkSemaphore image_available_semaphore = Create_Semaphore(device);	
	VkSemaphore render_finished_semaphore = Create_Semaphore(device);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		Render_Frame(device, graphics_queue, swapchain, command_buffers, image_available_semaphore, render_finished_semaphore);
	}

	vkDeviceWaitIdle(device);

	vkDestroySemaphore(device, render_finished_semaphore, NULL);
	vkDestroySemaphore(device, image_available_semaphore, NULL);
	vkDestroyCommandPool(device, command_pool, NULL);
	free(command_buffers);
	for (int i = 0; i < swapchain_framebuffer_count; ++i)
		vkDestroyFramebuffer(device, swapchain_framebuffer[i], NULL);
	free(swapchain_framebuffer);
	vkDestroyPipeline(device, graphics_pipeline, NULL);
	vkDestroyPipelineLayout(device, pipeline_layout, NULL);
	vkDestroyRenderPass(device, render_pass, NULL);
	for (int i = 0; i < swapchain_image_view_count; ++i)
		vkDestroyImageView(device, swapchain_image_views[i], NULL);
	free(swapchain_image_views);
	vkDestroySwapchainKHR(device, swapchain, NULL);	
	free(swapchain_images);
	vkDestroyDevice(device, NULL);
	vkDestroySurfaceKHR(instance, surface, NULL);
	DestroyDebugReportCallbackEXT(instance, callback, NULL);
	vkDestroyInstance(instance, NULL);
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

GLFWwindow *Create_Window()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Window", NULL, NULL);

	return window;
}

VkInstance Create_Vulkan_Instance()
{
	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = NULL;
	app_info.pApplicationName = "Hello Triangle";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "No Engine";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.pApplicationInfo = &app_info;
	create_info.flags = 0;	

	glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

	if (enable_layers) {
		++glfw_extension_count;
		glfw_extensions = realloc(glfw_extensions, glfw_extension_count * sizeof(char *));
		glfw_extensions[glfw_extension_count - 1] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
	}

	create_info.enabledExtensionCount = glfw_extension_count;
	create_info.ppEnabledExtensionNames = glfw_extensions;

	printf("Added %d extensions.\n", create_info.enabledExtensionCount);
	for (int i = 0; i < create_info.enabledExtensionCount; ++i)
		printf("%s\n", create_info.ppEnabledExtensionNames[i]);

	Get_Available_Layers();

	if (enable_layers) {
		create_info.enabledLayerCount = available_layer_count;
		create_info.ppEnabledLayerNames = (const char * const *) available_layers;
		printf("Added %d layers.\n", create_info.enabledLayerCount);
		for (int i = 0; i < create_info.enabledLayerCount; ++i)
			printf("%s\n", create_info.ppEnabledLayerNames[i]);
	} else {
		create_info.enabledLayerCount = 0;
		create_info.ppEnabledLayerNames = NULL;
	}

	VkInstance instance;
	VkResult result = vkCreateInstance(&create_info, NULL, &instance);

	if (result != VK_SUCCESS) {
		printf("Failed to create instance: %d.\n", result);
		return NULL;
	}

	return instance;
}

void Get_Available_Layers()
{
	uint32_t layer_properties_count;
	vkEnumerateInstanceLayerProperties(&layer_properties_count, NULL);
	
	if (layer_properties_count == 0) {
		printf("Layers are not available.\n");
		return;
	}

	VkLayerProperties *layer_properties = malloc(layer_properties_count * sizeof(VkLayerProperties));
	vkEnumerateInstanceLayerProperties(&layer_properties_count, layer_properties);
	
	for (int i = 0; i < LAYER_COUNT; ++i) {
		for (int j = 0; j < layer_properties_count; ++j) {
			if (strcmp(requested_layers[i], layer_properties[j].layerName) != 0)
				continue;

			available_layers[available_layer_count++] = requested_layers[i];
		}
	}

	free(layer_properties);
}

VkDebugReportCallbackEXT Setup_Debug_Callback(VkInstance instance)
{
	if (!enable_layers)
		return NULL;

	VkDebugReportCallbackCreateInfoEXT create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	create_info.pfnCallback = Debug_Callback;

	VkDebugReportCallbackEXT callback;
	VkResult result = CreateDebugReportCallbackEXT(instance, &create_info, NULL, &callback);

	if (result != VK_SUCCESS) {
		printf("Failed to set up debug callback: %d.\n", result);
		return NULL;
	}

	return callback;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL Debug_Callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT obj_type, uint64_t obj, size_t location, int32_t code, const char *layer_prefix, const char *msg, void *user_data)
{
	printf("Validation error: %s\n", msg);

	return VK_FALSE;
}

VkSurfaceKHR Create_Surface(VkInstance instance, GLFWwindow *window)
{
	VkSurfaceKHR surface;
	VkResult result = glfwCreateWindowSurface(instance, window, NULL, &surface);

	if (result != VK_SUCCESS) {
		printf("Could not create window surface.\n");
		return NULL;
	}

	return surface;
}

VkPhysicalDevice Select_Physical_Device(VkInstance instance)
{
	uint32_t physical_device_count = 0;
	vkEnumeratePhysicalDevices(instance, &physical_device_count, NULL);

	if (physical_device_count == 0) {
		printf("Could not find any device that supports vulkan.\n");
		return VK_NULL_HANDLE;
	}

	VkPhysicalDevice *physical_devices = malloc(physical_device_count * sizeof(VkPhysicalDevice));
	vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices);

	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	for (int i = 0; i < physical_device_count; ++i) {
		if (Check_Physical_Device_Suitability(physical_devices[i])) {
			physical_device = physical_devices[i];
			break;
		}
	}

	free(physical_devices);
	
	if (physical_device == VK_NULL_HANDLE)
		printf("Could not find suitable device.\n");

	return physical_device;
}

uint8_t Check_Physical_Device_Suitability(VkPhysicalDevice physical_device)
{
	// Only requirement for current device is that it should have graphics and compute capabilities
	int graphics_queue_family_index = Get_Queue_Family_Index(physical_device, VK_QUEUE_GRAPHICS_BIT);
	int compute_queue_family_index = Get_Queue_Family_Index(physical_device, VK_QUEUE_COMPUTE_BIT);

	return (graphics_queue_family_index > -1 && compute_queue_family_index > -1);
}

int Get_Queue_Family_Index(VkPhysicalDevice physical_device, VkQueueFlagBits flag_bits)
{
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);

	VkQueueFamilyProperties *queue_families = malloc(queue_family_count * sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);

	int queue_family_index = -1;
	for (int i = 0; i < queue_family_count; ++i) {
		if (queue_families[i].queueCount > 0 && queue_families[i].queueFlags & flag_bits) {
			queue_family_index = i;			
			break;
		}
	}

	free(queue_families);
	return queue_family_index;
}

VkDevice Create_Logical_Device(VkSurfaceKHR surface, VkPhysicalDevice physical_device, int *graphics_queue_family_index, VkQueue *graphics_queue)
{
	VkDeviceQueueCreateInfo queue_create_info = {};
	queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.pNext = NULL;
	queue_create_info.queueFamilyIndex = (uint32_t) Get_Queue_Family_Index(physical_device, VK_QUEUE_GRAPHICS_BIT);
	queue_create_info.queueCount = 1;
	
	// make sure grpahics queue family index supports window presentation. Improve and make it more generalized later by choosing physical device which supports this feature.
	VkBool32 presentation_supported = 0;
	vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, queue_create_info.queueFamilyIndex, surface, &presentation_supported);
	if (!presentation_supported) {
		printf("Window presentation is not supported by queue.\n");
		return NULL;
	}
	
	*graphics_queue_family_index = queue_create_info.queueFamilyIndex;

	float queue_priority = 1.0f;
	queue_create_info.pQueuePriorities = &queue_priority;

	VkPhysicalDeviceFeatures device_features = {};

	VkDeviceCreateInfo device_create_info = {};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pNext = NULL;
	device_create_info.flags = 0;
	device_create_info.queueCreateInfoCount = 1;
	device_create_info.pQueueCreateInfos = &queue_create_info;
	device_create_info.pEnabledFeatures = &device_features;

	Get_Available_Device_Extensions(physical_device);
	device_create_info.enabledExtensionCount = available_device_extension_count;
	device_create_info.ppEnabledExtensionNames = (const char * const *) available_device_extensions;
	printf("Added %d physical device extensions.\n", device_create_info.enabledExtensionCount);
	for (int i = 0; i < device_create_info.enabledExtensionCount; ++i)
		printf("%s\n", device_create_info.ppEnabledExtensionNames[i]);
	
	if (enable_layers) {
		device_create_info.enabledLayerCount =	(uint32_t) available_layer_count;
		device_create_info.ppEnabledLayerNames = (const char * const *) available_layers;
	} else {
		device_create_info.enabledLayerCount = 0;
	}

	VkDevice device;
	VkResult result = vkCreateDevice(physical_device, &device_create_info, NULL, &device);
	if (result != VK_SUCCESS) {
		printf("Failed to create logical device.\n");
		return NULL;
	}


	vkGetDeviceQueue(device, queue_create_info.queueFamilyIndex, 0, graphics_queue);

	return device;
}

void Get_Available_Device_Extensions(VkPhysicalDevice physical_device)
{
	uint32_t device_extension_count;
	vkEnumerateDeviceExtensionProperties(physical_device, NULL, &device_extension_count, NULL);

	if (device_extension_count == 0) {
		printf("Physical device extensions are not available.\n");
		return;
	}

	VkExtensionProperties *extension_properties = malloc(device_extension_count * sizeof (VkExtensionProperties));
	vkEnumerateDeviceExtensionProperties(physical_device, NULL, &device_extension_count, extension_properties);

	for (int i = 0; i < DEVICE_EXTENSION_COUNT; ++i) {
		for (int j = 0; j < device_extension_count; ++j) {
			if (strcmp(requested_device_extensions[i], extension_properties[j].extensionName) != 0)
				continue;

			available_device_extensions[available_device_extension_count++] = requested_device_extensions[i];
		}
	}

	free(extension_properties);
}

VkSwapchainKHR Create_Swapchain(VkSurfaceKHR surface, VkPhysicalDevice physical_device, VkDevice device, VkImage **swapchain_images, uint32_t *swapchain_image_count, VkFormat *swapchain_format, VkExtent2D *swapchain_extent)
{
	VkSurfaceFormatKHR surface_format = Choose_Surface_Format(surface, physical_device);
	VkPresentModeKHR present_mode = Choose_Present_Mode(surface, physical_device);
	VkSurfaceCapabilitiesKHR surface_capabilities = Get_Surface_Capabilities(surface, physical_device);
	VkExtent2D extent = Get_Extent(surface_capabilities);

	uint32_t image_count = surface_capabilities.minImageCount + 1;
	if (surface_capabilities.maxImageCount > 0 && image_count > surface_capabilities.maxImageCount)
		image_count = surface_capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR swapchain_create_info = {};
	swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.pNext = NULL;
	swapchain_create_info.surface = surface;
	swapchain_create_info.minImageCount = image_count;
	swapchain_create_info.imageFormat = surface_format.format;
	swapchain_create_info.imageColorSpace = surface_format.colorSpace;
	swapchain_create_info.imageExtent = extent;
	swapchain_create_info.imageArrayLayers = 1;
	swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_create_info.queueFamilyIndexCount = 0;
	swapchain_create_info.pQueueFamilyIndices = NULL;
	swapchain_create_info.preTransform = surface_capabilities.currentTransform;
	swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_create_info.presentMode = present_mode;
	swapchain_create_info.clipped = VK_TRUE;
	swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;
	
	VkSwapchainKHR swapchain;
	VkResult result = vkCreateSwapchainKHR(device, &swapchain_create_info, NULL, &swapchain);
	if (result != VK_SUCCESS) {
		printf("Failed to create swap chain.\n");
		return NULL;
	}

	vkGetSwapchainImagesKHR(device, swapchain, swapchain_image_count, NULL);
	*swapchain_images = malloc((*swapchain_image_count) * sizeof(VkImage));
	vkGetSwapchainImagesKHR(device, swapchain, swapchain_image_count, *swapchain_images);

	*swapchain_format = swapchain_create_info.imageFormat;
	*swapchain_extent = swapchain_create_info.imageExtent;

	return swapchain;	
}

VkSurfaceFormatKHR Choose_Surface_Format(VkSurfaceKHR surface, VkPhysicalDevice physical_device)
{
	uint32_t surface_format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, NULL);

	if (surface_format_count == 0)
		printf("Failed to get surface format.\n");

	VkSurfaceFormatKHR *surface_formats = malloc(surface_format_count * sizeof(VkSurfaceFormatKHR));
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, surface_formats);
	
	VkSurfaceFormatKHR surface_format = {};
	if (surface_format_count == 1 && surface_formats[0].format == VK_FORMAT_UNDEFINED) {
		surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
		surface_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		free(surface_formats);
		return surface_format;
	}

	for (int i = 0; i < surface_format_count; ++i) {
		if (surface_formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			surface_format = surface_formats[i];
			break;
		}
	}

	free(surface_formats);
	
	return surface_format;
}

VkPresentModeKHR Choose_Present_Mode(VkSurfaceKHR surface, VkPhysicalDevice physical_device)
{
	uint32_t present_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, NULL);

	if (present_mode_count == 0)
		printf("Could not find present mode.\n");

	VkPresentModeKHR *present_modes = malloc(present_mode_count * sizeof(VkPresentModeKHR));
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes);
	
	VkPresentModeKHR present_mode;
	for (int i = 0; i < present_mode_count; ++i) {
		if (present_modes[i] == VK_PRESENT_MODE_FIFO_KHR) {
			present_mode = present_modes[i];
			break;
		}
	}

	free(present_modes);
	
	return present_mode;
}

VkSurfaceCapabilitiesKHR Get_Surface_Capabilities(VkSurfaceKHR surface, VkPhysicalDevice physical_device)
{
	VkSurfaceCapabilitiesKHR surface_capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);
	return surface_capabilities;
}

VkExtent2D Get_Extent(VkSurfaceCapabilitiesKHR surface_capabilities)
{
	if (surface_capabilities.currentExtent.width != UINT_MAX)
		return surface_capabilities.currentExtent;

	VkExtent2D extent = { WIDTH, HEIGHT };
	return extent;
}

uint32_t Create_Image_Views(VkDevice device, VkFormat swapchain_format, VkImage *swapchain_images, uint32_t swapchain_image_count, VkImageView **swapchain_image_views)
{
	uint32_t swapchain_image_view_count = swapchain_image_count;
	*swapchain_image_views = malloc(swapchain_image_view_count * sizeof(VkImageView));

	for (int i = 0; i < swapchain_image_view_count; ++i) {
		VkImageViewCreateInfo image_view_create_info = {};
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.pNext = NULL;
		image_view_create_info.flags = 0;
		image_view_create_info.image = swapchain_images[i];
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format = swapchain_format;
		image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_create_info.subresourceRange.baseMipLevel = 0;
		image_view_create_info.subresourceRange.levelCount = 1; 
		image_view_create_info.subresourceRange.baseArrayLayer = 0;
		image_view_create_info.subresourceRange.layerCount = 1;

		VkResult result = vkCreateImageView(device, &image_view_create_info, NULL, &((*swapchain_image_views)[i]));
		if (result != VK_SUCCESS) {
			printf("Could not create image view.\n");
		}
	}

	return swapchain_image_view_count;
}

VkRenderPass Create_Render_Pass(VkDevice device, VkFormat swapchain_format)
{
	VkAttachmentDescription color_attachment = {};
	color_attachment.flags = 0;
	color_attachment.format = swapchain_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.pNext = NULL;
	render_pass_info.flags = 0;
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;

	VkRenderPass render_pass;
	VkResult result = vkCreateRenderPass(device, &render_pass_info, NULL, &render_pass);
	if (result != VK_SUCCESS) {
		printf("Could not create render pass.\n");
		return NULL;
	}

	return render_pass;
}

VkPipeline Create_GraphicsPipeline(VkDevice device, VkExtent2D swapchain_extent, VkRenderPass render_pass, VkPipelineLayout *pipeline_layout)
{
	char *vert_shader_code;
	char *frag_shader_code;
	int vert_shader_code_length = Read_File("vert.spv", &vert_shader_code);
	int frag_shader_code_length = Read_File("frag.spv", &frag_shader_code);

	VkShaderModule vert_shader_module = Create_Shader_Module(device, vert_shader_code, vert_shader_code_length);
	VkShaderModule frag_shader_module = Create_Shader_Module(device, frag_shader_code, frag_shader_code_length);

	VkPipelineShaderStageCreateInfo vert_shader_stage_info = {};
	vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_shader_stage_info.pNext = NULL;
	vert_shader_stage_info.flags = 0;
	vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_shader_stage_info.module = vert_shader_module;
	vert_shader_stage_info.pName = "main";
	vert_shader_stage_info.pSpecializationInfo = NULL;

	VkPipelineShaderStageCreateInfo frag_shader_stage_info = {};
	frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_shader_stage_info.pNext = NULL;
	frag_shader_stage_info.flags = 0;
	frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_shader_stage_info.module = frag_shader_module;
	frag_shader_stage_info.pName = "main";
	frag_shader_stage_info.pSpecializationInfo = NULL;

	VkPipelineShaderStageCreateInfo shader_stages[] = { vert_shader_stage_info, frag_shader_stage_info };

	VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.pNext = NULL;
	vertex_input_info.flags = 0;
	vertex_input_info.vertexBindingDescriptionCount = 0;
	vertex_input_info.vertexAttributeDescriptionCount = 0;

	VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {};
	input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_info.pNext = NULL;
	input_assembly_info.flags = 0;
	input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly_info.primitiveRestartEnable = VK_FALSE;
	
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0;
	viewport.width = (float) swapchain_extent.width;
	viewport.height = (float) swapchain_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset.x = scissor.offset.y = 0;
	scissor.extent = swapchain_extent;
	
	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.pNext = NULL;
	viewport_state.flags = 0;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &viewport;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.pNext = NULL;
	rasterizer.flags = 0;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.pNext = NULL;
	multisampling.flags = 0;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState color_blend_attachment = {};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;
	
	VkPipelineColorBlendStateCreateInfo color_blending = {};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.pNext = NULL;
	color_blending.flags = 0;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;
	color_blending.blendConstants[0] = 0.0f;
	color_blending.blendConstants[1] = 0.0f;
	color_blending.blendConstants[2] = 0.0f;
	color_blending.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.pNext = NULL;
	pipeline_layout_info.flags = 0;
	pipeline_layout_info.setLayoutCount = 0;
	pipeline_layout_info.pushConstantRangeCount = 0;

	VkResult result = vkCreatePipelineLayout(device, &pipeline_layout_info, NULL, pipeline_layout);

	if (result != VK_SUCCESS) {
		printf("Failed to create pipeline layout.\n");
	}

	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly_info;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.layout = *pipeline_layout;
	pipeline_info.renderPass = render_pass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

	VkPipeline graphics_pipeline;
	result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &graphics_pipeline);

	free(vert_shader_code);
	free(frag_shader_code);
	vkDestroyShaderModule(device, vert_shader_module, NULL);
	vkDestroyShaderModule(device, frag_shader_module, NULL);

	if (result != VK_SUCCESS) {
		printf("Failed to create graphics pipeline.\n");
		return NULL;
	}

	return graphics_pipeline;	
}

int Read_File(const char *file_name, char **file_content)
{
	FILE *file_ptr;

	file_ptr = fopen(file_name, "rb");

	fseek(file_ptr, 0, SEEK_END);
	int length = ftell(file_ptr);

	*file_content = malloc((length) * sizeof(char));
	fseek(file_ptr, 0, SEEK_SET);
	fread(*file_content, length, sizeof(char), file_ptr);

	fclose(file_ptr);

	return length;
}

VkShaderModule Create_Shader_Module(VkDevice device, char *shader_code, int shader_code_length)
{
	VkShaderModuleCreateInfo shader_module_create_info = {};
	shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_create_info.pNext = NULL;
	shader_module_create_info.flags = 0;
	
	shader_module_create_info.codeSize = shader_code_length;
	shader_module_create_info.pCode = (const uint32_t *) shader_code;

	VkShaderModule shader_module;
	VkResult result = vkCreateShaderModule(device, &shader_module_create_info, NULL, &shader_module);

	if (result != VK_SUCCESS) {
		printf("Could not create shader module.\n");
		return NULL;
	}

	return shader_module;
}

int Create_Framebuffer(VkDevice device, VkExtent2D swapchain_extent, VkImageView *swapchain_image_views, uint32_t swapchain_image_view_count, VkRenderPass render_pass, VkFramebuffer **swapchain_framebuffer)
{
	int swapchain_framebuffer_count = swapchain_image_view_count;
	*swapchain_framebuffer = malloc(swapchain_framebuffer_count * sizeof(VkFramebuffer));

	for (int i = 0; i < swapchain_framebuffer_count; ++i) {
		VkImageView attachments[] = { swapchain_image_views[i] };
	
		VkFramebufferCreateInfo framebuffer_info = {};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = render_pass;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = attachments;
		framebuffer_info.width = swapchain_extent.width;
		framebuffer_info.height = swapchain_extent.height;
		framebuffer_info.layers = 1;

		VkResult result = vkCreateFramebuffer(device, &framebuffer_info, NULL, &((*swapchain_framebuffer)[i]));
		if (result != VK_SUCCESS) {
			printf("Could not create frame buffer.\n");
		}
	}

	return swapchain_framebuffer_count;
}

VkCommandPool Create_Command_Pool(VkDevice device, int graphics_queue_family_index)
{
	VkCommandPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.pNext = NULL;
	pool_info.queueFamilyIndex = (uint32_t) graphics_queue_family_index;
	pool_info.flags = 0;

	VkCommandPool command_pool;
	VkResult result = vkCreateCommandPool(device, &pool_info, NULL, &command_pool);
	if (result != VK_SUCCESS) {
		printf("Could not create command pool.\n");
		return NULL;
	}

	return command_pool;
}

int Create_Command_Buffers(VkDevice device, VkExtent2D swapchain_extent, VkRenderPass render_pass, VkPipeline graphics_pipeline, int swapchain_framebuffer_count, VkFramebuffer *swapchain_framebuffer, VkCommandPool command_pool, VkCommandBuffer **command_buffers)
{
	int command_buffer_count = swapchain_framebuffer_count;
	*command_buffers = malloc(command_buffer_count * sizeof(VkCommandBuffer));

	VkCommandBufferAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocate_info.pNext = NULL;
	allocate_info.commandPool = command_pool;
	allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocate_info.commandBufferCount = (uint32_t) command_buffer_count;

	VkResult result = vkAllocateCommandBuffers(device, &allocate_info, *command_buffers);
	if (result != VK_SUCCESS) {
		printf("Could not create command buffers.\n");
	}

	for (int i = 0; i < command_buffer_count; ++i) {
		VkCommandBufferBeginInfo command_buffer_begin_info  = {};
		command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		vkBeginCommandBuffer((*command_buffers)[i], &command_buffer_begin_info);

		VkRenderPassBeginInfo render_pass_begin_info = {};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.pNext = NULL;
		render_pass_begin_info.renderPass = render_pass;
		render_pass_begin_info.framebuffer = swapchain_framebuffer[i];
		render_pass_begin_info.renderArea.offset.x =  0;
		render_pass_begin_info.renderArea.offset.y =  0;
		render_pass_begin_info.renderArea.extent = swapchain_extent;

		VkClearValue clear_color;
		clear_color.color.float32[0] =  0.0f;
		clear_color.color.float32[1] =  0.0f;
		clear_color.color.float32[2] =  0.0f;
		clear_color.color.float32[3] =  1.0f;
		render_pass_begin_info.clearValueCount = 1;
		render_pass_begin_info.pClearValues = &clear_color;

		vkCmdBeginRenderPass((*command_buffers)[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE); 
		{
			vkCmdBindPipeline((*command_buffers)[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);
			vkCmdDraw((*command_buffers)[i], 3, 1, 0, 0);
		}
		vkCmdEndRenderPass((*command_buffers)[i]);

		VkResult result = vkEndCommandBuffer((*command_buffers)[i]);
		if (result != VK_SUCCESS) {
			printf("Couldn't record command buffer.\n");
		}
	}
	
	return command_buffer_count;
}

VkSemaphore Create_Semaphore(VkDevice device)
{
	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkSemaphore semaphore;
	VkResult result = vkCreateSemaphore(device, &semaphore_info, NULL, &semaphore);
	if(result != VK_SUCCESS) {
		printf("Could not create semaphore.\n");
		return NULL;
	}

	return semaphore;
}

void Render_Frame(VkDevice device, VkQueue graphics_queue, VkSwapchainKHR swapchain, VkCommandBuffer *command_buffers, VkSemaphore image_available_semaphore, VkSemaphore render_finished_semaphore)
{
	uint32_t image_index;
	vkAcquireNextImageKHR(device, swapchain, UINT_MAX, image_available_semaphore, VK_NULL_HANDLE, &image_index);

	VkSubmitInfo submit_info  = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore wait_semaphores[] = { image_available_semaphore };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffers[image_index];

	VkSemaphore signal_semaphores[] = { render_finished_semaphore };
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;

	VkResult result = vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	if (result != VK_SUCCESS) {
		printf("Failed to submit draw command buffer.\n");
	}

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;
	
	VkSwapchainKHR swapchains[] = { swapchain };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapchains;
	present_info.pImageIndices = &image_index;
	present_info.pResults = NULL;

	vkQueuePresentKHR(graphics_queue, &present_info);

	vkQueueWaitIdle(graphics_queue);
}
