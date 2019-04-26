#include "stdafx.h"
#include "WidgetPanel.h"
#include "../JobSystem.h"

void WidgetPanel::AddControl(std::shared_ptr<WidgetControlBase> InControl)
{
	_Controls.push_back(InControl);
}

bool WidgetPanel::OnInit()
{
	return true;
}

void WidgetPanel::OnRender(class UIRectBatchRender* InUIRender)
{
	std::shared_ptr<WidgetControlBase>* pWidgetBase = &_Controls[0];
	int Nums = _Controls.size();
	if (false&&Nums > 100)
	{
		auto pJob = JobSystem::createParallelForJob(pWidgetBase, Nums, nullptr, [InUIRender](std::shared_ptr<WidgetControlBase>* pWidgetBase, int Count, void * UserData) {
			for (int i = 0; i < Count; ++i)
			{
				pWidgetBase[i]->OnRender(InUIRender);
			}
		}, 100, nullptr, [](void* pWidgetBase) {});
		JobSystem::waitForJob(pJob);
	}
	else
	{
		for (auto It = _Controls.begin(); It != _Controls.end(); ++It)
		{
			(*It)->OnRender(InUIRender);
		}
	}
}

void WidgetPanel::OnUpdate(float InDeltaTime)
{
	std::shared_ptr<WidgetControlBase>* pWidgetBase = &_Controls[0];
	int Nums = _Controls.size();

	if ( Nums > 100)
	{
		auto pJob = JobSystem::createParallelForJob(pWidgetBase, Nums, nullptr, [InDeltaTime](std::shared_ptr<WidgetControlBase>* pWidgetBase, int Count, void * UserData) {
			for (int i = 0; i < Count; ++i)
			{
				pWidgetBase[i]->OnUpdate(InDeltaTime);
			}
		}, 100, nullptr, [](void* pWidgetBase) {});
		JobSystem::waitForJob(pJob);
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
	for (auto It = _Controls.begin(); It != _Controls.end(); ++It)
	{
		(*It)->OnPostRender();
	}
}
