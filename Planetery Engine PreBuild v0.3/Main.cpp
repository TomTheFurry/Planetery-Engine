#include "Logger.h"
#include "ThreadEvents.h"
#include "ThreadRender.h"
#include "Font.h"
#include "GL.h"

#include <thread>
#include <iostream>
#include <chrono>
#include <atomic>




int main() {
	logger.newThread("MainThread");

	logger("Starting core event thread...\n");
	events::ThreadEvents::start();
	logger("core thread started.\n");
	uint fullScreenMode = 0;
	events::ThreadEvents::addInlineKeyEventCallback(
		[](int keyCode, events::KeyAction action, events::KeyModFlags flags) {
			events::ThreadEvents::closeWindow();
		}, events::KeyCode::escape, events::KeyAction::press);

	events::ThreadEvents::addInlineKeyEventCallback(
		[](int keyCode, events::KeyAction action, events::KeyModFlags flags) {
			if (events::ThreadEvents::getFullScreenModeInline()!=events::FullScreenMode::windowed) {
				events::ThreadEvents::setWindowedInline();
			}
			else {
				events::ThreadEvents::setFullScreenInline();
			}
		}, events::KeyCode::F11, events::KeyAction::press, events::KeyModFlags(events::KeyModFlag::matchShift | events::KeyModFlag::shift));

	events::ThreadEvents::addInlineKeyEventCallback(
		[](int keyCode, events::KeyAction action, events::KeyModFlags flags) {
			if (events::ThreadEvents::getFullScreenModeInline()!=events::FullScreenMode::windowedFullscreen) {
				events::ThreadEvents::setWindowedFullScreenInline();
			} else {
				events::ThreadEvents::setWindowedInline();
			}
		}, events::KeyCode::F11, events::KeyAction::press, events::KeyModFlags(events::KeyModFlag::matchShift));
	auto rHandle = render::ThreadRender::newRenderHandle([]() {
		vec2 v = gl::target->normalize(vec2(events::ThreadEvents::getMousePos()));
		FontManager::renderString("MousePointer", vec2(v.x,-v.y),12.f);
		});
	logger("Waiting for core thread end...\n");
	events::ThreadEvents::join();
	logger("core thread ended.\n");
	logger("Exiting...\n");
	logger.closeThread("MainThread");
	system("pause");
	return 0;
}