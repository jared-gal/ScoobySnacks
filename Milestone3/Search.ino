//Starting positions
int stx = 5;
int sty = 5;
//Size of the frontier
int frontsize = 0;
//Size of Visited array
int vissize = 0;
//depth of the neigbhbor 
int depth = 0;
//Array locations. 20 to represent every possible location. 2 for x and y position.
int initial[20][2] = {stx, sty};
int frontier[20][2] = {stx, sty};
int visited[20][2] = {stx, sty};
int neighbor[20][2];
//Number of nodes in the maze.
int mazesize = 9;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
}

boolean checkVisited(int a[20][2], int x, int y) {
  answer = false;
  for(int i = 0, i <= 20, i++) {
    if(a[j][0] == x && a[j][1] == y) {
      answer = true;
    }
  }
}

void loop() {
  while(frontsize > 0) {
    int n[2] = {frontier[0][0], frontier[0][1]};
    frontier[0][0] = 0;
    frontier[0][1] = 0;
    if(n[0] == mazesize && n[1] == mazesize) {
      Serial.println("Goal!");
      break;
    }
    else {
      //North
      if(n[1] > 1) {
        neighbor[0][0] = n[0];
        neighbor[0][1] = n[1] -1;
        depth++;
        if(~checkVisited) {
          visited[vissize+1][0] = neighbor[0][0];
          visited[vissize+1][1] = neighbor[0][1];
          frontier[frontsize+1][0] = neighbor[0][0];
          frontier[frontsize+1][1] = neighbor[0][1];
        }
      }
      //East
      if(n[0] < mazesize) {
        neighbor[0][0] = n[0] + 1;
        neighbor[0][1] = n[1];
        depth++;
        if(~checkVisited) {
          visited[vissize+1][0] = neighbor[0][0];
          visited[vissize+1][1] = neighbor[0][1];
          frontier[frontsize+1][0] = neighbor[0][0];
          frontier[frontsize+1][1] = neighbor[0][1];
        }
      }
      //South
      if(n[1] < mazesize) {
        neighbor[0][0] = n[0];
        neighbor[0][1] = n[1] + 1;
        depth++;
        if(~checkVisited) {
          visited[vissize+1][0] = neighbor[0][0];
          visited[vissize+1][1] = neighbor[0][1];
          frontier[frontsize+1][0] = neighbor[0][0];
          frontier[frontsize+1][1] = neighbor[0][1];
        }
      }
      //West
      if(n[0] > 1) {
        neighbor[0][0] = n[0] - 1;
        neighbor[0][1] = n[1];
        depth++;
        if(~checkVisited) {
          visited[vissize+1][0] = neighbor[0][0];
          visited[vissize+1][1] = neighbor[0][1];
          frontier[frontsize+1][0] = neighbor[0][0];
          frontier[frontsize+1][1] = neighbor[0][1];
        }
      }
    }
  }
}
