//Starting positions
int stx = 5;
int sty = 5;
//Number of locations in the frontier array.
int frontsize = 1;
//Number of locations in the visited array.
int vissize = 1;
//depth of the neigbhbor 
int depth = 0;
//Array locations. 20 to represent every possible location. 2 for x and y position.
int frontier[20][2];
int visited[81][2];
int neighbor[1][2];
//Size of the maze (one axis). If the maze isn't a perfect square, we may need another variable.
int mazesize = 9;
int dir = 0; //0, 1, 2, 3 for N, E, S, W

void setup() {
  // put your setup code here, to run once:
  frontier[0][0] = stx;
  frontier[0][1] = sty;
  visited[0][0] = stx;
  visited[0][1] = sty;
  Serial.begin(9600);
}

boolean checkVisited(int a[81][2], int x, int y) {
  boolean answer = false;
  for(int i = 0; i <= 81; i++) {
    if(a[i][0] == x && a[i][1] == y) {
      answer = true;
    }
  }
}

void adjustfrontier() {
  for(int i = 1; i < 21; i++) {
    frontier[i-1][0] = frontier[i][0];
    frontier[i-1][1] = frontier[i][1];
  }
}


void loop() {
  while(frontsize > 0) {
    int n[2] = {frontier[frontsize][0], frontier[frontsize][1]};
    adjustfrontier();
    frontsize--; //First in first out buffer (in theory).
    if(n[0] == mazesize && n[1] == mazesize) {
      Serial.println("Goal!");
      break;
    }
    else {
      //North
      if(dir == 0){
        if(n[1] > 1 && dir == 0) {
          neighbor[0][0] = n[0];
          neighbor[0][1] = n[1] - 1;
          depth++;
          if(checkVisited(visited, neighbor[0][0], neighbor[0][1]) == false) {
            //Insert code to turn robot in the appropriate direction and move
            visited[vissize][0] = neighbor[0][0]; //Array indicies start at 0, so these appended locations should always be fresh spots in the array.
            visited[vissize][1] = neighbor[0][1];
            frontier[frontsize][0] = neighbor[0][0];
            frontier[frontsize][1] = neighbor[0][1];
            vissize++;
            frontsize++;
          }
        }
        if(~(n[1] > 1)) {dir++;}
      }
      else if(dir == 1) {
        if(n[0] < mazesize && dir == 1) {
          neighbor[0][0] = n[0] + 1;
          neighbor[0][1] = n[1];
          depth++;
          if(checkVisited(visited, neighbor[0][0], neighbor[0][1]) == false) {
            //Insert code to turn robot in the appropriate direction and move
            visited[vissize][0] = neighbor[0][0];
            visited[vissize][1] = neighbor[0][1];
            frontier[frontsize][0] = neighbor[0][0];
            frontier[frontsize][1] = neighbor[0][1];
            vissize++;
            frontsize++;
          }
        }
        if(~(n[0] < mazesize)) {dir++;}
      }
      else if(dir == 2) {
        if(n[1] < mazesize && dir == 2) {
          neighbor[0][0] = n[0];
          neighbor[0][1] = n[1] + 1;
          depth++;
          if(checkVisited(visited, neighbor[0][0], neighbor[0][1]) == false) {
            //Insert code to turn robot in the appropriate direction and move
            visited[vissize][0] = neighbor[0][0];
            visited[vissize][1] = neighbor[0][1];
            frontier[frontsize][0] = neighbor[0][0];
            frontier[frontsize][1] = neighbor[0][1];
            vissize++;
            frontsize++;
          }
        }
        if(~(n[1] < mazesize)) {dir++;}
      }
      else if(dir == 3) {
        if(n[0] > 1) {
          neighbor[0][0] = n[0] - 1;
          neighbor[0][1] = n[1];
          depth++;
          if(checkVisited(visited, neighbor[0][0], neighbor[0][1]) == false) {
            //Insert code to turn robot in the appropriate direction and move
            visited[vissize][0] = neighbor[0][0];
            visited[vissize][1] = neighbor[0][1];
            frontier[frontsize][0] = neighbor[0][0];
            frontier[frontsize][1] = neighbor[0][1];
            vissize++;
            frontsize++;
          }
        }
        if(~(n[0] > 1)) {dir = 0;}
      }
    }
  }
}
