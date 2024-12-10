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
#define BOSSBullet 6
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
    int exist;
    int x;
    int y;
}Bullet;

typedef struct {
    int exist;
    int x;
    int y;
    int type; //적의 모양
    int stop; // 적이 멈췄는지 여부를 나타내는 변수
    int direction; // 이동 방향 추가 (0: 아래, 1: 왼쪽 대각선, 2: 오른쪽 대각선)
    DWORD stop_time;  // 적이 멈춘 시간
    DWORD stop_duration; // 적이 멈추는 시간
    int original_direction;  // 추가: 원래 이동 방향
} Enemy;

typedef struct {
    int exist;
    int x;
    int y;
    int origin_life;
    int life;
    int direction;
}Boss;

Hero hero = {13,35};
Bullet bullets[MAXBullet] = {{FALSE, 0, 0}, {FALSE, 0, 0}, {FALSE, 0, 0}, {FALSE, 0, 0}, {FALSE, 0, 0}};
Enemy enemies[MAXEnemies] = {{FALSE, 0, 0, 0}};
Boss boss = {FALSE, (MAP_X + MAP_WIDTH)/2 - 1, MAP_Y +1, 0, 0, 0};
Bullet boss_bullets[BOSSBullet] = {{FALSE, 0, 0}, {FALSE, 0, 0}, {FALSE, 0, 0}, {FALSE, 0, 0}, {FALSE, 0, 0}, {FALSE, 0, 0}};

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
void draw_boss(int, int);
void erase_boss();
void boss_update();
void check_collision();
void game_over();
void boss_fire_bullet();
void boss_update_bullet();

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
    const DWORD BULLET_UPDATE_INTERVAL = 100; // 총알 업데이트 주기 (100ms)


    DWORD lastHeroUpdate = GetTickCount();   // 히어로 입력 마지막 업데이트 시간
    DWORD lastEnemyUpdate = GetTickCount();  // 적 업데이트 마지막 업데이트 시간
    DWORD lastBulletUpdate = GetTickCount(); // 총알 업데이트를 위한 시간
    
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
            update_enemy(); 
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

// ���� ���� �� �ʱ�ȭ
void spawn_enemy() {
    for (int i = 0; i < MAXEnemies; i++) {
        if (!enemies[i].exist) {                 // �� �� ��ġ�� ����
            enemies[i].x = MAP_X + 1 + rand() % (MAP_WIDTH - 2);  // x ��ġ ���� ����
            enemies[i].y = MAP_Y + 1;
            enemies[i].exist = TRUE;
            enemies[i].stop = FALSE; // ���� ó�� �����Ǹ� ������ ����
            enemies[i].stop_duration = 0; // ���� �ð� �ʱ�ȭ
            enemies[i].direction = rand() % 3; //0:�Ʒ�, 1:�� �밢 2: ���� �밢
            enemies[i].type = rand() % 3;   // ���� Ÿ�� ������ ���� (0, 1, �Ǵ� 2)
            break;
        }
    }
}

// ���� ��ġ ������Ʈ �� ȭ�� ǥ��
void update_enemy() {
    DWORD currentTime = GetTickCount(); // 현재 시간 저장

    for (int i = 0; i < MAXEnemies; i++) {
        if (enemies[i].exist) {
            gotoxy(enemies[i].x, enemies[i].y, " "); // ���� ��ġ �����

            // ���� ���� ����ٸ�
            if (enemies[i].stop) {
                // ���� �ð��� �����ٸ� �̵� �簳
                if (currentTime - enemies[i].stop_time >= enemies[i].stop_duration) {
                    enemies[i].stop = FALSE; // ���� ����
                }
            }

            if (!enemies[i].stop) {
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

                    // ���� �߰� �������� ���߱�
                    if (enemies[i].y >= (MAP_Y + MAP_HEIGHT / 2) && !enemies[i].stop) {
                        enemies[i].stop = TRUE; // ���� ����
                        enemies[i].stop_time = currentTime; // ���� �ð� ���
                        enemies[i].stop_duration = (rand() % 3 + 3) * 1000; // 3�ʿ��� 5�� ���� ���� �ð�
                    }

                    switch (enemies[i].type) { // 타입별 적 표시
                        case 0:
                            gotoxy(enemies[i].x, enemies[i].y, "@");
                            break;
                        case 1:
                            gotoxy(enemies[i].x, enemies[i].y, "#");
                            break;
                        case 2:
                            gotoxy(enemies[i].x, enemies[i].y, "&");
                            break;
                    }
                }
            }
        }
    }

    if (rand() % 10 == 0) spawn_enemy(); // 일정 확률로 적 생성
}

void draw_boss(int x, int y) {

    if(boss.exist == TRUE){
        gotoxy(x, y,   "   ███████   "); // 맨끝
        gotoxy(x, y+1, " ███████████ "); 
        gotoxy(x, y+2, "███   █   ███"); // 가운데 대포+ 다리 연결부
        gotoxy(x, y+3, "████     ████"); // 다리 연결
        gotoxy(x, y+4, " ██       ██ "); // 다리쪽 대포    
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

    static DWORD lastBossUpdate = 0; // 보스 이동 시간 관리
    static DWORD for_bossUpdate = 0; // 마지막으로 보스 생성된 시점
    static DWORD lastBossFire = 0; //마지막으로 총알 발사한 시점


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
                fire_bullet(); //총알 전체 초기화
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
    for (int i = 0; i < MAXBullet; i++) {
        if (bullets[i].exist) {
            // 쫄병 충돌 검사
            for (int j = 0; j < MAXEnemies; j++) {
                if (enemies[j].exist &&
                    bullets[i].x == enemies[j].x &&
                    bullets[i].y == enemies[j].y) {
                    bullets[i].exist = FALSE;            // 충돌 시 총알 제거
                    enemies[j].exist = FALSE;            // 충돌 시 적 제거
                    player[player_i].score += 10;        // 점수 추가
                    gotoxy(40, 17, "score: ");
                    printf("%d", player[player_i].score);
                    gotoxy(enemies[j].x, enemies[j].y, "   ");  // 충돌한 적 위치를 비우기 (화면에서 제거)
                    break;
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
            hero_lives--;                         // 히어로 목숨 감소
            boss_bullets[k].exist = FALSE;        // 보스 총알 제거
            gotoxy(boss_bullets[k].x, boss_bullets[k].y, " "); // 총알 지우기

            if (hero_lives <= 0) {
                game_over();  // 게임 오버 처리
                return;
            }

            break;//충돌처리 후 루프 탈출 시켜서 중복으로 체력깎이는거 방지
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