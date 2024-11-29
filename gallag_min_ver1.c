#include<stdio.h>
#include<stdlib.h>  //rand()
#include<time.h>    // rand�� �ʱ�ȭ
#include<windows.h> // gotoxy
#include<conio.h> // �ܼ� ����� getch()

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
    char nickname[20];        //nickname�̶� score ���� ���� ����ü ����
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
    int type; //���� ���
    int stop; // ���� ������� ���θ� ��Ÿ���� ����
    int direction; // �̵� ���� �߰� (0: �Ʒ�, 1: ���� �밢��, 2: ������ �밢��)
    DWORD stop_time;  // ���� ���� �ð�
    DWORD stop_duration; // ���� ���ߴ� �ð�
    int original_direction;  // �߰�: ���� �̵� ����
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

Player player[11];          //�߰� �Ǵ� ����� �׻� 11��° �ε����� ��ġ�ϰ� ���� (������ ����� 10������ �ۿ� �ȵǴ�)
int player_i = 0;            //nickname�� i�� ���� ��������
int hero_lives = 3;  // ������� ���

void removeCursor(void) //11/12 Ŀ������� �Լ� (�ۿ�)
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
    srand(time(NULL));  // ������ ���� ���� ���� �õ� �ʱ�ȭ
    removeCursor();
    inter_face();
    map(player[player_i].nickname, player[player_i].score, hero_lives);// �� �� ����� ��ȯ

    move_hero();
    return 0;
}

void draw_hero(int x, int y){ //x4~x22, y35,36 �ȿ����� ������
    gotoxy(x,y," �� ");
    gotoxy(x,y+1,"���ࢺ");
}

void clear_hero(int x, int y){
    gotoxy(x,y,"   ");
    gotoxy(x,y+1,"   ");
}

void move_hero() {
    const DWORD HERO_UPDATE_INTERVAL = 50;   // ����� �Է� ������Ʈ �ֱ� (50ms)
    const DWORD ENEMY_UPDATE_INTERVAL = 200; // �� ������Ʈ �ֱ� (200ms)
    const DWORD BULLET_UPDATE_INTERVAL = 100; // �Ѿ� ������Ʈ �ֱ� (100ms)


    DWORD lastHeroUpdate = GetTickCount();   // ����� �Է� ������ ������Ʈ �ð�
    DWORD lastEnemyUpdate = GetTickCount();  // �� ������Ʈ ������ ������Ʈ �ð�
    DWORD lastBulletUpdate = GetTickCount(); // �Ѿ� ������Ʈ�� ���� �ð�
    
    char ch;

    int game_running = 1;

    while (game_running) {
        DWORD currentTime = GetTickCount();

        // **����� �Է� ó�� (50ms ����)**
        if (currentTime - lastHeroUpdate >= HERO_UPDATE_INTERVAL) {
            lastHeroUpdate = currentTime;

            // ����� �Է� ó��
            if (_kbhit()) { // Ű �Է� ���
                ch = _getch();
                clear_hero(hero.x, hero.y);

                switch (ch) {
                case 75: // �� Ű
                    if (hero.x > MAP_X + 1)
                        hero.x--;
                    break;
                case 77: // �� Ű
                    if (hero.x < MAP_X + MAP_WIDTH - 2)
                        hero.x++;
                    break;
                case 32: // �����̽��� (�Ѿ� �߻�)
                    fire_bullet();
                    break;
                case 27: // Esc Ű (���� ����)
                    game_running = 0;
                    break;
                }
                draw_hero(hero.x, hero.y);
            }
        }

        // **�� �� ���� ���� ������Ʈ (200ms ����)**
        if (currentTime - lastEnemyUpdate >= ENEMY_UPDATE_INTERVAL) {
            lastEnemyUpdate = currentTime;
            check_collision();
            update_enemy(); 
        }

        //���� ���� �׳� �ϳ��� ���� -> ������ �ʹ� ���Ƽ� ���� �������
        boss_update();

        // �Ѿ� �̵�
        if (currentTime - lastBulletUpdate >= BULLET_UPDATE_INTERVAL) {
            lastBulletUpdate = currentTime;
            update_bullet();
        }

        // ����� ���� Ȯ��
        if (hero_lives <= 0) {
            game_over();

            // ���� ���� �Ǵ� ����� ���� Ȯ��
            char choice = _getch();
            if (choice == 'e') {
                exit(0);
            }

            // ���� �ʱ�ȭ
            hero_lives = 3;
            player[player_i].score = 0;
            spawn_enemy();
            inter_face();
            map(player[player_i].nickname, player[player_i].score, hero_lives);
            continue;
        }
        
        // ���� ��� ǥ��
        gotoxy(40, 19, "Lives: ");
        for (int i = 0; i < hero_lives; i++) {
            printf("�� ");
        }
        for (int i = hero_lives; i < 3; i++) {
            printf("  ");
        }
    }
}
void fire_bullet(){ //�����ܰ��� �����ϸ� ��
    for(int i = 0; i<MAXBullet; i++){
        if(!bullets[i].exist){ //exist�� true��� �Ѿ� �غ�
            bullets[i].x = hero.x + 1;
            bullets[i].y = hero.y - 1;
            bullets[i].exist = TRUE;
            break;
        }
    }
}

void update_bullet(){
    for(int i = 0; i<MAXBullet; i++){
        if(bullets[i].exist){//�Ҹ��� �����ϸ�
            gotoxy(bullets[i].x, bullets[i].y, " "); //���� �ִ� �ڸ��� ������
            bullets[i].y--;

            if(bullets[i].y <= MAP_Y) //�ʿ��� �Ѿ��� ����� ��������
                bullets[i].exist = FALSE;
            else
                gotoxy(bullets[i].x, bullets[i].y, "��"); //�ƴϸ� �Ѿ� ������ ��ĭ ����
        }
    }
}

void boss_fire_bullet(){ //���� �ڵ� ���� �غ����� // �����ܰ�
    for(int i = 0; i<BOSSBullet; i++){
        if(!boss_bullets[i].exist){ //exist�� true��� �Ѿ� �غ�

            if(i % 3 == 0){ // ���� ����
                boss_bullets[i].x = boss.x + 1;
                boss_bullets[i].y = boss.y + 5;
            }
            else if(i % 3 == 1){// ��� ����
                boss_bullets[i].x = boss.x + 4;
                boss_bullets[i].y = boss.y + 5;
            }
            else if(i % 3 == 2){// �����ʴ���
                boss_bullets[i].x = boss.x + 7;
                boss_bullets[i].y = boss.y + 5;
            }

            boss_bullets[i].exist = TRUE;
            if(i==2)   // �ѹ� �극��ũ�� �ɾ���� ���ÿ� 3���� ������ �� ȭ�鿡 6�߱��� ����
                break;
        }
    }
}

void boss_update_bullet(){
    const DWORD BOSS_BULLET_UPDATE_INTERVAL = 100; //�Ѿ� �̵��ֱ�(200ms);
    static DWORD lastBossBulletUpdate = 0; //�Ѿ� �̵�����

    DWORD currentTime = GetTickCount();

    if (currentTime - lastBossBulletUpdate < BOSS_BULLET_UPDATE_INTERVAL) {
        return; // �Ѿ� �̵� �ֱⰡ ���� �������� �ʾ����� ��ȯ
    }
    lastBossBulletUpdate = currentTime;

    for(int i = 0; i<BOSSBullet; i++){
        if(boss_bullets[i].exist){//�Ҹ��� �����ϸ�
            gotoxy(boss_bullets[i].x, boss_bullets[i].y, " "); //���� �ִ� �ڸ��� ������
            boss_bullets[i].y++;

            if(boss_bullets[i].y >= MAP_Y+MAP_HEIGHT) //�ʿ��� �Ѿ��� ����� ��������
                boss_bullets[i].exist = FALSE;
            else
                gotoxy(boss_bullets[i].x, boss_bullets[i].y, "|"); //�ƴϸ� �Ѿ� ������ ��ĭ ����
        }
    }
}

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
    DWORD currentTime = GetTickCount(); // ���� �ð� ����

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
                // �� �̵�
                int new_x = enemies[i].x;
                int new_y = enemies[i].y;

                switch (enemies[i].direction) {
                    case 0: // �Ʒ���
                        new_y++;
                        break;
                    case 1: // ���� �밢�� �Ʒ���
                        new_x--;
                        new_y++;
                        break;
                    case 2: // ������ �밢�� �Ʒ���
                        new_x++;
                        new_y++;
                        break;
                }

                // ��� �˻�: ���ο� ��ġ�� ���� ��踦 ���� �ʴ��� Ȯ��
                if (new_x < MAP_X + 1) {
                    // ���� ��迡 �������� ��� ���������� ���� ��ȯ
                    enemies[i].direction = 2; // ������ �밢������ ����
                    new_x = MAP_X + 1; // ��迡 ���߱�
                } else if (new_x >= MAP_X + MAP_WIDTH - 1) {
                    // ������ ��迡 �������� ��� �������� ���� ��ȯ
                    enemies[i].direction = 1; // ���� �밢������ ����
                    new_x = MAP_X + MAP_WIDTH - 2; // ��迡 ���߱�
                }

                // ���� y ��ǥ�� �� ��踦 �Ѵ��� Ȯ��
                if (new_y >= MAP_Y + MAP_HEIGHT) {
                    // �ϴ� ��迡 �������� ��� �� ����
                    enemies[i].exist = FALSE;
                } else {
                    enemies[i].x = new_x; // ��ȿ�� ��ġ�� ������Ʈ
                    enemies[i].y = new_y;

                    // ���� �߰� �������� ���߱�
                    if (enemies[i].y >= (MAP_Y + MAP_HEIGHT / 2) && !enemies[i].stop) {
                        enemies[i].stop = TRUE; // ���� ����
                        enemies[i].stop_time = currentTime; // ���� �ð� ���
                        enemies[i].stop_duration = (rand() % 3 + 3) * 1000; // 3�ʿ��� 5�� ���� ���� �ð�
                    }

                    switch (enemies[i].type) { // Ÿ�Ժ� �� ǥ��
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

    if (rand() % 10 == 0) spawn_enemy(); // ���� Ȯ���� �� ����
}

void draw_boss(int x, int y) {

    if(boss.exist == TRUE){
        gotoxy(x, y,   "   ???????   "); // �ǳ�
        gotoxy(x, y+1, " ??????????? "); 
        gotoxy(x, y+2, "???   ?   ???"); // ��� ����+ �ٸ� �����
        gotoxy(x, y+3, "????     ????"); // �ٸ� ����
        gotoxy(x, y+4, " ??       ?? "); // �ٸ��� ����        
    }
}

void erase_boss(int x, int y) {
    gotoxy(x, y,   "             "); // �ǳ�
    gotoxy(x, y+1, "             "); 
    gotoxy(x, y+2, "             "); // ��� ����+ �ٸ� �����
    gotoxy(x, y+3, "             "); // �ٸ� ����
    gotoxy(x, y+4, "             "); // �ٸ��� ����
}

void boss_update() { // 11.24 �� ���� ���� ��¥ ���� �ȵǰ� ���� �ɸ�

    const DWORD BOSS_MOVE_INTERNAL = 100;   // ���� �̵� �ֱ� (100ms)
    const DWORD BOSS_UPDATE_INTERVAL = 10000; // ���� ���� �ֱ� (10��)
    const DWORD BOSS_FIRE_INTERVAL = 50; //���� �Ѿ� �߻��ֱ� (50ms)

    static DWORD lastBossUpdate = 0; // ���� �̵� �ð� ����
    static DWORD for_bossUpdate = 0; // ���������� ���� ������ ����
    static DWORD lastBossFire = 0; //���������� �Ѿ� �߻��� ����


    DWORD currentTime = GetTickCount();

    //���� ���� ù��°�� 10�ʵڿ� ����Ǳ� ���� 
            //-> static�̿��� ���� �����ϴ� �� ó������ 0��
    if (for_bossUpdate == 0) {
        for_bossUpdate = currentTime; // ���� ���� �������� �ʱ�ȭ
    }

    // ���� ���� ���� Ȯ��
    //���� ������ ����, ����ð� - ���� ������Ʈ�� ���� �ð� >= 10�� 
    if (!boss.exist && (currentTime - for_bossUpdate >= BOSS_UPDATE_INTERVAL)) {
        for_bossUpdate = currentTime;  // ���� ���� �ð� ����
        boss.exist = TRUE;             // ���� Ȱ��ȭ
        boss.origin_life += 100;       // ���� ü�� ����
        boss.life = boss.origin_life;  // ���� ü�� ����
        boss.x = (MAP_X + MAP_WIDTH) / 2 - 1; // ���� �ʱ� ��ġ ����
        boss.y = MAP_Y + 1;
        draw_boss(boss.x, boss.y);     // ���� �׸���
        gotoxy(25, 5, "BOSS SPAWNED  ");
    }

    // ���� �̵� ���� Ȯ��
    if (boss.exist && (currentTime - lastBossUpdate >= BOSS_MOVE_INTERNAL)) {
        lastBossUpdate = currentTime;

        erase_boss(boss.x, boss.y);

        // ��迡 �������� �� ���� ��ȯ
        if (boss.x <= MAP_X + 2) {
            boss.direction = 1; // ���������� ���� ����
        } else if (boss.x >= MAP_X + MAP_WIDTH - 8) {
            boss.direction = 0; // �������� ���� ����
        }

        // ���⿡ ���� �̵�
        switch (boss.direction) {
            case 0: boss.x--; break;  // ���� �̵�
            case 1: boss.x++; break;  // ������ �̵�
        }

        draw_boss(boss.x, boss.y);

        // ���� ü�� Ȯ��
        if (boss.life <= 0) {
            boss.exist = FALSE; // ���� ����
            erase_boss(boss.x, boss.y);
            boss.x = (MAP_X + MAP_WIDTH) / 2 - 1; // ���� �ʱ� ��ġ�� ����
            boss.y = MAP_Y + 1;
            boss.life = 0;            // ü�� �ʱ�ȭ
            boss.direction = 0;       // ���� �ʱ�ȭ
            gotoxy(25, 5, "BOSS DEFEATED  ");
            for_bossUpdate = currentTime; // ���� ���� �ð� ����

            for(int i = 0; i<BOSSBullet; i++){
                boss_bullets[i].exist = FALSE;
                gotoxy(boss_bullets[i].x, boss_bullets[i].y, " ");
                fire_bullet(); //�Ѿ� ��ü �ʱ�ȭ
            }
        }
    }

    if(boss.exist && currentTime - lastBossFire >= BOSS_FIRE_INTERVAL){
        lastBossFire = currentTime;
        boss_fire_bullet();
    }

    boss_update_bullet();

}


// �Ѿ˰� ���� �浹 �˻� �Լ�
void check_collision() {
    for (int i = 0; i < MAXBullet; i++) {
        if (bullets[i].exist) {
            // �̺� �浹 �˻�
            for (int j = 0; j < MAXEnemies; j++) {
                if (enemies[j].exist &&
                    bullets[i].x == enemies[j].x &&
                    bullets[i].y == enemies[j].y) {
                    bullets[i].exist = FALSE;            // �浹 �� �Ѿ� ����
                    enemies[j].exist = FALSE;            // �浹 �� �� ����
                    player[player_i].score += 10;        // ���� �߰�
                    gotoxy(40, 17, "score: ");
                    printf("%d", player[player_i].score);
                    gotoxy(enemies[j].x, enemies[j].y, "   ");  // �浹�� �� ��ġ�� ���� (ȭ�鿡�� ����)
                    break;
                }
            }

            // ���� �浹 �˻�
            if (boss.exist &&
                (bullets[i].x >= boss.x + 3 && bullets[i].x <= boss.x + 9) &&
                (bullets[i].y >= boss.y && bullets[i].y <= boss.y + 3)) {
                bullets[i].exist = FALSE;
                gotoxy(bullets[i].x, bullets[i].y, " "); // �Ѿ� �����
                player[player_i].score += 20;           // ���� ����
                boss.life -= 10;                        // ���� ü�� ����
                gotoxy(40, 17, "score: ");
                printf("%d", player[player_i].score);
                gotoxy(25, 5, "BOSS LIFE: ");
                printf("%d  ", boss.life);
            }
        }
    }

    // ����ο� ���� �Ѿ� �浹 �˻�
    for (int k = 0; k < BOSSBullet; k++) {
        if (boss_bullets[k].exist &&
            (boss_bullets[k].x >= hero.x && boss_bullets[k].x <= hero.x + 1) &&
            (boss_bullets[k].y >= hero.y && boss_bullets[k].y <= hero.y + 1)) {
            hero_lives--;                         // ����� ��� ����
            boss_bullets[k].exist = FALSE;        // ���� �Ѿ� ����
            gotoxy(boss_bullets[k].x, boss_bullets[k].y, " "); // �Ѿ� �����

            if (hero_lives <= 0) {
                game_over();  // ���� ���� ó��
                return;
            }

            break;//�浹ó�� �� ���� Ż�� ���Ѽ� �ߺ����� ü�±��̴°� ����
        }
    }
}


void inter_face() {
    system("cls");
    int i, j;

    for (i = MAP_X; i <= MAP_WIDTH + MAP_X; i++) {
        gotoxy(i, MAP_Y, "��");
    }
    for (j = MAP_Y + 1; j <= MAP_HEIGHT + MAP_Y; j++) {
        gotoxy(MAP_X, j, "��");
        gotoxy(MAP_X + MAP_WIDTH, j, "��");
    }
    for (i = MAP_X; i <= MAP_WIDTH + MAP_X; i++) {
        gotoxy(i, MAP_Y + MAP_HEIGHT, "��");
    }

    gotoxy(10, 15, "�� ���� ���� ��");
    gotoxy(8, 16, "����Ű�� ���� �մϴ�.");
    gotoxy(5, 17, "�г����� �Է��ϸ� ������ ���۵˴ϴ�.");
    gotoxy(6, 19, "nickname : ");
    scanf("%s", player[player_i].nickname);
    system("cls");
}

void map(char *nickname, int score, int lives) {       //map�Լ��� �Ķ���� �־��༭ gameover �ÿ��� status â ��ü �ʱ�ȭ
    int i, j;

    for (i = MAP_X; i <= MAP_WIDTH + MAP_X; i++) {
        gotoxy(i, MAP_Y, "��");
    }
    for (j = MAP_Y + 1; j <= MAP_HEIGHT + MAP_Y; j++) {
        gotoxy(MAP_X, j, "��");
        gotoxy(MAP_X + MAP_WIDTH, j, "��");
    }
    for (i = MAP_X; i <= MAP_WIDTH + MAP_X; i++) {
        gotoxy(i, MAP_Y + MAP_HEIGHT, "��");
    }

    draw_hero(13,35); //11.06 hero �ʸ��鶧 ���� �׷�����

    gotoxy(42, 4, "<���۹�>");
    gotoxy(40, 6, "<����Ű> : ��, ��");
    gotoxy(40, 7, "<Esc> : exit");
    gotoxy(40, 8, " <P> : pause");
    gotoxy(40, 9, " <e> : real exit game");
    
    gotoxy(42, 13,"<STATUS>");
    gotoxy(40, 15, "nickname: ");
    printf("%s", nickname);
    gotoxy(40, 17, "score: ");
    printf("%d", score); gotoxy(40, 19, "Lives: ");
    for (int i = 0; i < lives; i++) {
        printf("�� ");
    }
}

void game_over() {
    system("cls");
    gotoxy(10, 10, "*************************");
    gotoxy(10, 11, "*       GAME OVER       *");
    gotoxy(10, 12, "*************************");
    gotoxy(10, 14, "Press any key to restart");
   char ch = _getch();  // Ű �Է��� ����Ͽ� ���� �����
     if (ch == 13) {  // Enter Ű�� ���� �����
        hero_lives = 3;
        player[player_i].score = 0;
        spawn_enemy();
        move_hero();
    } else if (ch == 27) {  // Esc Ű�� ���� ����
        exit(0);
    }
}