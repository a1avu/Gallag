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
    int type;
} Enemy;

Bullet bullets[MAXBullet] = {{FALSE, 0, 0}, {FALSE, 0, 0}};
Enemy enemies[MAXEnemies] = {{FALSE, 0, 0, 0}};

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
void check_collision();
void game_over();

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
        update_bullet();//�������ȿ� �����Ƿ� ��� ���ư�
        update_enemy();           // �� ���� ������Ʈ
        check_collision();        // �浹 �˻�
         
         if (hero_lives <= 0) {
            game_over();  // ���� ���� ȭ�� ǥ��
            break;
        }
          // ���� ��� ���� ������Ʈ
        gotoxy(40, 19, "Lives: ");
        for (int i = 0; i < hero_lives; i++) {
            printf("�� ");
        }
        for (int i = hero_lives; i < 3; i++) {
            printf("  ");
        }

        Sleep(50);
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

void spawn_enemy() {
    for (int i = 0; i < MAXEnemies; i++) {
        if (!enemies[i].exist) {                 // �� �� ��ġ�� ����
            enemies[i].x = MAP_X + 1 + rand() % (MAP_WIDTH - 2);  // x ��ġ ���� ����
            enemies[i].y = MAP_Y + 1;
            enemies[i].exist = TRUE;
            enemies[i].type = rand() % 3;         // ���� Ÿ�� ������ ���� (0, 1, �Ǵ� 2)
            break;
        }
    }
}

// ���� ��ġ ������Ʈ �� ȭ�� ǥ��
void update_enemy() {
    for (int i = 0; i < MAXEnemies; i++) {
        if (enemies[i].exist) {
            gotoxy(enemies[i].x, enemies[i].y, "   ");  // ���� ��ġ �����
            enemies[i].y++;  // �� �Ʒ��� �̵�

            // ���� ����ο� ���� y ��ġ�� ������ ��
            if (enemies[i].y == hero.y) {
                if (abs(enemies[i].x - hero.x) <= 1) { // ����� ��ġ�� ������� Ȯ��
                    hero_lives--;  // ������� ��� �ϳ� ����
                    enemies[i].exist = FALSE;  // �� ����
                    if (hero_lives <= 0) {
                        gotoxy(MAP_X + MAP_WIDTH / 2, MAP_Y + MAP_HEIGHT / 2, "Game Over");
                        return;
                    }
                    continue;  // �ٸ� �� ������Ʈ�� �̵�
                }
            }

            // ���� ���� �ϴ� ��迡 ������ ����, �׵θ��� ������ ����
            if (enemies[i].y > MAP_Y + MAP_HEIGHT - 1) {
                enemies[i].exist = FALSE;
            } else {
                switch (enemies[i].type) {  // Ÿ�Ժ� �� ǥ��
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

    if (rand() % 10 == 0) spawn_enemy();  // ���� Ȯ���� �� ����
    Sleep(200);  // ���� �̵� �ӵ��� �����Ͽ� õõ�� �������� ��
}

// �Ѿ˰� ���� �浹 �˻� �Լ�
void check_collision() {
    for (int i = 0; i < MAXBullet; i++) {
        if (bullets[i].exist) {
            for (int j = 0; j < MAXEnemies; j++) {
                if (enemies[j].exist && bullets[i].x == enemies[j].x && bullets[i].y == enemies[j].y) {
                    bullets[i].exist = FALSE;            // �浹 �� �Ѿ� ����
                    enemies[j].exist = FALSE;            // �浹 �� �� ����
                    player[player_i].score += 10;        // ���� �߰�
                    gotoxy(40, 17, "score: ");
                    printf("%d", player[player_i].score);
                    gotoxy(enemies[j].x, enemies[j].y, "   ");  // �浹�� �� ��ġ�� ���� (ȭ�鿡�� ����)
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