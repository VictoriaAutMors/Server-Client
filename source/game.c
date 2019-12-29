#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <wait.h>

int ships[4];
int my_ships = 10;
int enemy_ships = 10;
char field_for_me[10][10];

void Field_raise () {
    char slot = 'A';
    if (fork() == 0){
        printf("  ");
        for (int i = 0; i < 10; i++) {
            printf("%d", i + 1);
        }
        putchar('\n');
        for (int i = 0; i < 10; i++) {
            putchar(slot + i);
            putchar(' ');
            for (int j = 0; j < 10; j++) {
                putchar(field_for_me[i][j]);
            }
            putchar('\n');
        }
    }
    wait(NULL);
}

void clear_execute(){
    if(fork() == 0){
        if(execlp("clear", "clear", NULL) < 0){
            perror("Error in cleaning terminal.");
        }
        /*for(int i = 0; i < 60; i++){
            putchar('\n');
        }*/
    }
    wait(NULL);
}

struct termios saved_attributes;

void
reset_input_mode (void)
{
  tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
}

void set_input_mode (void)
{
  struct termios tattr;
  char *name;

  /* Make sure stdin is a terminal. */
  if (!isatty (STDIN_FILENO))
    {
      fprintf (stderr, "Not a terminal.\n");
      exit (EXIT_FAILURE);
    }

  /* Save the terminal attributes so we can restore them later. */
  tcgetattr (STDIN_FILENO, &saved_attributes);
  atexit (reset_input_mode);

  /* Set the funny terminal modes. */
  tcgetattr (STDIN_FILENO, &tattr);
  tattr.c_lflag &= ~(ICANON|ECHO); /* Clear ICANON and ECHO. */
  tattr.c_cc[VMIN] = 1;
  tattr.c_cc[VTIME] = 0;
  tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);
}

void print_greetings(){
    puts("Welcome to SeaBattle game");
    puts("You have 4 types of ships");
    puts("At the left: The Place that ship occupied(in '0' symbols or cells)");
    puts("At the right: The count of this ships");
    puts("cells-count");
    puts("4      1");
    puts("3      2");
    puts("2      3");
    puts("1      4");
}

int check(int deck, int x, int y, char * side){
    x--;
    deck--;
    if (side[0] == 'U') {
        if (y - deck >= 0) {
            return 0;
        }
        else {
            return 1;
            }
    }
    if(side[0] == 'L') {
        if (x - deck >= 0) {
            return 0;
        }
        else {
            return 1;
        }
    }
    if(side[0] == 'D') {
        if (y + deck <= 9) {
            return 0;
        }
        else {
            return 1;
        }
    }
    if(side[0] == 'R'){
        if (x + deck <= 9){
            return 0;
        }
        else {
            return 1;
        }
    }
    printf("You enter a wrong side of rotate.");
    return 1;
}

void put_ship(int deck, int x, int y, char * side){
    x--;
    if (side[0] == 'R') {
        for (int i = 0; i < deck; i++){
            field_for_me[x][y + i] = 'x';
        }
    }
    if (side[0] == 'L') {
        for (int i = 0; i < deck; i++){
            field_for_me[x][y - i] = 'x';
        }
    }
    if (side[0] == 'D') {
        for (int i = 0; i < deck; i++){
            field_for_me[x + i][y] = 'x';
        }
    }
    if (side[0] == 'U') {
        for (int i = 0; i < deck; i++){
            field_for_me[x - i][y] = 'x';
        }
    }
}

int check_ships (char * enemy_field, 
                 char * enemy_field_for_enemy, 
                 int x, int y) {

    char up = enemy_field_for_enemy[x - 1][y];
    char down = enemy_field_for_enemy[x - 1][y - 2];
    char right = enemy_field_for_enemy[x][y - 1];
    char left = enemy_field_for_enemy[x - 2][y - 1];

    int ships_around = (up == '0' && left == '0' && down == '0' && right == '0');
    if ( ships_around ||
        ( enemy_field[x - 1][y] == 'x' && down == '0' && right == '0' && left == '0') || 
        ( enemy_field[x][y - 1] == 'x' && down == '0' && up == '0' && left == '0')|| 
        ( enemy_field[x - 1][y - 2] == 'x' && up == '0' && left == '0' && right == '0') || 
        ( enemy_field[x - 2][y - 1] == 'x' && up == '0' && right == '0' && down == '0') ) {
            enemy_ships--;
    }

}

char **shot ( char **enemy_field, 
              char **enemy_field_for_enemy) {
    int x, y;
    puts("Enter coordinates of the cell that you want to shoot.")
    printf("coordinate x:");
    scanf("%d", x);
    printf("coordinate y:");
    scanf("%d", y);
    if (enemy_field_for_enemy[x - 1][y - 1] == '0') {
        enemy_field[x - 1][y - 1] = '@';
    }
    else {
        enemy_field[x - 1][y - 1] = 'x';
        check_ships(enemy_field, enemy_field_for_enemy, x, y);
    }
    return enemy_field;
}


int enter_ship() {
    int deck, x;
    char y[1] = "A";
    char side[1];

    printf("Choose ship type:");
    scanf("%d", &deck) ;
    printf("%d", ships[deck - 1]);
    if (ships[deck - 1] > 0) {
        printf("Choose x coordinate:");
        scanf("%d", &x);
        printf("%d", x);
        if (x >= 1 && x <= 10) {
            printf("Choose y coordinate:");
            scanf("%s", y);
            printf("%s", y);
            if ((int)(y[0] - 'A') >= 1 && (int)(y[0] - 'A') <= 10) {
                scanf("%s", side);
                if (!check(deck, x, y[0] - 'A', side)) {
                    put_ship(deck , x, y[0] - 'A', side);
                }
                else {
                    printf("Error in putting ship.");
                }
            }
            else {
                puts("Wrong coordinate y.");
                return 1;
            }
        }
        else {
            puts("Wrong coordinate x.");
            return 1;
        }
    }
    else {
        puts("You hanen't enough ships");
        return 1;
    }
    return 0;
}

int main(int argc, char ** argv){
    //set_input_mode();
    print_greetings();
    char field_for_enemy[10][10], enemy_field_for_enemy[10][10], enemy_field[10][10];
    char slot = 'A';

    ships[0] = 4;
    ships[1] = 3;
    ships[2] = 2;
    ships[3] = 1;

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            field_for_enemy[i][j] = '0';
            field_for_me[i][j] = '0';
        }
    }

    while(1) { 
        Field_raise();
        puts("Please enter coordintes of your ship and side that he rotated relative to the stem.");
        puts("Count of Decks(1-4)");
        puts("x(0, 10)");
        puts("y(0, 10)");
        puts("Side({U,D,L,R})");
        if ( enter_ship() ) {
            break;
        }
        if (ships[0] == 0 && ships[1] == 0 && ships[2] == 0 && ships[3] == 0) {
            break;
        }
        clear_execute();
    }   
    clear_execute();

    puts ("You enter ALL your ships.");
    puts ("Get ready to start the game.");
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            enemy_field_for_enemy[i][j] = '0';
            enemy_field[i][j] = '0';
        }
    }

    while (1) {
        enemy_field = shot(field_for_enemy, enemy_field, enemy_field_for_enemy);
        
        puts("Your field");
        printf("  ");
        for (int i = 0; i < 10; i++) {
            printf("%d", i + 1);
        }
        putchar('\n');
        for (int i = 0; i < 10; i++) {
            putchar(slot + i);
            putchar(' ');
            for (int j = 0; j < 10; j++) {
                putchar(field_for_me[i][j]);
            }
            putchar('\n');
        }

        puts("Enemy field");
        printf("  ");
        for (int i = 0; i < 10; i++) {
            printf("%d", i + 1);
        }
        putchar('\n');
        for (int i = 0; i < 10; i++) {
            putchar(slot + i);
            putchar(' ');
            for (int j = 0; j < 10; j++) {
                putchar(enemy_field[i][j]);
            }
            putchar('\n');
        }

        if (enemy_ships == 0) {
            puts("Greetings!You win!");
            break;
        }
        if(my_ships == 0){
            puts("You lose!");
            break;
        }
        clear_execute();
    }

    return 0;
}