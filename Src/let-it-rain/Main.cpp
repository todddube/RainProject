#include "RainWindow.h"

// Callback function to be called for each display
BOOL CALLBACK MonitorEnumProc(const HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, const LPARAM lParam) {
	std::vector<MonitorData>* monitors = reinterpret_cast<std::vector<MonitorData>*>(lParam);

	MONITORINFOEX monitorInfo;
	monitorInfo.cbSize = sizeof(monitorInfo);
	if (GetMonitorInfo(hMonitor, &monitorInfo)) {
		MonitorData info;
		info.DisplayRect = monitorInfo.rcMonitor;
		info.Name = monitorInfo.szDevice;
		info.IsDefaultDisplay = (monitorInfo.dwFlags & MONITORINFOF_PRIMARY) != 0;
		monitors->push_back(info);
	}
	return TRUE;
}

//
// Provides the entry point to the application.
//
int WINAPI WinMain(
	const HINSTANCE hInstance,
	HINSTANCE /*hPrevInstance*/,
	LPSTR /*lpCmdLine*/,
	int /*nCmdShow*/
)
{
	// Ignore the return value because we want to continue running even in the
	// unlikely event that HeapSetInformation fails.
	HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);

	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	std::vector<MonitorData> monitorInfoList;	
	EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitorInfoList));	

	if (SUCCEEDED(CoInitialize(NULL)))
	{
		std::vector<RainWindow*> rainWindows;			
		for (const auto& monitorInfo : monitorInfoList) {
			RainWindow* rainWindow = new RainWindow();
			if (SUCCEEDED(rainWindow->Initialize(hInstance, monitorInfo)))
			{
				rainWindows.push_back(rainWindow);
			}
		}

		if (!rainWindows.empty())
		{
			MSG msg = {};
			while (msg.message != WM_QUIT)
			{
				if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
				{
					for (RainWindow* rainWindow : rainWindows) {
						rainWindow->Animate();
					}
					Sleep(10);
				}
			}
			for (const RainWindow* rainWindow : rainWindows) {
				delete rainWindow;
			}
		}
		CoUninitialize();
	}
	return 0;
}
