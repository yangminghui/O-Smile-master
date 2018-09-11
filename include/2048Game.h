#include "stdio.h"
#include "rand.h"

int numbers2048[4][4] = { { } }; // 4*4游戏盘
int tempUnit2048[4] = {}; // For function move()
unsigned score2048 = 0;//计分
int validity2048 = 0; // 玩家移动是否合法0不合法1合法
char option2048[2] = ""; // 玩家选择的方向

void initData(void);//将游戏盘全部置0
void morge2048(void);//检测移动方向遍历棋盘
void printNums2048(void);//打印游戏当前的状态
int isAlive2048(void);//确认玩家是否仍存活
int canEliminate2048(void);//检查游戏状态
int zeroNum2048(void);//检查空位数
void addrandom2048(void);//在随机位置加入2
void move2048(void);
void merge2048(void);

PUBLIC void start2048Game(int fd_stdin, int fd_stdout) {
    printf("Welcome to 2048 Game!\n");
    printf("Use w/a/s/d and enter key to move the blocks.\n");
    printf("For example, if you want to move left, just press \"a\" and the enter key.\n");

    // 初始化数据将棋盘置0
    initData();

    //加入两个随机数
    addrandom2048();
    addrandom2048();
    printNums2048();

    //定时检查游戏状态
    // while (scanf(" %c", &option)) {
    while (read(fd_stdin, option2048, 2)) {
        if (!isAlive2048()) {
            printf("You lose!!!\a\n");
            break;
        }

        morge2048();//检测玩家的移动方式
        if (validity2048) {
            addrandom2048();//随机位置加入2
        }
        validity2048 = 0;
        
        printNums2048();//重新输出当前游戏状态
    }
}

void morge2048(void) {
    /* Morges(moves and merges) the number blocks */
    switch (option2048[0]) {
    case 'w'://上
        for (int j = 0; j <= 3; j++) {
            for (int i = 0, k = 0; i <= 3; i++) {
                tempUnit2048[k++] = numbers2048[i][j];//从第一列第一个数开始按列处理
            }

            move2048();//是否可以移动
            merge2048();//是否可以合并
            move2048();//是否可以移动

            for (int i = 0, k = 0; i <= 3; i++) {
                numbers2048[i][j] = tempUnit2048[k++];//刷新数据
            }
        }
        break;
    case 'a'://左
        for (int i = 0; i <= 3; i++) {
            for (int j = 0, k = 0; j <= 3; j++) {
                tempUnit2048[k++] = numbers2048[i][j];//从第一行第一个数开始按行处理
            }

            move2048();//是否可以移动
            merge2048();//是否可以合并
            move2048();//是否可以移动

            for (int j = 0, k = 0; j <= 3; j++) {
                numbers2048[i][j] = tempUnit2048[k++];//刷新数据
            }
        }
        break;
    case 's'://下
        for (int j = 0; j <= 3; j++) {
            for (int i = 3, k = 0; i >= 0; i--) {
                tempUnit2048[k++] = numbers2048[i][j];//从第一列的最后一个数按列处理
            }

            move2048();//是否可以移动
            merge2048();//是否可以合并
            move2048();//是否可以移动

            for (int i = 3, k = 0; i >= 0; i--) {
                numbers2048[i][j] = tempUnit2048[k++];//刷新数据
            }
        }
        break;
    case 'd'://右
        for (int i = 0; i <= 3; i++) {
            for (int j = 3, k = 0; j >= 0; j--) {
                tempUnit2048[k++] = numbers2048[i][j];//从第一行最后一个数按行处理
            }

            move2048();//是否可以移动
            merge2048();//是否可以合并
            move2048();//是否可以移动

            for (int j = 3, k = 0; j >= 0; j--) {
                numbers2048[i][j] = tempUnit2048[k++];//刷新数据
            }
        }
        break;
    default:
        printf("Illegal input!!!\a\n");
    }
    return;
}

void printNums2048(void) {
//打印得分及游戏盘上当前的状态
    int i, j;
    for (i = 0; i <= 3; i++) {
        for (j = 0; j <= 3; j++) {
            if (numbers2048[i][j] != 0) {
                printf("%7d", numbers2048[i][j]);
            }
            else {
                printf("       ");
            }
        }
        printf("\n");
    }
    printf("Score: %d\n", score2048);

    return;
}

int isAlive2048(void) {
//确认玩家状态
    if (zeroNum2048()) {//是否还有空位
        return 1;
    }
    else if (canEliminate2048()) {//是否还有可消除的
        return 1;
    }
    else
        return 0;
}

int canEliminate2048(void) {
//确认游戏是否还可以进行，当棋盘满且不存在相邻两个相等时淘汰  
    // Rows
    for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 2; j++) {
            if (numbers2048[i][j] == numbers2048[i][j + 1])
                return 1;
        }
    }

    // Columns
    for (int i = 0; i <= 2; i++) {
        for (int j = 0; j <= 3; j++) {
            if (numbers2048[i][j] == numbers2048[i + 1][j])
                return 1;
        }
    }

    return 0;
}

int zeroNum2048(void) {
//确认棋盘是否已满
    int count = 0;
    for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
            if (numbers2048[i][j] == 0)
                count++;
        }
    }
    return count;
}

void addrandom2048(void) {
//在随机的空位置加入2
//随机一个索引值，用随机数除以空位数得到一个余数，那么一定可以找到一个随机的空位
    srand2048(1234);
    
    int index = rand2048() % zeroNum2048();
    int position = 0;
    for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
            if (numbers2048[i][j] == 0) {
                if (position == index) {
                    numbers2048[i][j] = 2;
                    return;
                }
                position++;
            }
        }
    }
    
    return;
}

void move2048(void) {
    int current = -1, count = 0;
    
    //找到这一列/行的第一个空位
    for (int i = 0; i <= 2; i++) {
        if (tempUnit2048[i] == 0) {
            current = i;
            break;
        }
    }

    //这一列/行没有空位，不能移动
    if (current == -1) {
        return;
    }

   //将数字都尽量聚集在一起
    for (; current <= 2; ) {
        if (tempUnit2048[current + 1] == 0) {
            count++;
            current++;
        }
        else {
            validity2048 = 1;
            tempUnit2048[current - count] = tempUnit2048[current + 1];
            current++;
            for (int k = current - count; k <= current; k++)
                tempUnit2048[k] = 0;
        }
    }

    return;
}

void merge2048(void) {
    for (int i = 0; i <= 2; i++) {
        if (tempUnit2048[i] != 0) {//如果存在两个相等的就合并，把后一个置为0
            if (tempUnit2048[i + 1] == tempUnit2048[i]) {
                validity2048 = 1;
                tempUnit2048[i] = tempUnit2048[i + 1] + tempUnit2048[i];
                score2048 += tempUnit2048[i];
                tempUnit2048[i + 1] = 0;
                i++;
            }
        }
        else
            break;
    }

    return;
}

//初始化数据，全部置0
void initData(void) {
    int row = 0, column = 0;
    
    for (row = 0; row < 4; row++) {
        for (column = 0; column < 4; column++) {
            numbers2048[row][column] = 0;
        }
    }
    
    for (row = 0; row < 4; row++) {
        tempUnit2048[row] = 0;
    }

    score2048 = 0;
    validity2048 = 0;
    option2048[2] = "";
}
