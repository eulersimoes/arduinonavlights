//1) USAR ARDUINO 1.06
//2) SELECIONAR A PLACA ATtiny85 (internal 8MHZ clock
//3) SELECIONAR GRAVAR BOOTLOAD para ajustar o clock do processador
//4) UPLOAD DO PROGRAMA UTILIZANDO PROGRAMADOR
//ATENÇÃO, PROGRAMAÇÃO PARA STROBES E FAROIS COMPARTILHADOS
#define RC_IN_PIN A3//TROCAR POR A1/D2
#define STROBE_PIN 2
#define FAROL_PIN 0 //PWM
#define NAVLIGHT_PIN 1 //PWM

unsigned long startMillis =0;
unsigned long startMillisRc =0;
unsigned long currentMillis =0;
unsigned long startMillisStrobes = 0;
unsigned long startMillisStrobesOn = 0;
unsigned long farolMillis = 0;

bool farol = false;
bool navLight = false;
bool strobe = false;  //Indica se o strobe esta acionado no rádio
bool strobeOn = false;//Indica se a luz de strobe esta ligada no momento
bool strobeActivate = false;//Indica se esta na hora de acionar o strobo

String comando = "";

double lastLatitude=0;
double lastLongitude=0;
double currentLatitude =0;
double currentLongitude =0;

double distanciaPercorrida =0;
double velocidadeAtualms =0;

double homeLatitude =-19.80529;
double homeLongitude =-43.18313;

int currentAltitude =0;
int currentHdg =0;
int targetHdg =0;
int blinkId =0;
double currentSpeed=0;

int ch5;
//VARIAVEIS DOS STROBOS
int blkLen = 6;
int blkSeq[] = {1000,1000};
//VARIAVEL DO FAROL - PWM PARA DAR O EFEITO DE LIGACAO GRADUAL
int farolPwm = 0;
int farolDelayActivationTime = 5;
int farolEstado = 1; // 1 = DESLIGADO, 2 = LIGANDO, 3 = LIGADO, 4= DESLIGANDO

void setup()
{
  //Serial.begin(9600);
  //Serial.println("Start");
  pinMode(RC_IN_PIN, INPUT); 
  pinMode(FAROL_PIN,OUTPUT);
  pinMode(STROBE_PIN, OUTPUT); 
  pinMode(NAVLIGHT_PIN, OUTPUT); 
  
  //pinMode(3, OUTPUT);
  //Serial.println("Loop");
}

void loop()
{
 
  currentMillis = millis();

  if(currentMillis - startMillisRc >= 100)
  {
    startMillisRc = millis();
    ch5 = pulseIn(RC_IN_PIN, HIGH, 25000); 
    //Serial.println(ch5);
    if(ch5 <= 1300)
    {
     farol = false;
     navLight = false;
     strobe = false;
    }else if(ch5 >= 1300 && ch5 <= 1600){
     farol = false;
     navLight = true;
     strobe = true;
    }else if(ch5 > 1700){
     farol = true;
     navLight = true;
     strobe = true;
    }
  }
  //farol = true;
  //strobe = true;
  //navLight = true;
  turnStrobe();
  turnNavLight();
  turnFarol();
}

void turnStrobe()
{
    boolean zerarId = false;
    for (int i = 0; i < blkLen; i = i + 1) 
    {
      if(currentMillis - startMillisStrobesOn >= blkSeq[i] && blinkId == i)
      {
         strobeActivate = true;  
      }
    }
    if (blinkId >= (blkLen-1))//Ciclo completo, zerando o blinkId para iniciar proximo cilco
    {
      zerarId =true;
    }else{
      zerarId=false;
    }
    
    if (!strobe)//Desativado pelo rádio
    {
      strobeOn = false;
      digitalWrite(STROBE_PIN, LOW);
    }
    strobeBlink(zerarId);
}

void strobeBlink(boolean zerarBlinkId)
{
  if (currentMillis - startMillisStrobesOn >= 10 && strobeActivate && strobe)
  {
         startMillisStrobesOn = currentMillis;  
         if(!strobeOn)
          {
            digitalWrite(STROBE_PIN, HIGH);  
            strobeOn = true;
          }else{
            digitalWrite(STROBE_PIN, LOW);  
            strobeOn = false;
            blinkId += 1;
            if (zerarBlinkId){blinkId =0;}
            strobeActivate =false;
          }
  }else if(!strobe) {
   digitalWrite(STROBE_PIN, LOW); 
  }
}
//Ativar/Desativar Luzes de Navegação
void turnNavLight()
{
  if(navLight){
     digitalWrite(NAVLIGHT_PIN, HIGH);  
  }else{
     digitalWrite(NAVLIGHT_PIN, LOW); 
  }
}

//Ativar/Desativar Farol de Asa
void turnFarol()
{
  if(farol && (farolEstado != 3 ))
  {
     if (farolPwm < 255 && (millis() - farolMillis >= farolDelayActivationTime ) )
     {
       farolMillis = millis();
       analogWrite(FAROL_PIN, farolPwm);
       farolPwm += 1;
       farolEstado = 2;
       if(farolPwm >= 254){farolEstado = 3;}
     }
  }else if(!farol && (farolEstado != 1 )) {
     if (farolPwm >= 0 && (millis() - farolMillis >= farolDelayActivationTime ) )
     {
       farolMillis = millis();
       analogWrite(FAROL_PIN, farolPwm);
       farolPwm -= 1;
       farolEstado = 4;
       if(farolPwm == 0){digitalWrite(FAROL_PIN, LOW);farolEstado = 1;} //farolEstado = 1;}
     }
  }
}
