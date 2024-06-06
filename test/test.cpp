
enum struct MyCustomPriorities {
	IMMEDIATE,
	HIGH,
	NORMAL,
	LOW,
	LAST
};
#define NES_PRIORITY_TYPE MyCustomPriorities
#include <nes/event_dispatcher.hpp>

#include <iostream>

struct MyEvent {
	bool mCancel = false;
};

struct SomeClass {
	explicit SomeClass(nes::event_dispatcher& dispatcher) : mDispatcher{dispatcher} {
		dispatcher.listen<MyEvent, &SomeClass::onMyFirstEvent, nes::event_priority::IMMEDIATE>(this);
		dispatcher.listen<MyEvent, &SomeClass::onMyNormalEvent>(this);
		dispatcher.listen<MyEvent, &SomeClass::onMyLastEvent, nes::event_priority::LOW>(this);
	}
	~SomeClass() {
		mDispatcher.deafen<MyEvent, &SomeClass::onMyFirstEvent>(this);
		mDispatcher.deafen<MyEvent, &SomeClass::onMyNormalEvent>(this);
		mDispatcher.deafen<MyEvent, &SomeClass::onMyLastEvent>(this);
	}
	void onMyFirstEvent(MyEvent& event) {
		std::cout << "onMyFirstEvent was called!" << std::endl;
	}
	void onMyNormalEvent(MyEvent& event) {
		std::cout << "onMyNormalEvent was called!" << std::endl;
	}
	void onMyLastEvent(MyEvent& event) {
		std::cout << "onMyLastEvent was called!" << std::endl;
	}
private:
	nes::event_dispatcher& mDispatcher;
};

int main() {
	nes::event_dispatcher dispatcher;
	SomeClass instance { dispatcher };

	auto event = nes::make_holder<MyEvent>();
	dispatcher.trigger(event);

	if (event->mCancel) {
		std::cout << "Event cancelled!" << std::endl;
	} else {
		std::cout << "Event not cancelled!" << std::endl;
	}
}