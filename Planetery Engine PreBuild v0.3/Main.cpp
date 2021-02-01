#include "Logger.h"
#include "ThreadEvents.h"
#include "ThreadRender.h"
#include "Font2.h"
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
		vec2 v = gl::target->normalizePos(vec2(events::ThreadEvents::getMousePos()));
		font::drawString("~_:{}[]\\/0123456789aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ", 24.f, vec2(v.x,-v.y));
		});
	logger("Waiting for core thread end...\n");
	events::ThreadEvents::join();
	logger("core thread ended.\n");
	logger("Exiting...\n");
	logger.closeThread("MainThread");
	system("pause");
	return 0;
}