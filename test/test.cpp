/*
enum struct MyCustomPriorities {
	Immediate,
	High,
	Regular,
	Low,
	Last
};
#define NES_PRIORITY_TYPE MyCustomPriorities
#define NES_PRIORITY_TRAITS template<> struct nes::event_priority_traits<NES_PRIORITY_TYPE> { using priority_type = NES_PRIORITY_TYPE; static constexpr priority_type default_value = priority_type::Regular; };*/
#include <nes/event_dispatcher.hpp>

#include <iostream>

struct MyEvent {
	bool mCancel = false;
};

static int object_test_count = 0;
struct SomeClass {
	explicit SomeClass(nes::event_dispatcher& dispatcher) : mDispatcher{dispatcher} {
		dispatcher.listen<MyEvent, &SomeClass::onMyFirstEvent, nes::event_priority::FIRST>(this);
		dispatcher.listen<MyEvent, &SomeClass::onMyNormalEvent>(this);
		dispatcher.listen<MyEvent, &SomeClass::onMyLastEvent, nes::event_priority::LAST>(this);
	}
	~SomeClass() {
		mDispatcher.deafen<MyEvent, &SomeClass::onMyFirstEvent>(this);
		mDispatcher.deafen<MyEvent, &SomeClass::onMyNormalEvent>(this);
		mDispatcher.deafen<MyEvent, &SomeClass::onMyLastEvent>(this);
	}
	void onMyFirstEvent(MyEvent& event) {
		std::cout << "onMyFirstEvent was called!" << std::endl;
		object_test_count++;
	}
	void onMyNormalEvent(MyEvent& event) {
		std::cout << "onMyNormalEvent was called!" << std::endl;
		object_test_count++;
	}
	void onMyLastEvent(MyEvent& event) {
		std::cout << "onMyLastEvent was called!" << std::endl;
		object_test_count++;
	}
private:
	nes::event_dispatcher& mDispatcher;
};

void object_test(nes::event_dispatcher& dispatcher) {
	SomeClass instanceA { dispatcher };
	{
		SomeClass instanceB { dispatcher };

		auto event = nes::make_holder<MyEvent>();
		dispatcher.trigger(event);

		if (event->mCancel) {
			std::cout << "Event cancelled!" << std::endl;
		} else {
			std::cout << "Event not cancelled!" << std::endl;
		}
	}

	auto event = nes::make_holder<MyEvent>();
	dispatcher.trigger(event);
}

static int scope_test_count = 0;
void scope_print(MyEvent& event) {
	std::cout << "scoped_listener was called!" << std::endl;
	scope_test_count++;
}
void scope_test(nes::event_dispatcher& dispatcher) {
	nes::scoped_listener<MyEvent, scope_print> listenerA{ dispatcher };
	{
		nes::scoped_listener<MyEvent, scope_print> listenerB{ dispatcher };
		auto event = nes::make_holder<MyEvent>();
		dispatcher.trigger(event);
	}
	auto event = nes::make_holder<MyEvent>();
	dispatcher.trigger(event);
}

static int scope_lambda_count = 0;
void scope_lamda(nes::event_dispatcher& dispatcher) {
	nes::scoped_listener<MyEvent, [](MyEvent& e) {
		std::cout << "scope_lambda was called!" << std::endl;
		scope_lambda_count++;
	}> listener{ dispatcher };
	auto event = nes::make_holder<MyEvent>();
	dispatcher.trigger(event);
}

int main() {
	nes::event_dispatcher dispatcher;
	object_test(dispatcher);
	std::cout << "Test completed!" << std::endl;
	auto event = nes::make_holder<MyEvent>();
	dispatcher.trigger(event);
	std::cout << "After test trigger" << std::endl;
	scope_test(dispatcher);
	std::cout << "Test completed!" << std::endl;
	dispatcher.trigger(event);
	std::cout << "After test trigger" << std::endl;
	scope_lamda(dispatcher);
	std::cout << "Test completed!" << std::endl;
	dispatcher.trigger(event);
	std::cout << "After test trigger" << std::endl;

	if (object_test_count != 9) {
		std::cerr << "Object Test FAILED! Triggered more or less than intended!" << std::endl;
	}
	if (scope_test_count != 3) {
		std::cerr << "Scope Test FAILED! Triggered more than intended!" << std::endl;
	}
	if (scope_lambda_count != 1) {
		std::cerr << "Scope Lambda FAILED! Triggered more than intended!" << std::endl;
	}
}