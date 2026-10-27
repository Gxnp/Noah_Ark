const int arr[10][7] = {
  {1,1,1,1,1,1,0},//0
  {0,1,1,0,0,0,0},//1
  {1,1,0,1,1,0,1},//2
  {1,1,1,1,0,0,1},//3
  {0,1,1,0,0,1,1},//4
  {1,0,1,1,0,1,1},//5
  {1,0,1,1,1,1,1},//6
  {1,1,1,0,0,0,0},//7
  {1,1,1,1,1,1,1},//8
  {1,1,1,1,0,1,1}//9
};

void setup()
{
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(11,OUTPUT);
  pinMode(12,OUTPUT);
}
void showNumber(int a){
  for (int j=0; j< 7; j++){
    digitalWrite(j+2, arr[a][j]);
  }
}

void loop()
{
  int numarr[8] = {6,8,0,7,0,1,5,3};
  for(int i=0;i < 8; i+=2){
    for (int k = 0; k <10;k++){
      showNumber(numarr[i]);
      digitalWrite(11,HIGH);
      digitalWrite(12,LOW);
      delay(100);
      showNumber(numarr[i+1]);
      digitalWrite(11,LOW);
      digitalWrite(12,HIGH);
      delay(100);
    }
  }
}
