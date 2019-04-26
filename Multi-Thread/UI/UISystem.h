#pragma once
#include "UISystemInterface.h"
#include <memory>
#include "MainFrame.h"
#include "UIRectBatchRender.h"
#include <string>

class UISystem : public IUISystemInterface
{
public:
	UISystem();
	virtual ~UISystem();

	bool Init();
	virtual IUIMainFrame* GetMainFrame() override;

	static UISystem* GetInstance();

	void OnRender(ID3D11DeviceContext* InD3dContext);
	void OnUpdate(float InDeltaTime);
	void OnPostRender();

	void DrawInCPU(ID3DX11Effect* InEffect, const std::string& InTechName, const std::string& InPassName, const UIRectBatchRender::RectRenderElementInCPU& InElement);


private:
	std::shared_ptr<MainFrame> _MainFrame;
	std::shared_ptr<UIRectBatchRender> _UIRectBatchRender;

	static UISystem _sInst;
};
