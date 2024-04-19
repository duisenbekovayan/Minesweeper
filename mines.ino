#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h> 
#define N 6
#define MINES 5
#define cs     53  
#define rst    8   
#define dc     7   
Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, rst);

// Joystick pins
int SW_pin = 6; 
int X_pin = A1; 
int Y_pin = A0; 

int mark_pin = 5;
int unmark_pin = 4;

char board[7][7];
char board_mines[7][7];
char last_board[7][7];
int row_mine[7];
int column_mine[7];

void generate_board(char b[][7]);
void display_board(char b[][7], int cursor_x, int cursor_y, int fl);
void position_mines(int mine_x[], int mine_y[]);
void set_mines(char bm[][7], int mine_x[], int mine_y[]);
void position_cursor(int *cursor_x, int *cursor_y);
int check_position(int mine_x[], int mine_y[], int cursor_x, int cursor_y);
int count_mines(char bm[][7], int cursor_x, int cursor_y);
void expand(char b[][7], char bm[][7], int cursor_x, int cursor_y);
int check_unconvers(char b[][7], char bm[][7]);
void clear_mines(char bm[][7]);

#define BLACK   0x0000 
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define YELLOW  0xFFE0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define ORANGE  0xFC00

void setup() {
  Serial.begin(9600);
  pinMode(SW_pin, INPUT);
  pinMode(mark_pin, INPUT);
  pinMode(unmark_pin, INPUT);

  tft.initR(INITR_BLACKTAB);  // Initialize display
  tft.fillScreen(BLACK); // fill display with black cells
  tft.setRotation(1);
  tft.setTextSize(2); // need to display the text
  tft.setCursor(0, 60);
  tft.println(" MINESWEEPER");  // start of the game
  delay(3000);
  tft.fillScreen(BLACK);
  tft.setCursor(0, 40);
  tft.println("  You have");
  tft.println("  5 mines");
  tft.println("  Good Luck!"); // print the introduction text
  delay(5000);
}

void loop() { // main program to run our game
  char option = ' ';
  int state = 0; // state 0 means we must continue our game
  int adjacent_mines;
  int cursor_row = 3, cursor_column = 3; // start from the position 3, 3
  int last_cursor_row = 3, last_cursor_column = 3;
  int	mines_uncovered = 0;
  int	flags = 0;

  generate_board(board);
  copyArray(board, last_board);
  position_mines(row_mine, column_mine);
  set_mines(board_mines, row_mine, column_mine);
  display_board(board, cursor_row, cursor_column, flags);
  do{
    position_cursor(&cursor_row, &cursor_column);
  
    if(last_cursor_row != cursor_row || last_cursor_column != cursor_column){
        last_cursor_row = cursor_row;
        last_cursor_column = cursor_column;
        display_board(board, cursor_row, cursor_column, flags);
    }
    if(digitalRead(SW_pin) == 0){ // try to open the cell
      option = 'a';
      delay(100);
    }
    else{
      if(digitalRead(mark_pin) == 0){ // mark the cell as flag
        option = 'b';
        delay(100);
      }
      else
        if(digitalRead(unmark_pin) == 0){ // unmark the cell 
          option = 'c';
          delay(100);
        }
        else
          option = ' ';
    }

    switch(option){
      case 'a':
        if (board[cursor_row][cursor_column] != 'M'){  // if the flag was putted before we can't open it
        //otherwise open this
          state = check_position(row_mine, column_mine, cursor_row, cursor_column);
          if(state == 0){
            adjacent_mines = count_mines(board_mines, cursor_row, cursor_column);
            board[cursor_row][cursor_column] = adjacent_mines + '0';
            if(adjacent_mines == 0){ 
              board[cursor_row][cursor_column] = '-';
              expand(board, board_mines, cursor_row, cursor_column, flags);
            }
          }
          display_board(board, cursor_row, cursor_column, flags);
        }
        break;
      case 'b': // mark the cell as flag
        if(board[cursor_row][cursor_column] == '#' && board_mines[cursor_row][cursor_column] == 'X' && flags < MINES){
          board[cursor_row][cursor_column] = 'M';
          mines_uncovered++;
          flags++;
          display_board(board, cursor_row, cursor_column, flags);
        }
        if(board[cursor_row][cursor_column] == '#' && board_mines[cursor_row][cursor_column] != 'X' && flags < MINES){
          board[cursor_row][cursor_column] = 'M';
          flags++;
          display_board(board, cursor_row, cursor_column, flags);
        }
        break;
      case 'c': // unmark the cell as flag
        if(board[cursor_row][cursor_column] == 'M' && board_mines[cursor_row][cursor_column] == 'X' && flags > 0){
          board[cursor_row][cursor_column] = '#';
          mines_uncovered--;
          flags--;
          display_board(board, cursor_row, cursor_column, flags);
        }
        if(board[cursor_row][cursor_column] == 'M' && board_mines[cursor_row][cursor_column] != 'X' && flags > 0){
          board[cursor_row][cursor_column] = '#';
          flags--;
          display_board(board, cursor_row, cursor_column, flags);
        }
        break;
    }
    
    if (check_unconvers(board, board_mines) == 1)
      state = 2;

  }while(state == 0); // continue until we win or lose
tft.fillScreen(BLACK); 
tft.setCursor(0, 40);
  if(state == 1){ // we lose
      tft.println("   CaBOOM!");
      delay(2000);
      display_board(board, cursor_row, cursor_column, flags);
      display_board_GG(board, cursor_row, cursor_column, flags, board_mines);
      delay(2000);
      tft.fillScreen(BLACK);
      tft.setCursor(0, 40);
      tft.println("  Game over");
  }
  else if(state == 2){ // we win
    tft.println("  Gracio!");
    tft.println("  You win!");
    delay(2000);
    tft.fillScreen(BLACK);
    delay(2000);
    display_board_GG(board, cursor_row, cursor_column, flags, board_mines);
    delay(2000);
  }

  delay(4000);
  clear_mines(board_mines);
}

void copyArray(char source[][7], char destination[][7]) {  // Copies the contents of one array to another.
    for (int i = 0; i < 7; ++i) {
      for(int j = 0; j < 7; j++)
        destination[i][j] = source[i][j];
    }
}


void generate_board(char b[][7]){ //  Initializes the game board with all cells covered.
	int x, y;
	for(x = 1;  x <= N; x++){
		for(y = 1; y <= N; y++){
			b[x][y] = '#';
		}
	}
}

void display_board_GG(char b[][7], int cursor_x, int cursor_y, int fl, char bm[][7]) // Displays the final game state.
{
  tft.fillScreen(BLACK);
	int x, y;
  char text_flags[] = "flags ";
  int r; 
  
  tft.setTextSize(2);
  tft.setTextColor(ORANGE);
  tft.setCursor(0,0);
  tft.println();
  tft.print(" ");

	for(x = 1; x <= N; x++){
		for(y = 1; y <= N; y++){
      if(x == cursor_x && y == cursor_y){
        tft.print(bm[x][y]);
        tft.print("<");
      }
      else{
        if(bm[x][y] == 'X') 
        {
          tft.print(bm[x][y]);
        }
        else
        {
          tft.print(b[x][y]);
        }
        tft.print(" ");
      }
		}
    tft.println();
    tft.print(" ");
    
	}
  tft.setTextSize(2);
  tft.print(text_flags);
  tft.print(MINES - fl);

  delay(100);
}

void display_board(char b[][7], int cursor_x, int cursor_y, int fl){ // Displays the game board on the TFT display, highlighting the current cursor position.
  tft.fillScreen(BLACK);
	int x, y;
  char text_flags[] = "flags ";
  int r; 
  
  tft.setTextSize(2);
  tft.setTextColor(ORANGE);
  tft.setCursor(0,0);
  tft.println();
  tft.print(" ");

	for(x = 1; x <= N; x++){
		for(y = 1; y <= N; y++){
      if(x == cursor_x && y == cursor_y){
        tft.print(b[x][y]);
        tft.print("<");
      }
      else{
  			tft.print(b[x][y]);
        tft.print(" ");
      }
		}
    tft.println();
    tft.print(" ");
    
	}
  tft.setTextSize(2);
  tft.print(text_flags);
  tft.print(MINES - fl);

  delay(100);
}

void position_mines(int mine_x[], int mine_y[]){
	int i, j;
	randomSeed(analogRead(A5));

	for(i = 1; i <= MINES; i++){
		mine_x[i] = random(1, N);
		mine_y[i] = random(1, N);
		for(j = 1 ; j<i ; j++){
			if(mine_x[i] == mine_x[j] && mine_y[i] == mine_y[j])
				i--;
		}
	}
}6tyuj
 
void set_mines(char bm[][7], int mine_x[], int mine_y[]){ // Marks mine positions on the board.
	int i;
	for(i=1; i <= MINES; i++){
		bm[mine_x[i]][mine_y[i]] = 'X';
	}
}

void position_cursor(int *cursor_x, int *cursor_y){ // Handles joystick input to move the cursor.
  int joy_x = analogRead(A1);
  int joy_y = analogRead(A0);

  if(joy_x < 300 && *cursor_x < 6){
    (*cursor_x)++;
  }
  else if(joy_x > 700 && *cursor_x > 1){
    (*cursor_x)--;
  }

  if(joy_y > 700 && *cursor_y < 6){
    (*cursor_y)++;
  }
  else if(joy_y < 300 && *cursor_y > 1){
    (*cursor_y)--;
  }
}

int check_position(int mine_x[], int mine_y[], int cursor_x, int cursor_y){  // Checks if a given position contains a mine.
	int i;
	for(i = 1; i <= MINES; i++){
		if(mine_x[i] == cursor_x && mine_y[i] == cursor_y)
			return 1;
	}
	return 0;
}

int count_mines(char bm[][7], int cursor_x, int cursor_y){ // Counts the number of adjacent mines.
  int i, j;
  int total = 0;

  for(i = -1; i <= 1; i++){
    for(j = -1; j <= 1; j++){
      if(bm[cursor_x - i][cursor_y - j] == 'X')
        total++;
    }
  }
	return total;
}

void expand(char b[][7], char bm[][7], int cursor_x, int cursor_y, int &fl){  // Expands the board when a cell with no adjacent mines is selected. Use recursion
  int x, y;
  int i, j;

  for(i = -1; i <= 1; i++){
    for(j = -1; j <= 1; j++){
      x = cursor_x;
      y = cursor_y;
      while(count_mines(bm, x, y) == 0 && b[x+i][y+j] != '-'){
        x += i;
        y += j;
        if(x > 0 && y > 0 && x < 7 && y < 7){
          if(count_mines(bm, x, y) == 0){ // case when we can expand from the x and y cell to the x + i and y + j cel
      			if(b[x][y] == 'M') 
            {
              fl++;
            }
            b[x][y] = '-';
            expand(b, bm, x, y, fl);
          }
      		else
          {
            if(b[x][y] == 'M')
            {
              fl++;
            }
      			b[x][y] = count_mines(bm, x, y) + '0';
          }
        }
    	}
    }
  }
}
int check_unconvers(char b[][7], char bm[][7]){ // Checks if all non-mine cells have been uncovered.
	int x, y;
	for(x = 1 ; x <= N; x++){
		for(y = 1 ; y <= N; y++){
			if(bm[x][y] != 'X'){
				if(b[x][y] == '#')
					return 0;
			}
		}
	}
	return 1;
}
void clear_mines(char bm[][7]){ // Clears the board
  int x, y;
  for(x = 1;  x <= N; x++){
    for(y = 1; y <= N; y++){
      bm[x][y] = ' ';
    }
  }
}