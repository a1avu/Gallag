#include<stdio.h>
#include<stdlib.h>  //rand()
#include<time.h>    // rand값 초기화
#include<windows.h> // gotoxy
#include<conio.h> // 콘솔 입출력 getch()

#define MAP_WIDTH 20
#define MAP_HEIGHT 35
#define MAP_X 3
#define MAP_Y 2

#define MAXEnemies 5
#define MAXBullet 2
#define TRUE 1
#define FALSE 0

typedef struct{
    int x;
    int y;
}Hero;

Hero hero = {13,35};

typedef struct {
    int score;
    char nickname[20];        //nickname이랑 score 묶기 위한 구조체 생성
} Player;

typedef struct{
    int exist;
    int x;
    int y;
}Bullet;

typedef struct {
    int exist;
    int x;
    int y;
    int type;
} Enemy;

Bullet bullets[MAXBullet] = {{FALSE, 0, 0}, {FALSE, 0, 0}};
Enemy enemies[MAXEnemies] = {{FALSE, 0, 0, 0}};

Player player[11];          //추가 되는 사람은 항상 11번째 인덱스에 위치하게 했음 (어차피 출력은 10번까지 밖에 안되니)
int player_i = 0;            //nickname의 i값 받을 전역변수

void removeCursor(void) //11/12 커서지우기 함수 (퍼옴)
{
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = 0;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}
void gotoxy(int x, int y, char* s){       
    COORD Pos;
    Pos.X = 2*x;
    Pos.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
    printf("%s", s);
}

void inter_face();
void map(char* nickname, int score);
void draw_hero();
void clear_hero();
void move_hero();
void fire_bullet();
void update_bullet();
void spawn_enemy();
void update_enemy();
void check_collision();

int main(void){
    srand(time(NULL));  // 무작위 값을 위한 랜덤 시드 초기화
    removeCursor();
    inter_face();
    map(player[player_i].nickname, player[player_i].score);// 맵 및 히어로 소환

    move_hero();
    return 0;
}

void draw_hero(int x, int y){ //x4~x22, y35,36 안에서만 움직임
    gotoxy(x,y," ▲ ");
    gotoxy(x,y+1,"◀□▶");
}

void clear_hero(int x, int y){
    gotoxy(x,y,"   ");
    gotoxy(x,y+1,"   ");
}

void move_hero(){
    char ch;

    while(1){
        
        if(_kbhit()){
            
            ch = _getch();
            clear_hero(hero.x,hero.y);
            
            switch(ch){
                case 75: 
                    if(hero.x> MAP_X + 1)
                        hero.x--;
                    break;
                case 77: 
                    if(hero.x < MAP_X + MAP_WIDTH - 2)
                        hero.x++;
                    break;
                case 32:
                    fire_bullet();
                    break;
            }

            draw_hero(hero.x, hero.y); 
        }
        update_bullet();//루프문안에 있으므로 계속 돌아감
        update_enemy();           // 적 상태 업데이트
        check_collision();        // 충돌 검사
        Sleep(50);
    }
}

void fire_bullet(){ //장전단계라고 생각하면 됌
    for(int i = 0; i<MAXBullet; i++){
        if(!bullets[i].exist){ //exist가 true라면 총알 준비
            bullets[i].x = hero.x + 1;
            bullets[i].y = hero.y - 1;
            bullets[i].exist = TRUE;
            break;
        }
    }
}

void update_bullet(){
    for(int i = 0; i<MAXBullet; i++){
        if(bullets[i].exist){//불릿이 존재하면
            gotoxy(bullets[i].x, bullets[i].y, " "); //원래 있던 자리를 지워줌
            bullets[i].y--;

            if(bullets[i].y <= MAP_Y) //맵에서 총알이 벗어나면 지워버림
                bullets[i].exist = FALSE;
            else
                gotoxy(bullets[i].x, bullets[i].y, "º"); //아니면 총알 앞으로 한칸 전진
        }
    }
}

void spawn_enemy() {
    for (int i = 0; i < MAXEnemies; i++) {
        if (!enemies[i].exist) {                 // 빈 적 위치에 생성
            enemies[i].x = MAP_X + 1 + rand() % (MAP_WIDTH - 2);  // x 위치 랜덤 설정
            enemies[i].y = MAP_Y + 1;
            enemies[i].exist = TRUE;
            enemies[i].type = rand() % 3;         // 적의 타입 무작위 선택 (0, 1, 또는 2)
            break;
        }
    }
}

// 적의 위치 업데이트 및 화면 표시
void update_enemy() {
    for (int i = 0; i < MAXEnemies; i++) {
        if (enemies[i].exist) {
            gotoxy(enemies[i].x, enemies[i].y, "   ");  // 기존 위치 지우기
            enemies[i].y++;                             // 적 아래로 이동
            if (enemies[i].y > MAP_Y + MAP_HEIGHT) {
                enemies[i].exist = FALSE;               // 맵 밖으로 나가면 제거
            } else {
                switch (enemies[i].type) {              // 타입별 적 표시
                    case 0:
                        gotoxy(enemies[i].x, enemies[i].y, " @ ");
                        break;
                    case 1:
                        gotoxy(enemies[i].x, enemies[i].y, " # ");
                        break;
                    case 2:
                        gotoxy(enemies[i].x, enemies[i].y, " & ");
                        break;
                }
            }
        }
    }
    if (rand() % 10 == 0) spawn_enemy();  // 일정 확률로 적 생성
}

// 총알과 적의 충돌 검사 함수
void check_collision() {
    for (int i = 0; i < MAXBullet; i++) {
        if (bullets[i].exist) {
            for (int j = 0; j < MAXEnemies; j++) {
                if (enemies[j].exist && bullets[i].x == enemies[j].x && bullets[i].y == enemies[j].y) {
                    bullets[i].exist = FALSE;            // 충돌 시 총알 제거
                    enemies[j].exist = FALSE;            // 충돌 시 적 제거
                    player[player_i].score += 10;        // 점수 추가
                    gotoxy(40, 17, "score: ");
                    printf("%d", player[player_i].score);
                    break;
                }
            }
        }
    }
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

    draw_hero(13,35); //11.06 hero 맵만들때 같이 그려지게

    gotoxy(42, 4, "<조작법>");
    gotoxy(40, 6, "<방향키> : ←, →");
    gotoxy(40, 7, "<Esc> : exit");
    gotoxy(40, 8, " <P> : pause");
    gotoxy(40, 9, " <e> : real exit game");
    
    gotoxy(42, 13,"<STATUS>");
    gotoxy(40, 15, "nickname: ");
    printf("%s", nickname);
    gotoxy(40, 17, "score: ");
    printf("%d", score);
}