#include <stdio.h>
#include <graphics.h>//esayx图形库的头文件
#include <time.h>
#include <math.h>
#include "tools.h"

#include <mmsystem.h>//���ű������ֺ���Ч��ͷ�ļ�
#pragma comment(lib,"winmm.lib")//����������Ҫ�Ŀ��ļ�



//开发日志
//1.构建初始的界面
//2.构建初始的方块数组
//3.绘制初始的方块数组
//4.实现方块的移动

#define WIN_WIDTH				864
#define WIN_HEIGHT				821
#define ROWS					 8
#define COLS					 8
#define BLOCK_TYPE_COUNT		 7


IMAGE imgBg;//表示背景图片
IMAGE imgBlocks[BLOCK_TYPE_COUNT];


struct block {
	int type;//方块的类型,0:表示空白
	int x, y;
	int row, col;//行,列
	int match;//匹配次数
	int tmd;//透明度：0-255，255表示完全不透明,0表示完全透明
};

struct block map[ROWS + 2][COLS + 2];

const int off_x = 200;
const int off_y = 150;
const int block_size = 52;

int click;//表示相邻位置的单机次数，第2次单击，才会交换
int posX1, posY1;//第一次单击的行和列
int posX2, posY2;//第二次单击的行和列



bool isMoving;//表示当前是否正在移动
bool isSwap;//当点击两个相邻的方块后，设置为true

int score;


void init() {
	//创建游戏窗口
	initgraph(WIN_WIDTH, WIN_HEIGHT);

	loadimage(&imgBg, "res/bg2.png");

	char name[64];
	for (int i = 0; i < BLOCK_TYPE_COUNT; i++) {
		//res/1.png....res/7.png
		sprintf_s(name, sizeof(name), "res/%d.png", i + 1);
		loadimage(&imgBlocks[i], name, block_size, block_size, true);
	}


	//配置随机数的随机种子
	srand(time(NULL));


	//初始化方块数组
	for (int i = 1; i <= ROWS; i++) {
		for (int j = 1; j <= COLS; j++) {
			map[i][j].type = 1 + rand() % 4;
			map[i][j].row = i;
			map[i][j].col = j;
			map[i][j].x = off_x + (j - 1) * (block_size + 5);
			map[i][j].y = off_y + (i - 1) * (block_size + 5);
			map[i][j].match = 0;
			map[i][j].tmd = 255;
		}
	}
	click = 0;
	isMoving = false;
	isSwap = false;
	score = 0;
	setFont("Segoe UI Black", 30, 40);

	//���ű�������
	//mciSendString("play res/bg.mp3 repeat", 0, 0, 0);
	//�򿪱�������
	mciSendString("open res/bg.mp3 alias bgm", 0, 0, 0);
	mciSendString("play bgm repeat", 0, 0, 0);
	mciSendString("setaudio bgm volume to 80", 0, 0, 0);

	mciSendString("paly res/start.mp3", 0, 0, 0);

}



void updateWindow() {

	BeginBatchDraw();//开始双缓冲
	putimage(0, 0, &imgBg);

	for (int i = 1; i <= ROWS; i++) {
		for (int j = 1; j <= COLS; j++) {
			if (map[i][j].type) {
				IMAGE* img = &imgBlocks[map[i][j].type - 1];
				putimageTMD(map[i][j].x, map[i][j].y, img, map[i][j].tmd);

			}

		}
	}


	char scoreStr[16];
	sprintf_s(scoreStr, sizeof(scoreStr), "%d", score);
	int x = 750 + (75 - strlen(scoreStr) * 20) /2;
	outtextxy(x, 60, scoreStr);

	EndBatchDraw();//结束双缓冲

}

void exchange(int row1, int col1, int row2, int col2) {
	struct block tmp = map[row1][col1];
	map[row1][col1] = map[row2][col2];
	map[row2][col2] = tmp;

	map[row1][col1].row = row1;
	map[row1][col1].col = col1;
	map[row2][col2].row = row2;
	map[row2][col2].col = col2;
}

void userClick() {
	ExMessage msg;//安装最新版的easyx图形库
	if (peekmessage(&msg) && msg.message == WM_LBUTTONDOWN) {
		/*

		   map[i][j].x = off_x + (j - 1) * (block_size + 5);
		   map[i][j].y = off_y + (i - 1) * (block_size + 5);
	*/

		if (msg.x < off_x || msg.y < off_y) return;

		int col = (msg.x - off_x) / (block_size + 5) + 1;
		int row = (msg.y - off_y) / (block_size + 5) + 1;

		if (col > COLS || row > ROWS) return;


		click++;

		if (click == 1) {

			posX1 = col;
			posY1 = row;
		}
		else if (click == 2) {
			posX2 = col;
			posY2 = row;

			if (abs(posX2 - posX1) + abs(posY2 - posY1) == 1) {
				exchange(posY1, posX1, posY2, posX2);
				click = 0;
				isSwap = true;
				//播放音效
				PlaySound("res/pao.wav", 0, SND_FILENAME | SND_ASYNC);
			}
			else {
				click = 1;
				posX1 = col;
				posY1 = row;
			}

		}

	}
}

void check() {
	for (int i = 1; i <= ROWS; i++) {
		for (int j = 1; j <= COLS; j++) {
			if (map[i][j].type == map[i + 1][j].type && map[i][j].type == map[i - 1][j].type) {
				for (int k = -1; k <= 1; k++)map[i + k][j].match++;
			}

			if (map[i][j].type == map[i][j - 1].type && map[i][j].type == map[i][j + 1].type) {
				for (int k = -1; k <= 1; k++)map[i][j + k].match++;

			}
		}
	}
}

void move() {

	isMoving = false;
	for (int i = ROWS; i > 0; i--) {
		for (int j = 1; j <= COLS; j++) {
			struct block* p = &map[i][j];
			int dx, dy;

			for (int k = 0; k < 4; k++) {
				int x = off_x + (p->col - 1) * (block_size + 5);
				int y = off_y + (p->row - 1) * (block_size + 5);

				dx = p->x - x;
				dy = p->y - y;

				if (dx) p->x -= dx / abs(dx);
				if (dy) p->y -= dy / abs(dy);
			}

			if (dx || dy) isMoving = true;

		}

	}
}


void huanYuan() {
	//发生移动后，而且这个单向移动已经结束
	if (isSwap && !isMoving) {
		//如果没有匹配到三个或三个以上的方块，就要还原
		//to do

		int count = 0;
		for (int i = 1; i <= ROWS; i++) {
			for (int j = 1; j <= COLS; j++) {
				count += map[i][j].match;
			}
		}


		if (count == 0) {//再优化为是否需要消除
			exchange(posY1, posX1, posY2, posX2);

		}

		isSwap = false;
	}

}

void xiaochu() {
	bool flag = false;
	for (int i = 1; i < ROWS; i++) {
		for (int j = 1; j <= COLS; j++) {
			if (map[i][j].match && map[i][j].tmd > 10) {
				if (map[i][j].tmd == 255) {
					flag = true;
				}
				map[i][j].tmd -= 10;
				isMoving = true;
			}
		}
	}
	if (flag) {
		PlaySound("res/clear.wav", 0, SND_FILENAME | SND_ASYNC);
	}

}

void updateGame() {
	//下沉降落
	for (int i = ROWS; i >= 1; i--) {
		for (int j = 1; j <= COLS; j++) {
			if (map[i][j].match) {
				for (int k = i - 1; k >= 1; k--) {
					if (map[k][j].match == 0) {
						exchange(k, j, i, j);
						break;
					}
				}
			}
		}
	}

	//�����µķ��飬���н��䴦��
	for (int j = 1; j <= COLS; j++) {
		int n = 0;
		for (int i = ROWS; i >= 1; i--) {
			if (map[i][j].match) {
				map[i][j].type = 1 + rand() % 7;
				map[i][j].y = off_y - (n + 1) * (block_size + 5);
				n++;
				map[i][j].match = 0;
				map[i][j].tmd = 255;
			}
		}
		score += n;
	}
}

int main(void) {
	init();//初始化

	updateWindow();//处理用户的点击操作

	while (1) {

		userClick();//�����û��ĵ������
		check();//ƥ��������
		move();
		if (!isMoving) xiaochu();

		huanYuan();//��ԭ
		updateWindow();//���´���

		if (!isMoving) updateGame();//更新游戏数据(降落)

		if (isMoving)Sleep(5);//֡�ȴ������������Ż���

	}


	system("pause");//暂停
	return 0;

}