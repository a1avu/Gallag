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
#define MAXBullet 8
#define BOSSBullet 6
#define MAXItem 5

#define TRUE 1
#define FALSE 0

typedef struct{
    int x;
    int y;
}Hero;

typedef struct {
    int score;
    char nickname[20];        //nickname이랑 score 묶기 위한 구조체 생성
} Player;

typedef struct{
    int active; //얘도 false 일때는 작동 안함 안보임
    int exist;
    int x;
    int y;
}Bullet;

typedef struct {
    int exist;
    int x;
    int y;
    int type; //적의 모양
    int direction; // 이동 방향 추가 (0: 아래, 1: 왼쪽 대각선, 2: 오른쪽 대각선)
} Enemy;

typedef struct {
    int exist;
    int x;
    int y;
    int origin_life;
    int life;
    int direction;
}Boss;

typedef struct {
    int active; // 이게 false가 되면 아예 안나오게 되는거임
    int exist;
    int x;
    int y;
    int type; //item 종류
}Item; 
typedef struct ScoreNode {
    char nickname[20];
    int score;
    struct ScoreNode* next;
} ScoreNode;

ScoreNode* head = NULL; // 리스트의 헤드 포인터

Hero hero = {13,35};
Bullet bullets[MAXBullet] = {{TRUE, FALSE, 0, 0},{TRUE, FALSE, 0, 0},
        {TRUE, FALSE, 0, 0},{FALSE, FALSE, 0, 0},{FALSE, FALSE, 0, 0},
        {FALSE, FALSE, 0, 0},{FALSE, FALSE, 0, 0}, {FALSE, FALSE, 0, 0},};
Enemy enemies[MAXEnemies] = {{FALSE, 0, 0, 0}};
Boss boss = {FALSE, (MAP_X + MAP_WIDTH)/2 - 1, MAP_Y +1, 0, 0, 0};
Bullet boss_bullets[BOSSBullet] = {{FALSE, 0, 0}, {FALSE, 0, 0}, {FALSE, 0, 0}, {FALSE, 0, 0}, {FALSE, 0, 0}, {FALSE, 0, 0}};
Item item[MAXItem] = {{TRUE, FALSE, 0 , 0 , 0}, {TRUE, FALSE, 0 , 0 , 0}, {TRUE, FALSE, 0 , 0 , 0}, {TRUE, FALSE, 0 , 0 , 0}, {TRUE, FALSE, 0 , 0 , 0}};

Player player[11];        //추가 되는 사람은 항상 11번째 인덱스에 위치하게 했음 (어차피 출력은 10번까지 밖에 안되니)
int player_i = 0;   //nickname의 i값 받을 전역변수
int hero_lives = 3;  // 히어로의 목숨

// 보스가 자꾸 게임 재시작하자마자 나와서 따로 빼둠 
DWORD lastBossUpdate = 0; // 보스 이동 시간 관리
DWORD for_bossUpdate = 0; // 마지막으로 보스 생성된 시점
DWORD lastBossFire = 0; //마지막으로 총알 발사한 시점



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
void draw_boss(int, int);
void erase_boss();
void boss_update();
void check_collision();
void game_over();
void boss_fire_bullet();
void boss_update_bullet();
void game_reset();

void spawn_item();
void update_item();
void print_lives();
void add_score();
void sort_scores();
void print_top_scores();
void save_scores_to_file();
void free_score_list();
void load_scores_from_file();

int main(void){
    srand(time(NULL));  // 무작위 값을 위한 랜덤 시드 초기화
    removeCursor();
    load_scores_from_file();
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
    const DWORD BULLET_UPDATE_INTERVAL = 100; // 총알 업데이트 주기 (100ms)
    const DWORD Item_UPDATE_INTERVAL = 200; // 아이템 업데이트 주기 (100ms)

    DWORD lastHeroUpdate = GetTickCount();   // 히어로 입력 마지막 업데이트 시간
    DWORD lastEnemyUpdate = GetTickCount();  // 적 업데이트 마지막 업데이트 시간
    DWORD lastBulletUpdate = GetTickCount(); // 총알 업데이트를 위한 시간
    DWORD lastItemUpdate = GetTickCount(); // 아이템 업데이트를 위한 시간

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
                    game_over();
                    break;
                }
                draw_hero(hero.x, hero.y);
            }
        }

        // **적 및 게임 상태 업데이트 (200ms 간격)**
        if (currentTime - lastEnemyUpdate >= ENEMY_UPDATE_INTERVAL) {
            lastEnemyUpdate = currentTime;
            check_collision();
            update_enemy();
            update_item();
        }

        //보스 생성 그냥 하나로 묶음 -> 오류가 너무 많아서 갈아 엎어버림
        boss_update();

        // 총알 이동
        if (currentTime - lastBulletUpdate >= BULLET_UPDATE_INTERVAL) {
            lastBulletUpdate = currentTime;
            update_bullet();
        }
       
        
        // 히어로 생명 확인
        if (hero_lives <= 0) {
            game_over();

            // 상태 초기화 및 각종 시간 초기화
            game_reset();                
            lastHeroUpdate = GetTickCount();
            lastEnemyUpdate = GetTickCount();
            lastBulletUpdate = GetTickCount();
            lastItemUpdate = GetTickCount();
            continue;
        }
        
    }
}
void fire_bullet(){ //장전단계라고 생각하면 됌
    for(int i = 0; i<MAXBullet; i++){
        if(!(bullets[i].exist) && (bullets[i].active)){ //exist가 false고 active가 true면 총알 준비
            bullets[i].x = hero.x;
            bullets[i].y = hero.y - 1;
            bullets[i].exist = TRUE;
            break;
        }
    }
}

void update_bullet(){
    for(int i = 0; i<MAXBullet; i++){
        if(bullets[i].exist && bullets[i].active){//불릿이 존재하면
            gotoxy(bullets[i].x, bullets[i].y, " "); //원래 있던 자리를 지워줌
            bullets[i].y--;

            if(bullets[i].y <= MAP_Y) //맵에서 총알이 벗어나면 지워버림
                bullets[i].exist = FALSE;
            else
                gotoxy(bullets[i].x, bullets[i].y, "º"); //아니면 총알 앞으로 한칸 전진
        }
    }
}

void boss_fire_bullet(){ //위의 코드 재사용 해볼거임 // 장전단계
    for(int i = 0; i<BOSSBullet; i++){
        if(!boss_bullets[i].exist){ //exist가 true라면 총알 준비

            if(i % 3 == 0){ // 왼쪽 대포
                boss_bullets[i].x = boss.x + 1;
                boss_bullets[i].y = boss.y + 5;
            }
            else if(i % 3 == 1){// 가운데 대포
                boss_bullets[i].x = boss.x + 4;
                boss_bullets[i].y = boss.y + 5;
            }
            else if(i % 3 == 2){// 오른쪽대포
                boss_bullets[i].x = boss.x + 7;
                boss_bullets[i].y = boss.y + 5;
            }

            boss_bullets[i].exist = TRUE;
            if(i==2)   // 한번 브레이크를 걸어줘야 동시에 3발이 나가고 총 화면에 6발까지 찍힘
                break;
        }
    }
}

void boss_update_bullet(){
    const DWORD BOSS_BULLET_UPDATE_INTERVAL = 100; //총알 이동주기(200ms);
    static DWORD lastBossBulletUpdate = 0; //총알 이동관련

    DWORD currentTime = GetTickCount();

    if (currentTime - lastBossBulletUpdate < BOSS_BULLET_UPDATE_INTERVAL) {
        return; // 총알 이동 주기가 아직 도래하지 않았으면 반환
    }
    lastBossBulletUpdate = currentTime;

    for(int i = 0; i<BOSSBullet; i++){
        if(boss_bullets[i].exist){//불릿이 존재하면
            gotoxy(boss_bullets[i].x, boss_bullets[i].y, " "); //원래 있던 자리를 지워줌
            boss_bullets[i].y++;

            if(boss_bullets[i].y >= MAP_Y+MAP_HEIGHT) //맵에서 총알이 벗어나면 지워버림
                boss_bullets[i].exist = FALSE;
            else
                gotoxy(boss_bullets[i].x, boss_bullets[i].y, "|"); //아니면 총알 앞으로 한칸 전진
        }
    }
}

// 적의 생성 및 초기화
void spawn_enemy() {
    for (int i = 0; i < MAXEnemies; i++) {
        if (!enemies[i].exist) { // 빈 적 위치에 생성
            enemies[i].x = MAP_X + 1 + rand() % (MAP_WIDTH - 2); // x 위치 랜덤 설정
            enemies[i].y = MAP_Y + 1;
            enemies[i].exist = TRUE;
            enemies[i].direction = rand() % 3; // 0: 아래, 1: 왼쪽 대각선, 2: 오른쪽 대각선
            enemies[i].type = rand() % 3; // 적의 타입 무작위 선택 (0, 1, 또는 2)
            break;
        }
    }
}
// 적의 위치 업데이트 및 화면 표시
void update_enemy() {

    for (int i = 0; i < MAXEnemies; i++) {
        if (enemies[i].exist) {
            // 기존 위치를 지우기
            gotoxy(enemies[i].x, enemies[i].y, "   "); // 기존 위치 지우기

            // 적 이동
            int new_x = enemies[i].x;
            int new_y = enemies[i].y;

            switch (enemies[i].direction) {
                case 0: // 아래로
                    new_y++;
                    break;
                case 1: // 왼쪽 대각선 아래로
                    new_x--;
                    new_y++;
                    break;
                case 2: // 오른쪽 대각선 아래로
                    new_x++;
                    new_y++;
                    break;
            }

            // 경계 검사: 새로운 위치가 맵의 경계를 넘지 않는지 확인
            if (new_x < MAP_X + 1) {
                // 왼쪽 경계에 도달했을 경우 오른쪽으로 방향 전환
                enemies[i].direction = 2; // 오른쪽 대각선으로 변경
                new_x = MAP_X + 1; // 경계에 맞추기
            } else if (new_x >= MAP_X + MAP_WIDTH - 1) {
                // 오른쪽 경계에 도달했을 경우 왼쪽으로 방향 전환
                enemies[i].direction = 1; // 왼쪽 대각선으로 변경
                new_x = MAP_X + MAP_WIDTH - 2; // 경계에 맞추기
            }

            // 적의 y 좌표도 맵 경계를 넘는지 확인
            if (new_y >= MAP_Y + MAP_HEIGHT) {
                // 하단 경계에 도달했을 경우 적 제거
                enemies[i].exist = FALSE;
            } else {
                enemies[i].x = new_x; // 유효한 위치로 업데이트
                enemies[i].y = new_y;

                // 새로운 위치에 적 표시
                switch (enemies[i].type) { // 타입별 적 표시
                    case 0:
                        gotoxy(enemies[i].x, enemies[i].y, "0_0");
                        break;
                    case 1:
                        gotoxy(enemies[i].x, enemies[i].y, "@_@");
                        break;
                    case 2:
                        gotoxy(enemies[i].x, enemies[i].y, "$_$");
                        break;
                }
            }
        }
    }

    if (rand() % 10 == 0) spawn_enemy(); // 일정 확률로 적 생성
}

void draw_boss(int x, int y) {

    if(boss.exist == TRUE){
        gotoxy(x, y,   "   ???????   "); // 맨끝
        gotoxy(x, y+1, " ??????????? "); 
        gotoxy(x, y+2, "???   ?   ???"); // 가운데 대포+ 다리 연결부
        gotoxy(x, y+3, "????     ????"); // 다리 연결
        gotoxy(x, y+4, " ??       ?? "); // 다리쪽 대포        
    }
}

void erase_boss(int x, int y) {
    gotoxy(x, y,   "             "); // 맨끝
    gotoxy(x, y+1, "             "); 
    gotoxy(x, y+2, "             "); // 가운데 대포+ 다리 연결부
    gotoxy(x, y+3, "             "); // 다리 연결
    gotoxy(x, y+4, "             "); // 다리쪽 대포
}

void boss_update() { // 11.24 얘 갈아 엎음 진짜 말도 안되게 오래 걸림

    const DWORD BOSS_MOVE_INTERNAL = 100;   // 보스 이동 주기 (100ms)
    const DWORD BOSS_UPDATE_INTERVAL = 10000; // 보스 생성 주기 (10초)
    const DWORD BOSS_FIRE_INTERVAL = 50; //보스 총알 발사주기 (50ms)



    DWORD currentTime = GetTickCount();

    //보스 생성 첫번째가 10초뒤에 실행되기 위함 
            //-> static이여서 게임 시작하는 딱 처음에만 0임
    if (for_bossUpdate == 0) {
        for_bossUpdate = currentTime; // 게임 시작 시점으로 초기화
    }

    // 보스 생성 조건 확인
    //만약 보스가 없고, 현재시간 - 보스 업데이트를 위한 시간 >= 10초 
    if (!boss.exist && (currentTime - for_bossUpdate >= BOSS_UPDATE_INTERVAL)) {
        for_bossUpdate = currentTime;  // 보스 생성 시간 갱신
        boss.exist = TRUE;             // 보스 활성화
        boss.origin_life += 100;       // 보스 체력 증가
        boss.life = boss.origin_life;  // 보스 체력 설정
        boss.x = (MAP_X + MAP_WIDTH) / 2 - 1; // 보스 초기 위치 설정
        boss.y = MAP_Y + 1;
        draw_boss(boss.x, boss.y);     // 보스 그리기
        gotoxy(25, 5, "BOSS SPAWNED  ");
    }

    // 보스 이동 조건 확인
    if (boss.exist && (currentTime - lastBossUpdate >= BOSS_MOVE_INTERNAL)) {
        lastBossUpdate = currentTime;

        erase_boss(boss.x, boss.y);

        // 경계에 도달했을 때 방향 전환
        if (boss.x <= MAP_X + 2) {
            boss.direction = 1; // 오른쪽으로 방향 변경
        } else if (boss.x >= MAP_X + MAP_WIDTH - 8) {
            boss.direction = 0; // 왼쪽으로 방향 변경
        }

        // 방향에 따라 이동
        switch (boss.direction) {
            case 0: boss.x--; break;  // 왼쪽 이동
            case 1: boss.x++; break;  // 오른쪽 이동
        }

        draw_boss(boss.x, boss.y);

        // 보스 체력 확인
        if (boss.life <= 0) {
            boss.exist = FALSE; // 보스 제거
            erase_boss(boss.x, boss.y);
            boss.x = (MAP_X + MAP_WIDTH) / 2 - 1; // 보스 초기 위치로 복귀
            boss.y = MAP_Y + 1;
            boss.life = 0;            // 체력 초기화
            boss.direction = 0;       // 방향 초기화
            gotoxy(25, 5, "BOSS DEFEATED  ");
            for_bossUpdate = currentTime; // 보스 생성 시간 갱신

            for(int i = 0; i<BOSSBullet; i++){
                boss_bullets[i].exist = FALSE;
                gotoxy(boss_bullets[i].x, boss_bullets[i].y, " ");
                
            }
        }
    }

    if(boss.exist && currentTime - lastBossFire >= BOSS_FIRE_INTERVAL){
        lastBossFire = currentTime;
        boss_fire_bullet();
    }

    boss_update_bullet();

}

// 총알과 적의 충돌 검사 함수
void check_collision() {
    //히어로의 총알과 적의 총알 충돌 검사 
    for (int i = 0; i < MAXBullet; i++) {
        if (bullets[i].exist) {
            // 쫄병 충돌 검사
            for (int j = 0; j < MAXEnemies; j++) {
                if (enemies[j].exist) {
                    // 적의 위치와 크기를 고려하여 충돌 검사
                    if ((bullets[i].x >= enemies[j].x && bullets[i].x <= enemies[j].x + 3) && 
                        (bullets[i].y >= enemies[j].y && bullets[i].y <= enemies[j].y + 1)) {
                        bullets[i].exist = FALSE;            // 충돌 시 총알 제거
                        gotoxy(bullets[i].x, bullets[i].y, " "); //총알 지워                         
                        gotoxy(enemies[j].x, enemies[j].y, "   ");  // 충돌한 적 위치를 비우기 (화면에서 제거)                        
                        
                        enemies[j].exist = FALSE;            // 충돌 시 적 제거
                        player[player_i].score += 10;        // 점수 추가
                        gotoxy(40, 17, "score: ");
                        printf("%d", player[player_i].score);
                        break;
                    }
                }
            }

            // 보스 충돌 검사
            if (boss.exist &&
                (bullets[i].x >= boss.x + 3 && bullets[i].x <= boss.x + 9) &&
                (bullets[i].y >= boss.y && bullets[i].y <= boss.y + 3)) {
                bullets[i].exist = FALSE;
                gotoxy(bullets[i].x, bullets[i].y, " "); // 총알 지우기
                player[player_i].score += 20;           // 점수 증가
                boss.life -= 10;                        // 보스 체력 감소
                gotoxy(40, 17, "score: ");
                printf("%d", player[player_i].score);
                gotoxy(25, 5, "BOSS LIFE: ");
                printf("%d  ", boss.life);
            }
        }
    }

    // 히어로와 보스 총알 충돌 검사
    for (int k = 0; k < BOSSBullet; k++) {
        if (boss_bullets[k].exist &&
            (boss_bullets[k].x >= hero.x && boss_bullets[k].x <= hero.x + 1) &&
            (boss_bullets[k].y >= hero.y && boss_bullets[k].y <= hero.y + 1)) {
            hero_lives--;                       // 히어로 목숨 감소
            print_lives();
            boss_bullets[k].exist = FALSE;        // 보스 총알 제거
            gotoxy(boss_bullets[k].x, boss_bullets[k].y, " "); // 총알 지우기

            if (hero_lives <= 0) {
                game_over();  // 게임 오버 처리
                return;
            }

            break; // 충돌처리 후 루프 탈출 시켜서 중복으로 체력깎이는거 방지
        }
    }

    //아이템과 히어로 충돌 검사
    for (int l = 0; l < MAXItem; l++) {
        if (item[l].exist &&
            (item[l].x >= hero.x && item[l].x <= hero.x + 1) &&
            (item[l].y >= hero.y && item[l].y <= hero.y + 1)) {
            
            if(item[l].type == 0){
                hero_lives++;
                print_lives();
            }
            else if(item[l].type == 1){
                for(int b_count = 0; b_count < MAXBullet; b_count++){
                    if(bullets[b_count].active == FALSE){ //활동 안하는 총알 만나자마자 활동시키고 break;
                        bullets[b_count].active = TRUE;
                        break; 
                    }
                }
            }
            item[l].active = FALSE;
            item[l].exist = FALSE;                // 아이템 제거
            gotoxy(item[l].x, item[l].y, " ");    // 아이템 지우기
            gotoxy(25, 6, "            ");
            break;
        }
    }
    
    // 히어로와 적 충돌 검사
    for (int j = 0; j < MAXEnemies; j++) {
        if (enemies[j].exist &&
            (enemies[j].x >= hero.x && enemies[j].x <= hero.x + 3) &&
            (enemies[j].y >= hero.y && enemies[j].y <= hero.y + 1)) {
            hero_lives--;                       // 히어로 목숨 감소
            print_lives();
            enemies[j].exist = FALSE;          // 충돌한 적 제거
            gotoxy(enemies[j].x, enemies[j].y, " "); // 적 지우기

            if (hero_lives <= 0) {
                game_over();  // 게임 오버 처리
                return;
            }
            break; // 충돌처리 후 루프 탈출
        }
    }

}

// 적의 생성 및 초기화
void spawn_item() {
    for (int i = 0; i < MAXItem; i++) {
        if (item[i].active && !item[i].exist) { // 빈 적 위치에 생성
            item[i].x = MAP_X + 1 + rand() % (MAP_WIDTH - 2); // x 위치 랜덤 설정
            item[i].y = MAP_Y + 1;
            item[i].type = rand() % 2;
            item[i].exist = TRUE;
            break;
        }
    }
}
// 적의 위치 업데이트 및 화면 표시
void update_item() {
    for (int i = 0; i < MAXItem; i++) {
        if (item[i].active && item[i].exist) {
            // 기존 위치를 지우기
            gotoxy(item[i].x, item[i].y, " "); // 기존 위치 지우기

            // 아이템 이동
            item[i].y++;

            // 적의 y 좌표도 맵 경계를 넘는지 확인
            if (item[i].y >= MAP_Y + MAP_HEIGHT) {
                // 하단 경계에 도달했을 경우 아이템 제거
                item[i].exist = FALSE;
                item[i].active = FALSE;
                gotoxy(25, 6, "            ");
            }
            else {
                // 새로운 위치에 아이템 표시
                switch (item[i].type) { // 타입별 아이템 표시
                    case 0:
                        gotoxy(item[i].x, item[i].y, "♥");
                        break;
                    case 1:
                        gotoxy(item[i].x, item[i].y, "♠");
                        break;
                }
            }
        }
    }

    if (rand() % 300 == 0) { // 1/300 확률로 아이템 나옴 ㅇㅇ
        spawn_item(); // 일정 확률로 아이템 생성
        for(int i = 0; i < MAXItem; i++){
            if(item[i].active == TRUE){ //활동 안하는 총알 만나자마자 활동시키고 break;
                gotoxy(25, 6, "item spawned");
                break; 
            }
        }
    }
}

void print_lives(){
    gotoxy(40, 19, "Lives: ");
    printf("                              ");
    gotoxy(44,19, "");    
    for (int j = 0; j < hero_lives; j++) {
        printf("♥ ");
    }
}

// 점수를 추가하는 함수
void add_score(const char* nickname, int score) {
    ScoreNode* new_node = (ScoreNode*)malloc(sizeof(ScoreNode));
    strncpy(new_node->nickname, nickname, 19);
    new_node->nickname[19] = '\0'; // null terminator
    new_node->score = score;
    new_node->next = head; // 새 노드를 리스트의 맨 앞에 추가
    head = new_node;
}
// 점수 정렬 함수 (내림차순)
void sort_scores() {
    if (head == NULL) return;

    ScoreNode* current;
    ScoreNode* next;
    int temp_score;
    char temp_nickname[20];

    for (current = head; current->next != NULL; current = current->next) {
        for (next = current->next; next != NULL; next = next->next) {
            if (current->score < next->score) {
                // Swap scores
                temp_score = current->score;
                current->score = next->score;
                next->score = temp_score;
                // Swap nicknames
                strncpy(temp_nickname, current->nickname, 20);
                strncpy(current->nickname, next->nickname, 20);
                strncpy(next->nickname, temp_nickname, 20);
            }
        }
    }
}

// 점수 출력 함수
void print_top_scores() {
    ScoreNode* current = head;
    int rank = 1;

    gotoxy(10, 10, "Top Scores:");
    while (current != NULL && rank <= 10) {
        gotoxy(10, 10 + rank, ""); // 커서 위치 이동
        printf("%d. %s: %d", rank, current->nickname, current->score); // 점수 출력
        current = current->next;
        rank++;
    }
}
// 점수를 파일에 저장하는 함수
void save_scores_to_file() {
    FILE* file = fopen("score.txt", "w");
    if (file == NULL) {
        printf("Error opening file for writing.\n");
        return;
    }
    ScoreNode* current = head;
    int rank = 1;
    while (current != NULL && rank <= 10) {
        fprintf(file, "%s %d\n", current->nickname, current->score);
        current = current->next;
        rank++;
    }
    fclose(file);
}
void load_scores_from_file() {
    FILE* file = fopen("score.txt", "r");
    if (file == NULL) {
        // 파일이 없으면 그냥 리턴
        return;
    }

    char nickname[20];
    int score;

    while (fscanf(file, "%s %d", nickname, &score) == 2) {
        add_score(nickname, score);
    }

    fclose(file);
}
// 점수 리스트 메모리 해제 함수
void free_score_list() {
    ScoreNode* current = head;
    while (current != NULL) {
        ScoreNode* temp = current;
        current = current->next;
        free(temp);
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
    gotoxy(4, 18, "아이템은 딱 5개만 나옵니다 집중하세요!");
    gotoxy(8, 20, "nickname : ");
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
    gotoxy(40, 7, "<Space> : shoot!");
    gotoxy(40, 8, "<Esc> : exit");
    
    gotoxy(42, 10, "<아이템>");
    gotoxy(39, 11, "♥ : life   ♠ : bullet");
    
    gotoxy(42, 13,"<STATUS>");
    gotoxy(40, 15, "nickname: ");
    printf("%s", nickname);
    gotoxy(40, 17, "score: ");
    printf("%d", score);
    print_lives();
}

void game_over() {
    system("cls");

    add_score(player[player_i].nickname, player[player_i].score);
    sort_scores(); // 점수 정렬

    gotoxy(10, 5, "*************************");
    gotoxy(10, 6, "*       GAME OVER       *");
    gotoxy(10, 7, "*************************");
    gotoxy(10, 8, "Press Any Key to restart");
    gotoxy(10, 9, "or Esc to exit");
    
    print_top_scores(); // 상위 10점수 출력
    save_scores_to_file(); // 점수를 파일에 저장

    char ch = _getch();
    
    if (ch == 27) {  // Esc 키로 게임 종료
        exit(0);
    }
    else{
        game_reset();
        move_hero();        
    }


}


void game_reset() {
    // 히어로 상태 초기화
    hero.x = 13;
    hero.y = 35;
    hero_lives = 3;

    // 플레이어 점수 초기화
    player[player_i].score = 0;

    // 총알 초기화
    for (int i = 0; i < MAXBullet; i++) {
        if(i<3)
            bullets[i].active = TRUE; // 처음 4발 활성화
        else
            bullets[i].active = FALSE;

        bullets[i].exist = FALSE;
        bullets[i].x = 0;
        bullets[i].y = 0;
    }

    // 적 초기화
    for (int i = 0; i < MAXEnemies; i++) {
        enemies[i].exist = FALSE;
        enemies[i].x = 0;
        enemies[i].y = 0;
    }

    // 보스 초기화
    boss.exist = FALSE;
    boss.x = (MAP_X + MAP_WIDTH) / 2 - 1;
    boss.y = MAP_Y + 1;
    boss.origin_life = 0;
    boss.life = 0;
    boss.direction = 0;

    // 보스 총알 초기화
    for (int i = 0; i < BOSSBullet; i++) {
        boss_bullets[i].exist = FALSE;
        boss_bullets[i].x = 0;
        boss_bullets[i].y = 0;
    }

    //보스 시간 초기화 -> 시간관계상 더 깔끔하게 처리하고 싶었는데 못함
    lastBossUpdate = 0;
    for_bossUpdate = 0;
    lastBossFire = 0;

    // 아이템 초기화
    for (int i = 0; i < MAXItem; i++) {
        item[i].active = TRUE;
        item[i].exist = FALSE;
        item[i].x = 0;
        item[i].y = 0;
        item[i].type = 0;
    }

    // 인터페이스와 맵 초기화
    system("cls");
    inter_face();
    map(player[player_i].nickname, player[player_i].score, hero_lives);
}