#pragma once

template <typename T>
class IInterface
{
public:
	IInterface(T* Interface) : Interface(Interface)
	{
	}

	virtual ~IInterface() = default;

	template <typename E, typename = std::enable_if_t<std::is_base_of_v<T, E>>>
	[[nodiscard]] E* As() const
	{
		return static_cast<E*>(Interface);
	}

private:
	T* Interface;
};
