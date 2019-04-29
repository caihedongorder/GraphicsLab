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

	void OnRender();
	void OnUpdate(float InDeltaTime);
	void OnPostRender();

	void BeginFrame();
	void EndFrame();

	UIRectBatchRender* GetUIRectBatchRender() { return _UIRectBatchRender.get(); }

private:
	std::shared_ptr<MainFrame> _MainFrame;
	std::shared_ptr<UIRectBatchRender> _UIRectBatchRender;
};
