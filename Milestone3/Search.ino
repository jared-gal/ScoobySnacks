//Starting positions
int stx = 5;
int sty = 5;
//Number of locations in the frontier array.
int frontsize = 1;
//Number of locations in the visited array.
int vissize = 0;
//depth of the neigbhbor 
int depth = 0;
//Array locations. 20 to represent every possible location. 2 for x and y position.
int frontier[81][2];
int visited[81][2];
int neighbor[81][2];
//Size of the maze (one axis). If the maze isn't a perfect square, we may need another variable.
int mazesize = 9;

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

void loop() {
  while(frontsize > 0) {
    int n[2] = {frontier[0][0], frontier[0][1]};
    frontier[0][0] = 0;
    frontier[0][1] = 0;
    frontsize--;
    if(n[0] == mazesize && n[1] == mazesize) {
      Serial.println("Goal!");
      break;
    }
    else {
      //North
      if(n[1] > 1) {
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
      //East
      if(n[0] < mazesize) {
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
      //South
      if(n[1] < mazesize) {
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
      //West
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
    }
  }
}
