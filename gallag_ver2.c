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
    char nickname[20];        //nickname�̶� score ���� ���� ����ü ����
} Player;

typedef struct{
    int active; //�굵 false �϶��� �۵� ���� �Ⱥ���
    int exist;
    int x;
    int y;
}Bullet;

typedef struct {
    int exist;
    int x;
    int y;
    int type; //���� ���
    int direction; // �̵� ���� �߰� (0: �Ʒ�, 1: ���� �밢��, 2: ������ �밢��)
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
    int active; // �̰� false�� �Ǹ� �ƿ� �ȳ����� �Ǵ°���
    int exist;
    int x;
    int y;
    int type; //item ����
}Item; 
typedef struct ScoreNode {
    char nickname[20];
    int score;
    struct ScoreNode* next;
} ScoreNode;

ScoreNode* head = NULL; // ����Ʈ�� ��� ������

Hero hero = {13,35};
Bullet bullets[MAXBullet] = {{TRUE, FALSE, 0, 0},{TRUE, FALSE, 0, 0},
        {TRUE, FALSE, 0, 0},{FALSE, FALSE, 0, 0},{FALSE, FALSE, 0, 0},
        {FALSE, FALSE, 0, 0},{FALSE, FALSE, 0, 0}, {FALSE, FALSE, 0, 0},};
Enemy enemies[MAXEnemies] = {{FALSE, 0, 0, 0}};
Boss boss = {FALSE, (MAP_X + MAP_WIDTH)/2 - 1, MAP_Y +1, 0, 0, 0};
Bullet boss_bullets[BOSSBullet] = {{FALSE, 0, 0}, {FALSE, 0, 0}, {FALSE, 0, 0}, {FALSE, 0, 0}, {FALSE, 0, 0}, {FALSE, 0, 0}};
Item item[MAXItem] = {{TRUE, FALSE, 0 , 0 , 0}, {TRUE, FALSE, 0 , 0 , 0}, {TRUE, FALSE, 0 , 0 , 0}, {TRUE, FALSE, 0 , 0 , 0}, {TRUE, FALSE, 0 , 0 , 0}};

Player player[11];        //�߰� �Ǵ� ����� �׻� 11��° �ε����� ��ġ�ϰ� ���� (������ ����� 10������ �ۿ� �ȵǴ�)
int player_i = 0;   //nickname�� i�� ���� ��������
int hero_lives = 3;  // ������� ���

// ������ �ڲ� ���� ��������ڸ��� ���ͼ� ���� ���� 
DWORD lastBossUpdate = 0; // ���� �̵� �ð� ����
DWORD for_bossUpdate = 0; // ���������� ���� ������ ����
DWORD lastBossFire = 0; //���������� �Ѿ� �߻��� ����



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
    srand(time(NULL));  // ������ ���� ���� ���� �õ� �ʱ�ȭ
    removeCursor();
    load_scores_from_file();
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
    const DWORD Item_UPDATE_INTERVAL = 200; // ������ ������Ʈ �ֱ� (100ms)

    DWORD lastHeroUpdate = GetTickCount();   // ����� �Է� ������ ������Ʈ �ð�
    DWORD lastEnemyUpdate = GetTickCount();  // �� ������Ʈ ������ ������Ʈ �ð�
    DWORD lastBulletUpdate = GetTickCount(); // �Ѿ� ������Ʈ�� ���� �ð�
    DWORD lastItemUpdate = GetTickCount(); // ������ ������Ʈ�� ���� �ð�

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
                    game_over();
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
            update_item();
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

            // ���� �ʱ�ȭ �� ���� �ð� �ʱ�ȭ
            game_reset();                
            lastHeroUpdate = GetTickCount();
            lastEnemyUpdate = GetTickCount();
            lastBulletUpdate = GetTickCount();
            lastItemUpdate = GetTickCount();
            continue;
        }
        
    }
}
void fire_bullet(){ //�����ܰ��� �����ϸ� ��
    for(int i = 0; i<MAXBullet; i++){
        if(!(bullets[i].exist) && (bullets[i].active)){ //exist�� false�� active�� true�� �Ѿ� �غ�
            bullets[i].x = hero.x;
            bullets[i].y = hero.y - 1;
            bullets[i].exist = TRUE;
            break;
        }
    }
}

void update_bullet(){
    for(int i = 0; i<MAXBullet; i++){
        if(bullets[i].exist && bullets[i].active){//�Ҹ��� �����ϸ�
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

// ���� ���� �� �ʱ�ȭ
void spawn_enemy() {
    for (int i = 0; i < MAXEnemies; i++) {
        if (!enemies[i].exist) { // �� �� ��ġ�� ����
            enemies[i].x = MAP_X + 1 + rand() % (MAP_WIDTH - 2); // x ��ġ ���� ����
            enemies[i].y = MAP_Y + 1;
            enemies[i].exist = TRUE;
            enemies[i].direction = rand() % 3; // 0: �Ʒ�, 1: ���� �밢��, 2: ������ �밢��
            enemies[i].type = rand() % 3; // ���� Ÿ�� ������ ���� (0, 1, �Ǵ� 2)
            break;
        }
    }
}
// ���� ��ġ ������Ʈ �� ȭ�� ǥ��
void update_enemy() {

    for (int i = 0; i < MAXEnemies; i++) {
        if (enemies[i].exist) {
            // ���� ��ġ�� �����
            gotoxy(enemies[i].x, enemies[i].y, "   "); // ���� ��ġ �����

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

                // ���ο� ��ġ�� �� ǥ��
                switch (enemies[i].type) { // Ÿ�Ժ� �� ǥ��
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
    //������� �Ѿ˰� ���� �Ѿ� �浹 �˻� 
    for (int i = 0; i < MAXBullet; i++) {
        if (bullets[i].exist) {
            // �̺� �浹 �˻�
            for (int j = 0; j < MAXEnemies; j++) {
                if (enemies[j].exist) {
                    // ���� ��ġ�� ũ�⸦ ����Ͽ� �浹 �˻�
                    if ((bullets[i].x >= enemies[j].x && bullets[i].x <= enemies[j].x + 3) && 
                        (bullets[i].y >= enemies[j].y && bullets[i].y <= enemies[j].y + 1)) {
                        bullets[i].exist = FALSE;            // �浹 �� �Ѿ� ����
                        gotoxy(bullets[i].x, bullets[i].y, " "); //�Ѿ� ����                         
                        gotoxy(enemies[j].x, enemies[j].y, "   ");  // �浹�� �� ��ġ�� ���� (ȭ�鿡�� ����)                        
                        
                        enemies[j].exist = FALSE;            // �浹 �� �� ����
                        player[player_i].score += 10;        // ���� �߰�
                        gotoxy(40, 17, "score: ");
                        printf("%d", player[player_i].score);
                        break;
                    }
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
            hero_lives--;                       // ����� ��� ����
            print_lives();
            boss_bullets[k].exist = FALSE;        // ���� �Ѿ� ����
            gotoxy(boss_bullets[k].x, boss_bullets[k].y, " "); // �Ѿ� �����

            if (hero_lives <= 0) {
                game_over();  // ���� ���� ó��
                return;
            }

            break; // �浹ó�� �� ���� Ż�� ���Ѽ� �ߺ����� ü�±��̴°� ����
        }
    }

    //�����۰� ����� �浹 �˻�
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
                    if(bullets[b_count].active == FALSE){ //Ȱ�� ���ϴ� �Ѿ� �����ڸ��� Ȱ����Ű�� break;
                        bullets[b_count].active = TRUE;
                        break; 
                    }
                }
            }
            item[l].active = FALSE;
            item[l].exist = FALSE;                // ������ ����
            gotoxy(item[l].x, item[l].y, " ");    // ������ �����
            gotoxy(25, 6, "            ");
            break;
        }
    }
    
    // ����ο� �� �浹 �˻�
    for (int j = 0; j < MAXEnemies; j++) {
        if (enemies[j].exist &&
            (enemies[j].x >= hero.x && enemies[j].x <= hero.x + 3) &&
            (enemies[j].y >= hero.y && enemies[j].y <= hero.y + 1)) {
            hero_lives--;                       // ����� ��� ����
            print_lives();
            enemies[j].exist = FALSE;          // �浹�� �� ����
            gotoxy(enemies[j].x, enemies[j].y, " "); // �� �����

            if (hero_lives <= 0) {
                game_over();  // ���� ���� ó��
                return;
            }
            break; // �浹ó�� �� ���� Ż��
        }
    }

}

// ���� ���� �� �ʱ�ȭ
void spawn_item() {
    for (int i = 0; i < MAXItem; i++) {
        if (item[i].active && !item[i].exist) { // �� �� ��ġ�� ����
            item[i].x = MAP_X + 1 + rand() % (MAP_WIDTH - 2); // x ��ġ ���� ����
            item[i].y = MAP_Y + 1;
            item[i].type = rand() % 2;
            item[i].exist = TRUE;
            break;
        }
    }
}
// ���� ��ġ ������Ʈ �� ȭ�� ǥ��
void update_item() {
    for (int i = 0; i < MAXItem; i++) {
        if (item[i].active && item[i].exist) {
            // ���� ��ġ�� �����
            gotoxy(item[i].x, item[i].y, " "); // ���� ��ġ �����

            // ������ �̵�
            item[i].y++;

            // ���� y ��ǥ�� �� ��踦 �Ѵ��� Ȯ��
            if (item[i].y >= MAP_Y + MAP_HEIGHT) {
                // �ϴ� ��迡 �������� ��� ������ ����
                item[i].exist = FALSE;
                item[i].active = FALSE;
                gotoxy(25, 6, "            ");
            }
            else {
                // ���ο� ��ġ�� ������ ǥ��
                switch (item[i].type) { // Ÿ�Ժ� ������ ǥ��
                    case 0:
                        gotoxy(item[i].x, item[i].y, "��");
                        break;
                    case 1:
                        gotoxy(item[i].x, item[i].y, "��");
                        break;
                }
            }
        }
    }

    if (rand() % 300 == 0) { // 1/300 Ȯ���� ������ ���� ����
        spawn_item(); // ���� Ȯ���� ������ ����
        for(int i = 0; i < MAXItem; i++){
            if(item[i].active == TRUE){ //Ȱ�� ���ϴ� �Ѿ� �����ڸ��� Ȱ����Ű�� break;
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
        printf("�� ");
    }
}

// ������ �߰��ϴ� �Լ�
void add_score(const char* nickname, int score) {
    ScoreNode* new_node = (ScoreNode*)malloc(sizeof(ScoreNode));
    strncpy(new_node->nickname, nickname, 19);
    new_node->nickname[19] = '\0'; // null terminator
    new_node->score = score;
    new_node->next = head; // �� ��带 ����Ʈ�� �� �տ� �߰�
    head = new_node;
}
// ���� ���� �Լ� (��������)
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

// ���� ��� �Լ�
void print_top_scores() {
    ScoreNode* current = head;
    int rank = 1;

    gotoxy(10, 10, "Top Scores:");
    while (current != NULL && rank <= 10) {
        gotoxy(10, 10 + rank, ""); // Ŀ�� ��ġ �̵�
        printf("%d. %s: %d", rank, current->nickname, current->score); // ���� ���
        current = current->next;
        rank++;
    }
}
// ������ ���Ͽ� �����ϴ� �Լ�
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
        // ������ ������ �׳� ����
        return;
    }

    char nickname[20];
    int score;

    while (fscanf(file, "%s %d", nickname, &score) == 2) {
        add_score(nickname, score);
    }

    fclose(file);
}
// ���� ����Ʈ �޸� ���� �Լ�
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
    gotoxy(4, 18, "�������� �� 5���� ���ɴϴ� �����ϼ���!");
    gotoxy(8, 20, "nickname : ");
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
    gotoxy(40, 7, "<Space> : shoot!");
    gotoxy(40, 8, "<Esc> : exit");
    
    gotoxy(42, 10, "<������>");
    gotoxy(39, 11, "�� : life   �� : bullet");
    
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
    sort_scores(); // ���� ����

    gotoxy(10, 5, "*************************");
    gotoxy(10, 6, "*       GAME OVER       *");
    gotoxy(10, 7, "*************************");
    gotoxy(10, 8, "Press Any Key to restart");
    gotoxy(10, 9, "or Esc to exit");
    
    print_top_scores(); // ���� 10���� ���
    save_scores_to_file(); // ������ ���Ͽ� ����

    char ch = _getch();
    
    if (ch == 27) {  // Esc Ű�� ���� ����
        exit(0);
    }
    else{
        game_reset();
        move_hero();        
    }


}


void game_reset() {
    // ����� ���� �ʱ�ȭ
    hero.x = 13;
    hero.y = 35;
    hero_lives = 3;

    // �÷��̾� ���� �ʱ�ȭ
    player[player_i].score = 0;

    // �Ѿ� �ʱ�ȭ
    for (int i = 0; i < MAXBullet; i++) {
        if(i<3)
            bullets[i].active = TRUE; // ó�� 4�� Ȱ��ȭ
        else
            bullets[i].active = FALSE;

        bullets[i].exist = FALSE;
        bullets[i].x = 0;
        bullets[i].y = 0;
    }

    // �� �ʱ�ȭ
    for (int i = 0; i < MAXEnemies; i++) {
        enemies[i].exist = FALSE;
        enemies[i].x = 0;
        enemies[i].y = 0;
    }

    // ���� �ʱ�ȭ
    boss.exist = FALSE;
    boss.x = (MAP_X + MAP_WIDTH) / 2 - 1;
    boss.y = MAP_Y + 1;
    boss.origin_life = 0;
    boss.life = 0;
    boss.direction = 0;

    // ���� �Ѿ� �ʱ�ȭ
    for (int i = 0; i < BOSSBullet; i++) {
        boss_bullets[i].exist = FALSE;
        boss_bullets[i].x = 0;
        boss_bullets[i].y = 0;
    }

    //���� �ð� �ʱ�ȭ -> �ð������ �� ����ϰ� ó���ϰ� �;��µ� ����
    lastBossUpdate = 0;
    for_bossUpdate = 0;
    lastBossFire = 0;

    // ������ �ʱ�ȭ
    for (int i = 0; i < MAXItem; i++) {
        item[i].active = TRUE;
        item[i].exist = FALSE;
        item[i].x = 0;
        item[i].y = 0;
        item[i].type = 0;
    }

    // �������̽��� �� �ʱ�ȭ
    system("cls");
    inter_face();
    map(player[player_i].nickname, player[player_i].score, hero_lives);
}