#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <conio.h>

int mod(int num1, int num2){
  return ((num1%num2)+num2)%num2;
}

//Gets the current position of the cursor
COORD GetConsoleCursorPosition(HANDLE hConsoleOutput){
    CONSOLE_SCREEN_BUFFER_INFO cbsi;
    if (GetConsoleScreenBufferInfo(hConsoleOutput, &cbsi)){
        return cbsi.dwCursorPosition;
    }
    else{
        COORD invalid = { 0, 0 };
        return invalid;
    }
}
//Moves the cursor to the given coordinates
void go_to_yx(int line, int column){
    COORD coord;
    coord.X = column;
    coord.Y = line;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(hConsole, coord);
}

//Board array size: board[0..boardHeight+1][0..boardWidth+2]
//Playable board size: board[1..boardHeight][1..boardWidth]
//Board accessing convention: board[y][x]

#define ESCAPE 27

int main() {
  const int boardHeight=24;
  const int boardWidth=80;

  HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
  COORD startingCoord=GetConsoleCursorPosition(hStdOut);

  const int topLeftBorderX=0;
  const int topLeftBorderY=startingCoord.Y+4;

  int relativeBoardPosX=1;
  int relativeBoardPosY=1;

  char board[boardHeight+2][boardWidth+3]; //this generation's current board state
  char nextGenBoard[boardHeight+2][boardWidth+3]; //the next generation's board state
  char savedBoard[boardHeight+2][boardWidth+3]; //the saved 'seed' board state

  //populates board array with 1-cell thick borders, boardHeight*boardWidth spaces, and '\n' for formatting
  for (int i=0; i<=boardHeight+1; i++){
    for (int j=0; j<=boardWidth; j++){
      board[i][j]=' ';
      board[0][j]='X';
      board[boardHeight+1][j]='X';
    }
    board[i][boardWidth+1]='X';
    board[i][boardWidth+2]='\n';
    board[i][0]='X';
  }
  board[boardHeight+1][boardWidth+2]='\0';

  while (1){ //beginning of game loop
  printf("CREATE STARTING PATTERN:\nWASD - Move cursor               Enter - Confirm location\nTab - Clear board                Backspace - Delete location\nSpacebar - Start simulation      Escape - Exit\n");
  printf(*board);
  relativeBoardPosX=boardWidth/2;
  relativeBoardPosY=boardHeight/2;
  go_to_yx(topLeftBorderY+relativeBoardPosY,topLeftBorderX+relativeBoardPosX); //start cursor in the middle of the board

  while (1){ //beginning of setup loop
  bool leaveLoop=0;
  char c = getch();
  switch (c){
  case 'w': //W: Move cursor up 1 cell
  case 'W':
    relativeBoardPosY=mod(relativeBoardPosY-2,boardHeight)+1;
    go_to_yx(topLeftBorderY+relativeBoardPosY,topLeftBorderX+relativeBoardPosX);
    break;
  case 's': //S: Move cursor down 1 cell
  case 'S':
    relativeBoardPosY=mod(relativeBoardPosY,boardHeight)+1;
    go_to_yx(topLeftBorderY+relativeBoardPosY,topLeftBorderX+relativeBoardPosX);
    break;
  case 'a': //A: Move cursor left 1 cell
  case 'A':
    relativeBoardPosX=mod(relativeBoardPosX-2,boardWidth)+1;
    go_to_yx(topLeftBorderY+relativeBoardPosY,topLeftBorderX+relativeBoardPosX);
    break;
  case 'd': //D: Move cursor right 1 cell
  case 'D':
    relativeBoardPosX=mod(relativeBoardPosX,boardWidth)+1;
    go_to_yx(topLeftBorderY+relativeBoardPosY,topLeftBorderX+relativeBoardPosX);
    break;
  case '\r': //Enter: Confirm X placement
      board[relativeBoardPosY][relativeBoardPosX]='X';
      printf("%c",'X');
    go_to_yx(topLeftBorderY+relativeBoardPosY,topLeftBorderX+relativeBoardPosX);
    Sleep(50);
    break;
  case '\b': //Backspace: Remove X
    board[relativeBoardPosY][relativeBoardPosX]=' ';
    printf("%c",' ');
    go_to_yx(topLeftBorderY+relativeBoardPosY,topLeftBorderX+relativeBoardPosX);
    Sleep(50);
    break;
  case ' ': //Spacebar: Start simulation
    go_to_yx(startingCoord.Y,0);
    printf("GAME OF LIFE SIMULATION:          \nSpacebar - Restart simulation                                                                               \nEscape - Back to setup                                       \n                                                               \n");
    go_to_yx(topLeftBorderY+boardHeight+1,topLeftBorderX+boardWidth+2);
    leaveLoop=1;
    Sleep(200);
    break;
  case '\t': //Tab: Clear board
    for (int i=1; i<=boardHeight; i++)
      for (int j=1; j<=boardWidth; j++)
        board[i][j]=' ';
    go_to_yx(topLeftBorderY,topLeftBorderX);
    printf(*board);
    relativeBoardPosX=boardWidth/2;
    relativeBoardPosY=boardHeight/2;
    go_to_yx(topLeftBorderY+relativeBoardPosY,topLeftBorderX+relativeBoardPosX);
    break;
  case ESCAPE: //Escape: Exit game
    go_to_yx(topLeftBorderY+boardHeight+1,topLeftBorderX+boardWidth+2);
    return 0;
  default:
    break;
  }
  if (leaveLoop)
    break;
  } //end of setup loop
  memcpy(*nextGenBoard,*board,(boardHeight+2)*(boardWidth+3));
  memcpy(*savedBoard,*board,(boardHeight+2)*(boardWidth+3));
  while (1){ //beginning of simulation loop
    bool boardChanged=0;
    bool leaveLoop=0;
    //Counts the number of living neighbors. Modulo enables wrap-around checking of neighbors.
    for (int i=1; i<=boardHeight; i++){
      for (int j=1; j<=boardWidth; j++){
        int livingNeighbors=0;
        for (int y=i-1; y<=i+1; y++){
          int neighborCoordY=mod(y-1,boardHeight)+1;
          for (int x=j-1; x<=j+1; x++){
            int neighborCoordX=mod(x-1,boardWidth)+1;
            if (board[neighborCoordY][neighborCoordX]=='X' && (neighborCoordY!=i || neighborCoordX!=j))
              livingNeighbors++;
          }
        }
        if (board[i][j]=='X'){ //if the cell is living
          if (livingNeighbors<2 || livingNeighbors>3){ //and has less than 2 or more than 3 living neighbors
            nextGenBoard[i][j]=' '; //the cell dies
            boardChanged=1;
          }
        }
        else if (livingNeighbors==3){ //if the cell is dead and has 3 living neighbors
            nextGenBoard[i][j]='X'; //the cell becomes alive
            boardChanged=1;
        }
      }
    }
    if (boardChanged){
      memcpy(*board,*nextGenBoard,(boardHeight+2)*(boardWidth+2));
      go_to_yx(topLeftBorderY,topLeftBorderX);
      printf(*board);
    }
    Sleep(200);
  if (_kbhit()){ //if any key is pressed
    char c = getch();
    switch (c){
    case ESCAPE: //Escape: return to setup
      leaveLoop=1;
      Sleep(50);
      memcpy(*board,*savedBoard,(boardHeight+2)*(boardWidth+3));
      go_to_yx(startingCoord.Y,0);
      break;
    case ' ': //Space: Restart simulation
      memcpy(*board,*savedBoard,(boardHeight+2)*(boardWidth+3));
      memcpy(*nextGenBoard,*savedBoard,(boardHeight+2)*(boardWidth+3));
      go_to_yx(topLeftBorderY,topLeftBorderX);
      printf(*board);
      Sleep(200);
      break;
    default:
      break;
    }
  }
  if (leaveLoop)
    break;
  } //end of simulation loop
  } //end of game loop
  return 0;
}


