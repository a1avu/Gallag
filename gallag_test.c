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
#define MAXBullet 5
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

Bullet bullets[MAXBullet] = {{FALSE, 0, 0}, {FALSE, 0, 0}, {FALSE, 0, 0}, {FALSE, 0, 0}, {FALSE, 0, 0}};
Enemy enemies[MAXEnemies] = {{FALSE, 0, 0, 0}};

Player player[11];          //추가 되는 사람은 항상 11번째 인덱스에 위치하게 했음 (어차피 출력은 10번까지 밖에 안되니)
int player_i = 0;            //nickname의 i값 받을 전역변수
int hero_lives = 3;  // 히어로의 목숨

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
void map(char* nickname, int score, int lives);
void draw_hero();
void clear_hero();
void move_hero();
void fire_bullet();
void update_bullet();
void spawn_enemy();
void update_enemy();
void check_collision();
void game_over();

int main(void){
    srand(time(NULL));  // 무작위 값을 위한 랜덤 시드 초기화
    removeCursor();
    inter_face();
    map(player[player_i].nickname, player[player_i].score, hero_lives);// 맵 및 히어로 소환

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

void move_hero() {
    const DWORD HERO_UPDATE_INTERVAL = 50;   // 히어로 입력 업데이트 주기 (50ms)
    const DWORD ENEMY_UPDATE_INTERVAL = 200; // 적 업데이트 주기 (200ms)
    DWORD lastHeroUpdate = GetTickCount();   // 히어로 입력 마지막 업데이트 시간
    DWORD lastEnemyUpdate = GetTickCount();  // 적 업데이트 마지막 업데이트 시간
    char ch;

    int game_running = 1;

    while (game_running) {
        DWORD currentTime = GetTickCount();

        // **히어로 입력 처리 (50ms 간격)**
        if (currentTime - lastHeroUpdate >= HERO_UPDATE_INTERVAL) {
            lastHeroUpdate = currentTime;

            // 사용자 입력 처리
            if (_kbhit()) { // 키 입력 대기
                ch = _getch();
                clear_hero(hero.x, hero.y);

                switch (ch) {
                case 75: // ← 키
                    if (hero.x > MAP_X + 1)
                        hero.x--;
                    break;
                case 77: // → 키
                    if (hero.x < MAP_X + MAP_WIDTH - 2)
                        hero.x++;
                    break;
                case 32: // 스페이스바 (총알 발사)
                    fire_bullet();
                    break;
                case 27: // Esc 키 (게임 종료)
                    game_running = 0;
                    break;
                }
                draw_hero(hero.x, hero.y);
            }
        }

        // **적 및 게임 상태 업데이트 (200ms 간격)**
        if (currentTime - lastEnemyUpdate >= ENEMY_UPDATE_INTERVAL) {
            lastEnemyUpdate = currentTime;
            
            check_collision();
            update_bullet();
            update_enemy();
            

            // 히어로 생명 확인
            if (hero_lives <= 0) {
                game_over();

                // 게임 종료 또는 재시작 여부 확인
                char choice = _getch();
                if (choice == 'e') {
                    exit(0);
                }

                // 상태 초기화
                hero_lives = 3;
                player[player_i].score = 0;
                spawn_enemy();
                inter_face();
                map(player[player_i].nickname, player[player_i].score, hero_lives);
                continue;
            }

            // 현재 목숨 표시
            gotoxy(40, 19, "Lives: ");
            for (int i = 0; i < hero_lives; i++) {
                printf("♥ ");
            }
            for (int i = hero_lives; i < 3; i++) {
                printf("  ");
            }
        }
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
            enemies[i].y++;  // 적 아래로 이동

            // 적이 히어로와 같은 y 위치에 도달할 때
            if (enemies[i].y == hero.y) {
                if (abs(enemies[i].x - hero.x) <= 1) { // 히어로 위치와 가까운지 확인
                    hero_lives--;  // 히어로의 목숨 하나 감소
                    enemies[i].exist = FALSE;  // 적 제거
                    if (hero_lives <= 0) {
                        gotoxy(MAP_X + MAP_WIDTH / 2, MAP_Y + MAP_HEIGHT / 2, "Game Over");
                        return;
                    }
                    continue;  // 다른 적 업데이트로 이동
                }
            }

            // 적이 맵의 하단 경계에 닿으면 제거, 테두리는 지우지 않음
            if (enemies[i].y > MAP_Y + MAP_HEIGHT - 1) {
                enemies[i].exist = FALSE;
            } else {
                switch (enemies[i].type) {  // 타입별 적 표시
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
    //Sleep(200); 지워놈 게임이 너무 버벅여서  // 적의 이동 속도를 조절하여 천천히 내려오게 함
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
                    gotoxy(enemies[j].x, enemies[j].y, "   ");  // 충돌한 적 위치를 비우기 (화면에서 제거)
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

void map(char *nickname, int score, int lives) {       //map함수에 파라미터 넣어줘서 gameover 시에는 status 창 전체 초기화
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
    printf("%d", score); gotoxy(40, 19, "Lives: ");
    for (int i = 0; i < lives; i++) {
        printf("♥ ");
    }
}

void game_over() {
    system("cls");
    gotoxy(10, 10, "*************************");
    gotoxy(10, 11, "*       GAME OVER       *");
    gotoxy(10, 12, "*************************");
    gotoxy(10, 14, "Press any key to restart");
   char ch = _getch();  // 키 입력을 대기하여 게임 재시작
     if (ch == 13) {  // Enter 키로 게임 재시작
        hero_lives = 3;
        player[player_i].score = 0;
        spawn_enemy();
        move_hero();
    } else if (ch == 27) {  // Esc 키로 게임 종료
        exit(0);
    }
}