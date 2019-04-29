#include "stdafx.h"
#include "WidgetPanel.h"
#include "../JobSystem.h"
#include "../GraphicsLabSystem.h"

static int SpliteNums = 100;

void WidgetPanel::AddControl(std::shared_ptr<WidgetControlBase> InControl)
{
	_Controls.push_back(InControl);
}

bool WidgetPanel::OnInit()
{
	return true;
}

void WidgetPanel::OnUpdate(float InDeltaTime)
{
	std::shared_ptr<WidgetControlBase>* pWidgetBase = &_Controls[0];
	int Nums = _Controls.size();

	if ( Nums > SpliteNums)
	{
		auto pJob = JobSystem::createParallelForJob(pWidgetBase, Nums, nullptr, [InDeltaTime](std::shared_ptr<WidgetControlBase>* pWidgetBase, int Count, void * UserData) {
			for (int i = 0; i < Count; ++i)
			{
				pWidgetBase[i]->OnUpdate(InDeltaTime);
			}
		}, SpliteNums, GraphicsLabSystem::GetInstance()->GetFrameJob(), [](void* pWidgetBase) {},0);
		//JobSystem::waitForJob(pJob);

	}
	else
	{
		for (auto It = _Controls.begin(); It != _Controls.end(); ++It)
		{
			(*It)->OnUpdate(InDeltaTime);
		}
	}
}

void WidgetPanel::OnPostRender()
{
	std::shared_ptr<WidgetControlBase>* pWidgetBase = &_Controls[0];
	int Nums = _Controls.size();

	if ( Nums > SpliteNums)
	{
		auto pJob = JobSystem::createParallelForJob(pWidgetBase, Nums, nullptr, [](std::shared_ptr<WidgetControlBase>* pWidgetBase, int Count, void * UserData) {
			for (int i = 0; i < Count; ++i)
			{
				pWidgetBase[i]->OnPostRender();
			}
		}, SpliteNums, GraphicsLabSystem::GetInstance()->GetFrameJob(), [](void* pWidgetBase) {},1);
		//JobSystem::waitForJob(pJob);
	}
	else
	{
		for ( int i = 0; i < Nums; ++i )
		{
			_Controls[i]->OnPostRender();
		}
	}
}
