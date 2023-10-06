#pragma once
#include <Windows.h>

class WinAPI
{
private:
	WinAPI(const WinAPI& winapi) = delete;
	WinAPI& operator=(const WinAPI& winapi) = delete;

	WinAPI();
	~WinAPI();

	WNDCLASSEX w{}; // ウィンドウクラスの設定
	HWND hwnd{};

	POINT windowSize{};
	POINT systemWindowSize{};

	//ウィンドウプロシージャ
	static LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:
	bool setWindowSize(int sizeX, int sizeY, const POINT* pos = nullptr, bool bRepaint = true);

public:
	static constexpr int window_width = 1920;
	static constexpr int window_height = 1080;

	// 初期状態のウィンドウタイトル
	static constexpr char winTitleDef[] = "YTエンジン";
	static constexpr wchar_t winTitleDef_wc[] = L"YTエンジン";

	// アスペクト比固定でウィンドウサイズを変更
	bool setWindowWidth(int sizeX);
	// アスペクト比固定でウィンドウサイズを変更
	bool setWindowHeight(int sizeY);

	inline const POINT& getWindowSize() const { return windowSize; }
	inline const POINT& getSystemWindowSize() const { return systemWindowSize; }

	static WinAPI* getInstance();

	HWND getHwnd();
	WNDCLASSEX getW();

	void setWindowText(const LPCSTR window_title);

	//ゲームループ内で毎回呼びだす。trueが返ってきたらすぐに終了させるべき
	//@return 異常の有無をbool型で返す(true == 異常 , false == 正常)
	bool processMessage();
};
