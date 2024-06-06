# NuvolaEventSystem
A single-header, thread-safe event system for modern C++.

## Usage
### Setup
Currently, this library relies on [EnTT](https://github.com/skypjack/entt). The event system uses EnTT for its compile-time type hashing, which helps avoid problems with identifying functions and tracking them for listening/deafening.

### Creating an `nes::event_dispatcher`
Typically you'd want just a single global `nes::event_dispatcher` instance. Having multiple is possible, but not currently supported. Due to how well this library works with concurrency, unless you absolutely cannot have a weak reference to the main event dispatcher, you'll probably just want to have the one instance. The dispatcher can be default-constructed like so:
```C++
nes::event_dispatcher dispatcher;
```

### Creating an event type
Event types are just structs. You can pass any kind of struct as an event. I recommend keeping the event structs small, and only containing members necessary for your use case.
```C++
struct MyEvent {
    bool mCancel = false;
};
```

### Creating an `nes::event_holder`
Event holders are responsible for 'holding' (and owning) the event instance that is going to be dispatched to the listeners. You can create an `nes::event_holder` like so:
```C++
auto event = nes::make_holder<MyEvent>();
```
If your event type requires parameters or you need to pass in data to the constructor of the event type, you can pass them in as function arguments to `nes::make_holder` in the same way you can with something like `std::make_unique`.

### Triggering events
Triggering (or dispatching) events is really simple, and can be done in a mere two lines of code. The first to create the `nes::event_holder` (as seen above), and the second to dispatch it.
```C++
auto event = nes::make_holder<MyEvent>();
dispatcher.trigger(event);
```
That's it! Nothing else needs to be done to invoke your event callbacks. Keep in mind, however, that **this is a blocking operation**. It will go through and execute all of the callbacks on the current thread. This allows you to check the state of the `event` object for things like event cancellation, for example. Even though this is technically blocking, if a second thread triggers the events, **it will not wait for the first thread to finish executing callbacks**. This means that, in this case, callbacks will be executed on **both (or more) threads at the same time**, so make sure your code can account for that!

### Listening for events
There are many ways to listen for events. All of which require the event type to be passed in *as a reference*, NOT a const reference, or a copy. Doing so is unsupported. Some approaches have more benefits over others, and you'll need to pick the way you do it as you need. Each example is in order of which I think are most common/useful first, and then the ways I think are least common/useful are last.

#### Listening for events with a class/struct member function
This is probably the best way to listen for events if you're using an object-oriented pattern in your code. You can give the class the instance of the `nes::event_dispatcher` however you'd like, but for this example I'm just passing it down in the constructor.
```C++
struct SomeClass {
	explicit SomeClass(nes::event_dispatcher& dispatcher) : mDispatcher{dispatcher} {
		dispatcher.listen<MyEvent, &SomeClass::onMyEvent>(this);
	}
	~SomeClass() {
		mDispatcher.deafen<MyEvent, &SomeClass::onMyEvent>(this);
	}
	void onMyEvent(MyEvent& event) {
		std::cout << "onMyEvent was called!" << std::endl;
		event.mCancel = true;
	}
private:
	nes::event_dispatcher& mDispatcher;
};
```
The `nes::event_dispatcher::listen` function takes in the event type to listen for as the first template argument, then the calback member function as the second. `this` gets passed in as the object instance to use in event callbacks. Because of this, you **must also deafen your events in the deconstructor** of the class. If you don't, and the event gets triggered, the function will be called with an invalid instance of the class.

#### Listening for events with a lambda object
This is a decent way to listen for events, but from here on, you won't be able to create some kind of destructor that automatically deafens your events. You'll have to **manually deafen your event listener when you don't need it**.
```C++
auto callback = [](MyEvent& event) {
    std::cout << "Lambda callback!" << std::endl;
};
dispatcher.listen<MyEvent>(callback);

//TODO: Trigger event

dispatcher.deafen<MyEvent>(callback);
```

#### Listening for events with lambda directly
This is probably the most convenient way to listen for an event, but this gives ownership of the lambda directly to the `nes::event_dispatcher` and **you won't be able to deafen the callback**. This could lead to some leaking issues if you're not careful!
```C++
dispatcher.listen<MyEvent>([](MyEvent& event) {
    std::cout << "Lambda callback!" << std::endl;
});
```

### Event Priorities
By default, this library defines an `nes::event_priotity` enum. The values are `FIRST`, `NORMAL` and `LAST` by default. When an event is triggered, callbacks will be invoked in the order of their priorities first, then in a non-deterministic order after. You can specify the priority of the callback when listening for events by passing it in as the last template argument.
```C++
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
```
By default, the priority is set to `nes::event_priority::NORMAL`. If you need more event priorities, you can define the `NES_PRIORITY_TYPE` with your own priority enum. It is expected that the `NORMAL` priority is available, so you'll probably want to define it in your own priority enum. Additionally, ensure to define the macro *before* including the header. Keep in mind that the smaller the actual value of the enum is, the higher of a priority it has. This means the enum with the value of 0 is triggered first.
```C++
enum struct MyCustomPriorities {
	IMMEDIATE,
	HIGH,
	NORMAL,
	LOW,
	LAST
};
#define NES_PRIORITY_TYPE MyCustomPriorities
// Include the library *after* defining the macro
#include <nes/event_dispatcher.hpp>
``` 
If you really need to change the default value, and your enum cannot contain a `NORMAL` value (ie. its casing doesn't match your project's code styling rules), you can override this requirement by specializing the `nes::event_priority_traits` template within the `NES_PRIORITY_TRAITS` macro.
```C++
enum struct MyCustomPriorities {
	Immediate,
	High,
	Regular,
	Low,
	Last
};
#define NES_PRIORITY_TYPE MyCustomPriorities

// Specialize the traits template
#define NES_PRIORITY_TRAITS template<> struct nes::event_priority_traits<NES_PRIORITY_TYPE> { using priority_type = NES_PRIORITY_TYPE; static constexpr priority_type default_value = priority_type::Regular; };

// Include the library *after* defining both of the macros
#include <nes/event_dispatcher.hpp>
```

## About
### Why
Originally, I created this for my project, *Nuvola*. It was a Minecraft: Bedrock Edition utility mod I created for fun. Due to how Minecraft is designed and how it is constantly evolving, I wanted to be able to write my code with a more abstract 'event' system than writing my code in hook callbacks directly. This way, if the function got changed, removed, or replaced with another, all I had to do was move the event trigger to the new function instead.

Now, this library has been extremely useful to me, and I have used it in all sorts of other projects that aren't just Nuvola or Minecraft related. I believe it can be quite a versatile library, and could bring value to many others. So, I decided to publish the code under a permissive open-source license.

**Please consider leaving a star!** `:)`

### FAQ
#### Doesn't EnTT have an event system?
Yes, but it's not exactly the most suitable for cases where you're going to have callbacks be invoked immediately on many different threads. If you need thread-safety, concurrency, or immediate callbacks for things like making events 'cancellable', this library can do that in a much more effective way.