#include "includes.h"

#ifdef _WIN64
#define GWL_WNDPROC GWLP_WNDPROC
#endif

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

EndScene oEndScene = NULL;
WNDPROC oWndProc;
static HWND window = NULL;

int ScrX = GetSystemMetrics(SM_CXSCREEN);
int ScrY = GetSystemMetrics(SM_CYSCREEN);

bool WorldToScreen(Vec3 pos, Vec2& screen, float matrix[16], int w, int h) {
	Vec4 clipCords;
	clipCords.x = pos.x * matrix[0] + pos.y * matrix[1] + pos.z * matrix[2] + matrix[3];
	clipCords.y = pos.x * matrix[4] + pos.y * matrix[5] + pos.z * matrix[6] + matrix[7];
	clipCords.z = pos.x * matrix[8] + pos.y * matrix[9] + pos.z * matrix[10] + matrix[11];
	clipCords.w = pos.x * matrix[12] + pos.y * matrix[13] + pos.z * matrix[14] + matrix[15];

	if (clipCords.w < 0.1f) {
		return false;
	};

	Vec3 NDC;
	NDC.x = clipCords.x / clipCords.w;
	NDC.y = clipCords.y / clipCords.w;
	NDC.z = clipCords.z / clipCords.w;
	
	screen.x = (w / 2 * NDC.x) + (NDC.x + w / 2);
	screen.y = (h / 2 * NDC.y) + (NDC.y + h / 2);

	return true;
};

Vec2 GetBonePosition(uintptr_t, Entity, int bone) {
	uintptr_t BoneMatrix_base = *(uintptr_t*)(Entity + m_dwBoneMatrix);
	BoneMatrix_t Bone = *(BoneMatrix_t*)(BoneMatrix_Base + sizeof(Bone) * bone);
	Vec3 Location = (Bone.x, Bone.y, Bone.Z);
	Vec2 SCords;
	float VMatrix[16];
	memcpy(&VMatrix, (PBYTE*)(BaseAddress + dwViewMatrix), sizeof(VMatrix));
	if (WorldToScreen(Location, SCords, VMatrix, ScrX, ScrY)) {
		return SCords;
	};

	return { 0,0 }
};

void InitImGui(LPDIRECT3DDEVICE9 pDevice)
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(pDevice);
}

bool init = false;
bool esp = false;
bool espbox = false;

float boxw = 0.5f;
int boxthic = 2;

bool menu = false;
long __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice)
{
	if (!init)
	{
		InitImGui(pDevice);
		init = true;
	}

	if (GetAsyncKeyState(VK_END))
	{
		kiero::shutdown();
		return 0;
	}

	if (GetAsyncKeyState(VK_DELETE) & 1)
	{
		menu = !menu;
	}

	if (menu)
	{
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("GameSense");
		ImGui::Checkbox("Esp", &esp);
		ImGui::End();

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	return oEndScene(pDevice);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}	

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
	DWORD wndProcId;
	GetWindowThreadProcessId(handle, &wndProcId);

	if (GetCurrentProcessId() != wndProcId)
		return TRUE; // skip to next window

	window = handle;
	return FALSE; // window found abort search
}

HWND GetProcessWindow()
{
	window = NULL;
	EnumWindows(EnumWindowsCallback, NULL);
	return window;
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
	bool attached = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D9) == kiero::Status::Success)
		{
			kiero::bind(42, (void**)& oEndScene, hkEndScene);
			do
				window = GetProcessWindow();
			while (window == NULL);
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)WndProc);
			attached = true;
		}
	} while (!attached);
	return TRUE;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hMod);
		CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
		break;
	case DLL_PROCESS_DETACH:
		kiero::shutdown();
		break;
	}
	return TRUE;
}
