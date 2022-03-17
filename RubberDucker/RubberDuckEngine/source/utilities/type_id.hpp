#pragma once

namespace RDE {

	template<typename OwnerClass>
	class TypeID
	{
		// Initialize counter in owner class CPP!
		static uint32_t s_counter;
	public:
		template<typename T>
		static void registerType()
		{
			removedConstRef<std::decay_t<T>>();
		}

		template<typename T>
		static uint32_t getID()
		{
			return removedConstRef<std::decay_t<T>>();
		}

		static uint32_t size()
		{
			return s_counter;
		}

	private:
		template<typename T>
		static uint32_t removedConstRef()
		{
			static uint32_t ID = s_counter++;
			return ID;
		}
	};
}