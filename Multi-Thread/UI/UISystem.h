#pragma once
#include "UISystemInterface.h"
#include <memory>
#include "MainFrame.h"

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

private:
	std::shared_ptr<MainFrame> _MainFrame;

	static UISystem _sInst;
};
