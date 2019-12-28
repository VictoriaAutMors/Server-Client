#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <wait.h>

#ifndef FIELD_H
#define FIELD_H

int ships[4];
char field_for_me[10][10];


void moved(int row, int col);
void clickhandler();
char slot = 'A';
void Field_raise () {
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

#endif

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

int enter_ship(){
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
    char field_for_enemy[10][10];
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
        if (enter_ship()){
            break;
        }
        clear_execute();
    }   
    clear_execute();
    return 0;
}