#pragma once
namespace Nimbus {
	template<typename T>
	class Singleton {
	protected:
		inline static T* ptr = nullptr;

	protected:
		Singleton() = default;
		~Singleton();

	public:
		Singleton(const Singleton&) = delete;
		Singleton& operator=(Singleton) = delete;
		static T* getInstance();
	};

	// Public methods

	template <typename T>
	Singleton<T>::~Singleton() {
		delete ptr;
		ptr = nullptr;
	}

	template<typename T>
	T* Singleton<T>::getInstance() {
		if (!ptr) {
			ptr = new T();
		}

		return ptr;
	}
}
