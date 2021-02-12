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

constexpr const char* UTF_8_TEST_STRING = R"V0G0N(English: The quick brown fox jumps over the lazy dog.
Jamaican: Chruu, a kwik di kwik brong fox a jomp huova di liezi daag de, yu no siit ?
Irish : "An ḃfuil do ċroí ag bualaḋ ó ḟaitíos an ġrá a ṁeall lena ṗóg éada ó ṡlí do leasa ṫú?" "D'ḟuascail Íosa Úrṁac na hÓiġe Beannaiṫe pór Éava agus Áḋaiṁ."
Dutch : Pa's wĳze lynx bezag vroom het fikse aquaduct.
German : Falsches Üben von Xylophonmusik quält jeden größeren Zwerg. (1)
German : Im finſteren Jagdſchloß am offenen Felsquellwaſſer patzte der affig-flatterhafte kauzig-höf‌liche Bäcker über ſeinem verſifften kniffligen C-Xylophon. (2)
Norwegian : Blåbærsyltetøy("blueberry jam", includes every extra letter used in Norwegian).
Swedish : Flygande bäckasiner söka strax hwila på mjuka tuvor.
Icelandic : Sævör grét áðan því úlpan var ónýt.
Finnish : (5) Törkylempijävongahdus(This is a perfect pangram, every letter appears only once.Translating it is an art on its own, but I'll say "rude lover's yelp". :-D)
	Finnish: (5) Albert osti fagotin ja töräytti puhkuvan melodian. (Albert bought a bassoon and hooted an impressive melody.)
	Finnish : (5) On sangen hauskaa, että polkupyörä on maanteiden jokapäiväinen ilmiö. (It's pleasantly amusing, that the bicycle is an everyday sight on the roads.)
		Polish: Pchnąć w tę łódź jeża lub osiem skrzyń fig.
		Czech : Příliš žluťoučký kůň úpěl ďábelské ódy.
		Slovak : Starý kôň na hŕbe kníh žuje tíško povädnuté ruže, na stĺpe sa ďateľ učí kvákať novú ódu o živote.
		Slovenian : Šerif bo za domačo vajo spet kuhal žgance.
		Greek(monotonic) : ξεσκεπάζω την ψυχοφθόρα βδελυγμία
		Greek(polytonic) : ξεσκεπάζω τὴν ψυχοφθόρα βδελυγμία
		Russian : Съешь же ещё этих мягких французских булок да выпей чаю.
		Russian : В чащах юга жил-был цитрус ? Да, но фальшивый экземпляр!ёъ.
		Bulgarian : Жълтата дюля беше щастлива, че пухът, който цъфна, замръзна като гьон.
		Sami(Northern) : Vuol Ruoŧa geđggiid leat máŋga luosa ja čuovžža.
		Hungarian : Árvíztűrő tükörfúrógép.
		Spanish : El pingüino Wenceslao hizo kilómetros bajo exhaustiva lluvia y frío, añoraba a su querido cachorro.
		Spanish : Volé cigüeña que jamás cruzó París, exhibe flor de kiwi y atún.
		Portuguese : O próximo vôo à noite sobre o Atlântico, põe freqüentemente o único médico. (3)
		French : Les naïfs ægithales hâtifs pondant à Noël où il gèle sont sûrs d'être déçus en voyant leurs drôles d'œufs abîmés.
		Esperanto : Eĥoŝanĝo ĉiuĵaŭde
		Esperanto : Laŭ Ludoviko Zamenhof bongustas freŝa ĉeĥa manĝaĵo kun spicoj.
		Hebrew : זה כיף סתם לשמוע איך תנצח קרפד עץ טוב בגן.
		Japanese(Hiragana) :
		いろはにほへど　ちりぬるを
		わがよたれぞ　つねならむ
		うゐのおくやま　けふこえて
		あさきゆめみじ　ゑひもせず(4))V0G0N";



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
			*listTest << UTF_8_TEST_STRING;
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