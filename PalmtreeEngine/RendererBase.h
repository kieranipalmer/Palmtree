#pragma once
class RendererBase
{
public:
	virtual ~RendererBase();
	virtual void Init(void* data) = 0;
	virtual void BeginFrame() = 0;
	virtual void EndFrame() = 0;
	virtual void DrawSprite() = 0;
	virtual void SetProjectionMatrix() = 0;
	virtual void SetViewMatrix() = 0;
	virtual void GetProjectionMatrix()=0;
	virtual void GetViewMatrix()=0;
};

