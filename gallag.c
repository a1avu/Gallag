#include<stdio.h>
#include<stdlib.h>  //rand()
#include<time.h>    // rand값 초기화
#include<windows.h> // gotoxy
#include<conio.h> // 콘솔 입출력 getch()

#define MAP_WIDTH 20
#define MAP_HEIGHT 35
#define MAP_X 3
#define MAP_Y 2

typedef struct {
    int score;
    char nickname[20];        //nickname이랑 score 묶기 위한 구조체 생성
} Player;

Player player[11];          //추가 되는 사람은 항상 11번째 인덱스에 위치하게 했음 (어차피 출력은 10번까지 밖에 안되니)
int player_i = 0;            //nickname의 i값 받을 전역변수


void gotoxy(int x, int y, char* s){       
    COORD Pos;
    Pos.X = 2*x;
    Pos.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
    printf("%s", s);
}

void inter_face();
void map(char* nickname, int score);

int main(void){
    inter_face();
    map(player[player_i].nickname, player[player_i].score);

    return 0;
}



void inter_face() {
    system("cls");
    int i, j;

    for (i = MAP_X; i <= MAP_WIDTH + MAP_X; i++) {
        gotoxy(i, MAP_Y, "■");
    }
    for (j = MAP_Y + 1; j <= MAP_HEIGHT + MAP_Y; j++) {
        gotoxy(MAP_X, j, "■");
        gotoxy(MAP_X + MAP_WIDTH, j, "■");
    }
    for (i = MAP_X; i <= MAP_WIDTH + MAP_X; i++) {
        gotoxy(i, MAP_Y + MAP_HEIGHT, "■");
    }

    gotoxy(10, 15, "★ 게임 시작 ★");
    gotoxy(8, 16, "방향키로 조작 합니다.");
    gotoxy(5, 17, "닉네임을 입력하면 게임이 시작됩니다.");
    gotoxy(6, 19, "nickname : ");
    scanf("%s", player[player_i].nickname);
    system("cls");
}

void map(char *nickname, int score) {       //map함수에 파라미터 넣어줘서 gameover 시에는 status 창 전체 초기화
    int i, j;

    for (i = MAP_X; i <= MAP_WIDTH + MAP_X; i++) {
        gotoxy(i, MAP_Y, "■");
    }
    for (j = MAP_Y + 1; j <= MAP_HEIGHT + MAP_Y; j++) {
        gotoxy(MAP_X, j, "■");
        gotoxy(MAP_X + MAP_WIDTH, j, "■");
    }
    for (i = MAP_X; i <= MAP_WIDTH + MAP_X; i++) {
        gotoxy(i, MAP_Y + MAP_HEIGHT, "■");
    }

    gotoxy(42, 4, "<조작법>");
    gotoxy(40, 6, "<방향키> : →, ←, ↑, ↓");
    gotoxy(40, 7, "<Esc> : exit");
    gotoxy(40, 8, " <P> : pause");
    gotoxy(40, 9, " <e> : real exit game");
    
    gotoxy(42, 13,"<STATUS>");
    gotoxy(40, 15, "nickname: ");
    printf("%s", nickname);
    gotoxy(40, 17, "score: ");
    printf("%d", score);
}