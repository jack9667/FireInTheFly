#include<windows.h>
#include<time.h>
#include <tchar.h>
//MCI
#include <mmsystem.h>// mci播放声音


#pragma comment(lib,"Msimg32.lib")
#pragma comment(lib,"winmm.lib")


#define WINDOW_TITLE L"飞机大战"
#define WINDOW_HEIGHT 852
#define WINDOW_WIDTH 480


//全局变量-双缓存画布
HDC g_hdc;
HDC g_mdc;
HDC g_bufdc;

int iScore = 0;
int maxScore = 0;

//阶段结构体声明
enum GameState {
	GS_Menu,
	GS_Playing,
	GS_Result
};

//函数声明
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM Iparam);
void GameStart(HWND hwnd);
void GameUpdate(HWND hwnd);
void GameEnd(HWND hwnd);
void ChangeToState(GameState gs, HWND hwnd);

//子弹结构体
struct Bullet
{
	int x;
	int y;
	bool isExist = false;
};

//敌人
struct Enemy 
{
	int x;
	int y;
	bool isExit = false;
	bool isDie = false;	//存在才能死亡，即isExist=true
	int iDieAnimationIndex = 0;
	int iDieAnimationTimer = 0;
};


//阶段具体定义
struct GameMenu
{
	HBITMAP hBackground;
	HBITMAP hTitle;
	HBITMAP hGameLoading;

	//初始化工作
	void Init(HWND hwnd) {
		hBackground = (HBITMAP)LoadImage(NULL, L"image/background.bmp", IMAGE_BITMAP, WINDOW_WIDTH, WINDOW_HEIGHT, LR_LOADFROMFILE);
		hTitle = (HBITMAP)LoadImage(NULL, L"image/title.bmp", IMAGE_BITMAP, 429, 84, LR_LOADFROMFILE);
		hGameLoading = (HBITMAP)LoadImage(NULL, L"image/game_loading.bmp", IMAGE_BITMAP, 176, 36, LR_LOADFROMFILE);
	}
	//状态开始，每次进入开始界面都要执行
	void Start(HWND hwnd) {
		SelectObject(g_bufdc, hBackground);
		BitBlt(g_mdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, g_bufdc, 0, 0, SRCCOPY);

		SelectObject(g_bufdc, hTitle);
		TransparentBlt(g_mdc, 20, 50, 429, 84, g_bufdc, 0, 0, 429, 84, RGB(0, 0, 0));

		SelectObject(g_bufdc, hGameLoading);
		TransparentBlt(g_mdc, 150, 600, 176, 36, g_bufdc, 0, 0, 176, 36, RGB(255, 255, 255));

		BitBlt(g_hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, g_mdc, 0, 0, SRCCOPY);
	}
	//状态更新
	void Update(HWND hwnd) {
		//绘制游戏开始界面
	}

	//处理鼠标和键盘事件
	void OnWindowMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message)
		{
		case WM_LBUTTONDOWN:
			//gameState = GS_Playing;
			ChangeToState(GS_Playing, hwnd);
			break;
		default:
			break;
		}
	}

	//状态销毁
	void Destory(HWND hwnd) {

	}
};

struct GamePlaying
{
	HBITMAP hBackground;
	int iBackgroundOffset = 0;

	HBITMAP hHeroArray[2];
	int iHeroIndex = 0;
	int iHeroTimer = 0;

	int iPlayerPositionX = 190;
	int iPlayerPositionY = 600;

	Bullet bulletArray[30];
	HBITMAP hBullet;
	int iBulletTimer = 0;

	bool bIsButtonDown = false;
	POINT pPreMousePoint;

	//敌人
	Enemy enemyArray[30];
	int iEnemySpawnTimer = 0;
	HBITMAP hEnemyArray[5];

	void Init(HWND hwnd) {

		hBackground = (HBITMAP)LoadImage(NULL, L"image/background.bmp", IMAGE_BITMAP, WINDOW_WIDTH, WINDOW_HEIGHT, LR_LOADFROMFILE);
		hHeroArray[0]= (HBITMAP)LoadImage(NULL, L"image/hero1.bmp", IMAGE_BITMAP, 100, 124, LR_LOADFROMFILE);
		hHeroArray[1] = (HBITMAP)LoadImage(NULL, L"image/hero2.bmp", IMAGE_BITMAP, 100, 124, LR_LOADFROMFILE);
		hBullet = (HBITMAP)LoadImage(NULL, L"image/bullet1.bmp", IMAGE_BITMAP, 9, 21, LR_LOADFROMFILE);
		hEnemyArray[0] = (HBITMAP)LoadImage(NULL, L"image/enemy0.bmp", IMAGE_BITMAP, 51, 39, LR_LOADFROMFILE);
		hEnemyArray[1] = (HBITMAP)LoadImage(NULL, L"image/enemy1.bmp", IMAGE_BITMAP, 51, 39, LR_LOADFROMFILE);
		hEnemyArray[2] = (HBITMAP)LoadImage(NULL, L"image/enemy2.bmp", IMAGE_BITMAP, 51, 39, LR_LOADFROMFILE);
		hEnemyArray[3] = (HBITMAP)LoadImage(NULL, L"image/enemy3.bmp", IMAGE_BITMAP, 57, 51, LR_LOADFROMFILE);
		hEnemyArray[4] = (HBITMAP)LoadImage(NULL, L"image/enemy4.bmp", IMAGE_BITMAP, 51, 39, LR_LOADFROMFILE);


		//随机数种子初始化
		srand((unsigned)time(NULL));
	}

	void Start(HWND hwnd) {
		////声音播放
		mciSendString(L"open sound/game_music.wav", NULL, 0, 0);
		mciSendString(L"play sound/game_music.wav", NULL, 0, 0);
	}

	void Update(HWND hwnd) {
		SelectObject(g_bufdc, hBackground);
		//画面移动偏移量--画背景图
		iBackgroundOffset += 2;
		if (iBackgroundOffset>WINDOW_HEIGHT)
		{
			iBackgroundOffset = 0;
		}
		BitBlt(g_mdc, 0, iBackgroundOffset, WINDOW_WIDTH, WINDOW_HEIGHT, g_bufdc, 0, 0, SRCCOPY);
		BitBlt(g_mdc, 0, -(WINDOW_HEIGHT-iBackgroundOffset), WINDOW_WIDTH, WINDOW_HEIGHT, g_bufdc, 0, 0, SRCCOPY);

		//画飞机
		iHeroTimer++;
		if (iHeroTimer>=5) {
			iHeroIndex++;
			iHeroIndex %= 2;
		}
		SelectObject(g_bufdc, hHeroArray[iHeroIndex]);
		TransparentBlt(g_mdc, iPlayerPositionX, iPlayerPositionY, 100, 124, g_bufdc, 0, 0, 100, 124, RGB(255, 255, 255));

		//控制子弹生成
		iBulletTimer++;
		if (iBulletTimer % 10 == 0) {
			for (int i=0;i<30;i++)
			{
				if (bulletArray[i].isExist == false) {
					int x = iPlayerPositionX + 50 - 4;
					bulletArray[i].x = x;
					bulletArray[i].y = iPlayerPositionY;
					bulletArray[i].isExist = true;
					break;
				}
			}
		}
		//绘制子弹，控制子弹的运动，控制子弹死亡
		SelectObject(g_bufdc, hBullet);
		for (int i=0;i<30;i++)
		{
			if (bulletArray[i].isExist == true) {
				bulletArray[i].y -= 20;
				if (bulletArray[i].y<-20)
				{
					bulletArray[i].isExist = false;
					continue;
				}
				TransparentBlt(g_mdc, bulletArray[i].x, bulletArray[i].y, 9, 21, g_bufdc, 0, 0, 9, 21, RGB(255, 255, 255));
			}
		}

		//控制主角移动
		if (bIsButtonDown)
		{
			POINT pNowMousePoint;
			GetCursorPos(&pNowMousePoint);
			int xOffset = pNowMousePoint.x - pPreMousePoint.x;
			int yOffset = pNowMousePoint.y - pPreMousePoint.y;
			iPlayerPositionX += xOffset;
			iPlayerPositionY += yOffset;
			pPreMousePoint = pNowMousePoint;
			if (iPlayerPositionX<0)
			{
				iPlayerPositionX = 0;
			}
			if (iPlayerPositionX>WINDOW_WIDTH-100)
			{
				iPlayerPositionX = WINDOW_WIDTH - 100;
			}
			if (iPlayerPositionY < 0)
			{
				iPlayerPositionY = 0;
			}
			if (iPlayerPositionY > WINDOW_HEIGHT - 124)
			{
				iPlayerPositionY = WINDOW_HEIGHT - 124;
			}
		}
		//控制敌人
		iEnemySpawnTimer++;
		if (iEnemySpawnTimer%10==0)
		{
			for (int i=0;i<30;i++)
			{
				if (enemyArray[i].isExit == false) {
					int y = -39;
					int x = rand() % (WINDOW_WIDTH - 51);	//随机生成数字
					enemyArray[i].x = x;
					enemyArray[i].y = y;
					enemyArray[i].isExit = true;
					enemyArray[i].isDie = false;
					enemyArray[i].iDieAnimationIndex = 0;
					enemyArray[i].iDieAnimationTimer = 0;
					break;
				}
			}
		}
		////控制敌人的运动
		for (int i = 0; i < 30; i++) {
			if (enemyArray[i].isExit == true && enemyArray[i].isDie == false)
			{
				enemyArray[i].y += 10;
				if (enemyArray[i].y > WINDOW_HEIGHT)
					enemyArray[i].isExit = false;
			}
		}

		//画敌人
		for (int i = 0; i < 30; i++)
		{
			if (enemyArray[i].isExit==true)
			{
				if (enemyArray[i].isDie==true)
				{
					//画死亡动画
					enemyArray[i].iDieAnimationTimer++;
					enemyArray[i].iDieAnimationIndex = enemyArray[i].iDieAnimationTimer / 5;
					if (enemyArray[i].iDieAnimationIndex>4)
					{
						enemyArray[i].isExit = false;
						enemyArray[i].isDie = false;
						continue;
					}
					SelectObject(g_bufdc, hEnemyArray[enemyArray[i].iDieAnimationIndex]);
					if (enemyArray[i].iDieAnimationIndex==3)
					{
						TransparentBlt(g_mdc, enemyArray[i].x, enemyArray[i].y, 57, 51, g_bufdc, 0, 0, 57, 51, RGB(255, 255, 255));
					}
					else
					{
						TransparentBlt(g_mdc, enemyArray[i].x, enemyArray[i].y, 51, 39, g_bufdc, 0, 0, 51, 39, RGB(255, 255, 255));
					}
					
				} 
				else
				{
					SelectObject(g_bufdc, hEnemyArray[0]);
					TransparentBlt(g_mdc, enemyArray[i].x, enemyArray[i].y, 51, 39, g_bufdc, 0, 0, 51, 39, RGB(255, 255, 255));
				}
			} 
		}

		//子弹和敌人的碰撞检测
		for (int i=0;i<30;i++)
		{
			if (enemyArray[i].isExit==true&&enemyArray[i].isDie==false)
			{
				for (int j=0;j<30;j++)
				{
					if (bulletArray[j].isExist) {
						if (IsInclude(enemyArray[i],bulletArray[j].x+4,bulletArray[j].y+10))
						{
							bulletArray[j].isExist = false;
							enemyArray[i].isDie = true;

							//打中加分播放声音
							iScore++;
							if (maxScore < iScore)
								maxScore++;
							PlaySound(L"sound/enemy0_down.wav", nullptr, SND_FILENAME|SND_ASYNC);

							break;
						}
					}
				}
			}
		}

		//主角和敌人的碰撞检测
		for (int i=0;i<30;i++)
		{
			if (enemyArray[i].isExit && !enemyArray[i].isDie) {
				bool isInclude1 = IsInclude(enemyArray[i], iPlayerPositionX + 50, iPlayerPositionY);
				bool isInclude2 = IsInclude(enemyArray[i], iPlayerPositionX-10, iPlayerPositionY + 84);
				bool isInclude3 = IsInclude(enemyArray[i], iPlayerPositionX + 90, iPlayerPositionY + 84);
				if (isInclude1||isInclude2||isInclude3)
				{
					//发生了碰撞
					mciSendString(L"stop sound/game_music.wav", NULL, 0, 0);
					PlaySound(L"sound/game_over.wav", NULL, SND_FILENAME | SND_ASYNC);
					ChangeToState(GS_Result, hwnd);
					break;
					
				}
			}
		}


		//最后将整个mdc画布画到hdc上
		BitBlt(g_hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, g_mdc, 0, 0, SRCCOPY);
	}

	bool IsInclude(Enemy enemy, int x, int y) {
		if (x>enemy.x&&y>enemy.y&&x<enemy.x+51&&y<enemy.y+39)
		{
			return true;
		}
		return false;
	}

	//处理鼠标和键盘事件
	void OnWindowMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message)
		{
		case WM_LBUTTONDOWN:
			bIsButtonDown = true;
			GetCursorPos(&pPreMousePoint);	//获取鼠标点坐标
			break;
		case WM_LBUTTONUP:
			bIsButtonDown = false;
			break;
		default:
			break;
		}
	}

	void Destory(HWND hwnd) {
		for (int i = 0; i < 30; i++) {
			enemyArray[i].isExit = false;
			enemyArray[i].isDie = false;
			bulletArray[i].isExist = false;
			iPlayerPositionX = 190;
			iPlayerPositionY = 600;
		}
	}
};

struct GameResult
{
	HBITMAP hGameOver;
	wchar_t scoreText[10];
	wchar_t mscoreText[10];


	void Init(HWND hwnd) {
		hGameOver = (HBITMAP)LoadImage(NULL, L"image/gameover.bmp",IMAGE_BITMAP, WINDOW_WIDTH, WINDOW_HEIGHT, LR_LOADFROMFILE);
		HFONT font = CreateFont(40, 0, 0, 0, 0, 0, 0, 0, GB2312_CHARSET, 0, 0, 0, 0, TEXT("微软雅黑"));	//自提创建
		SelectObject(g_mdc, font);//给画布设置字体
		SetBkMode(g_mdc, TRANSPARENT);//设置文字背景为透明

	}
	void Start(HWND hwnd) {
		swprintf_s(scoreText, L"%d", iScore);
		swprintf_s(mscoreText, L"%d", maxScore);
	}
	void Update(HWND hwnd) {
		SelectObject(g_bufdc, hGameOver);
		BitBlt(g_mdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, g_bufdc, 0, 0, SRCCOPY);
		//绘制文本
		
		
		TextOut(g_mdc, 225, 600, scoreText, wcslen(scoreText));
		TextOut(g_mdc, 225, 250, mscoreText, wcslen(mscoreText));


		BitBlt(g_hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, g_mdc, 0, 0, SRCCOPY);

	}
	//处理鼠标和键盘事件
	void OnWindowMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

	}
	void Destory(HWND hwnd) {

	}
};


//全局变量-游戏阶段
GameState gameState;
GameMenu gameMenu;
GamePlaying gamePlaying;
GameResult gameResult;



//主函数
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {

	WNDCLASSEX wndClass = { 0 };
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc = WndProc;
	wndClass.lpszClassName = L"myclassname";
	wndClass.hInstance = hInstance;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);	//使用系统的光标


	if (!RegisterClassEx(&wndClass)) {
		return -1;
	}

	HWND hwnd = CreateWindow(L"myclassname", WINDOW_TITLE, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);

	ShowWindow(hwnd, nShowCmd);
	UpdateWindow(hwnd);

	//进行游戏初始化的代码
	GameStart(hwnd);

	DWORD tNow = GetTickCount();
	DWORD tPre = GetTickCount();

	//消息处理
	MSG msg = { 0 };
	while (msg.message!=WM_QUIT)
	{
		if (PeekMessage(&msg,0,NULL, NULL, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			//帧率速度
			tNow = GetTickCount();
			if (tNow - tPre > 10) {
				GameUpdate(hwnd);
				tPre = tNow;
			}
		}

	}

	
	GameEnd(hwnd);
	UnregisterClass(L"myclassname", hInstance);	//注销窗口


	return 0;
}


void ChangeToState(GameState gs, HWND hwnd) {
	gameState = gs;
	switch (gs)
	{
	case GS_Menu:
		gameMenu.Start(hwnd);
		break;
	case GS_Playing:
		gamePlaying.Start(hwnd);
		break;
	case GS_Result:
		gameResult.Start(hwnd);
		break;
	
	}
}


//消息处理的回调函数。窗口自动处理各种消息
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lParam) {

	switch (message)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		switch (gameState)
		{
		case GS_Menu:
			gameMenu.OnWindowMessage(hwnd, message, wparam, lParam);
			break;
		case GS_Playing:
			gamePlaying.OnWindowMessage(hwnd, message, wparam, lParam);
			break;
		case GS_Result:
			gamePlaying.OnWindowMessage(hwnd, message, wparam, lParam);
			break;
		}
		break;
	case WM_KEYDOWN:
		switch (wparam)
		{
			case 81:
			MessageBox(hwnd, L"已暂停", L"title", 0);
			//mciSendString(L"stop sound/game_music.wav", NULL, 0, 0);
			
				break;
			default:
				GameEnd(hwnd);
				GameStart(hwnd);
				break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
	default:
		return DefWindowProc(hwnd, message, wparam, lParam);
		break;
	}
	return 0;
}

void GameStart(HWND hwnd) {
	//创建画布
	g_hdc = GetDC(hwnd);
	g_mdc = CreateCompatibleDC(g_hdc);
	g_bufdc = CreateCompatibleDC(g_hdc);

	HBITMAP whiteBmp = CreateCompatibleBitmap(g_hdc, WINDOW_WIDTH, WINDOW_HEIGHT);
	SelectObject(g_mdc, whiteBmp);
	

	//3中阶段的初始化
	gameMenu.Init(hwnd);
	gamePlaying.Init(hwnd);
	gameResult.Init(hwnd);

	//设置默认状态
	gameState = GS_Menu;
	gameMenu.Start(hwnd);
}

void GameUpdate(HWND hwnd) {
	switch (gameState)
	{
	case GS_Menu:
		gameMenu.Update(hwnd);
		break;
	case GS_Playing:
		gamePlaying.Update(hwnd);
		break;
	case GS_Result:
		gameResult.Update(hwnd);
		break;
	default:
		break;
	}
}

void GameEnd(HWND hwnd) {
	//处理下分数
	iScore = 0;
	if (maxScore <= iScore) {
		maxScore = iScore;
	}
	//销毁画布
	DeleteDC(g_bufdc);
	DeleteDC(g_mdc);
	ReleaseDC(hwnd,g_hdc);

	//各个状态的销毁操作
	gameMenu.Destory(hwnd);
	gamePlaying.Destory(hwnd);
	gameResult.Destory(hwnd);
}