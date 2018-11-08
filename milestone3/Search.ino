int stx = 5;
int sty = 5;
int frontsize = 0;
int vissize = 0;
int depth = 0;
int initial[20][2] = {stx, sty};
int frontier[20][2] = {stx, sty};
int visited[20][2] = {stx, sty};
int neighbor[20][2];
int mazesize = 9;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
}

void loop() {
  while(frontsize > 0) {
    int n[2] = {frontier[0][0], frontier[0][1]};
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
      }
      //East
      if(n[0] < mazesize) {
        
      }
      //South
      if(n[1] < mazesize) {
        
      }
      //West
      if(n[0] > 1) {
        
      }
    }
  }
}
