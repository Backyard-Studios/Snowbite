#pragma once

class IGraphicsInterface
{
public:
	virtual ~IGraphicsInterface() = default;

private:
	[[nodiscard]] virtual HRESULT Initialize() = 0;
	virtual void Destroy() = 0;

private:
	friend class FRenderer;
};
