#include "Logger.h"
#include "ThreadEvents.h"
#include "ThreadRender.h"
#include "Font.h"
#include "GL.h"
#include "StringBox.h"

#include <thread>
#include <iostream>
#include <chrono>
#include <atomic>

static StringBox* listTest = nullptr;

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
	
	events::ThreadEvents::addInlineKeyEventCallback(
		[](int keyCode, events::KeyAction action, events::KeyModFlags flags) {
			listTest->pos += vec2(0, 0.1);
		}, events::KeyCode::s, events::KeyAction::repeat);

	events::ThreadEvents::addInlineKeyEventCallback(
		[](int keyCode, events::KeyAction action, events::KeyModFlags flags) {
			listTest->pos += vec2(0, -0.1);
		}, events::KeyCode::w, events::KeyAction::repeat);

	auto rHandle = render::ThreadRender::newRenderHandle([]() {
		static StringBox* sb = nullptr;
		constexpr float HALF_SIZE_X = 0.5;
		constexpr float HALF_SIZE_Y = 0.5;
		vec2 v = gl::target->normalizePos(vec2(events::ThreadEvents::getMousePos()));
		if (sb==nullptr) {
			sb = new StringBox();
			sb->pos = vec2(v.x-HALF_SIZE_X, -v.y-HALF_SIZE_Y);
			sb->setSize(vec2(HALF_SIZE_X*2, HALF_SIZE_Y*2)/2.f);
			sb->setTextSize(48.f);
			sb->str("_~_:{}[]\\/01(2)22\n345\n*6789\n@aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ");
		} else {
			sb->pos = vec2(v.x-HALF_SIZE_X, -v.y-HALF_SIZE_Y);
		}
		if (events::ThreadEvents::isWindowResized() || events::ThreadEvents::isWindowMoved()) {
			sb->notifyPPIChanged();
		}
		sb->render();

		if (listTest==nullptr) {
			listTest = new StringBox();
			listTest->pos = vec2(-1, -1);
			listTest->setSize(vec2(1, 1));
			listTest->setTextSize(12.f);
			listTest->clear();
			for (uint i = 0; i<128; i++) {
				*listTest << i << ": Hello~ How are you doing?\n";
			}
		}
		if (events::ThreadEvents::isWindowResized() || events::ThreadEvents::isWindowMoved()) {
			listTest->notifyPPIChanged();
		}
		listTest->render();

		});
	logger("Waiting for core thread end...\n");
	events::ThreadEvents::join();
	logger("core thread ended.\n");
	logger("Exiting...\n");
	logger.closeThread("MainThread");
	system("pause");
	return 0;
}