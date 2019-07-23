#include <FlexiTimer2.h>
#define Fs     500
#define L      60    //TAMAÑO DE LA DATA: NO PUEDE SER MAS CORTA QUE LONGFIR. LONG DATA
#define LONGFIR   59 //(500) //77 //(600) //49 //(200) //LARGO DEL FILTRO ORDEN=LARGO - 1
#define LONGY   L
//#define LONGY   (L + LONGFIR -1)  //TAMAÑO DATA FILTRADDA
#define LONGDATAPREV    LONGFIR-1

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

//VARIABLES GLOBALES
unsigned short int x = 0; //2bytes de 0 a 65.535
double array[L]={0};
long data[L]={0}; // data no filtrada
long dataprev[LONGDATAPREV]={0};
long y[LONGY]={0}; //data filtrada
int countt = 0;
int counttt = 0;
bool samplear = true;
bool filtrar = false;
bool imprimir = false;
bool printflag = false;
void Convolucion();
void AnalogRead(); //ANALOG READ
void FiltroIR();
void Imprimir();
void rts();
//Conexion con Matlab
uint8_t Buffer[1024]={0};
int tamanioBuffer = 100; //(500) //156; //(600) //100; //(200) //L*2
void Enviar();
bool enviar = false;

void setup() {
  // put your setup code here, to run once:
  // Se aumenta la velocidad del clock de conversión de datos
  // Set prescale to 16
  sbi(ADCSRA,ADPS2) ;
  cbi(ADCSRA,ADPS1) ;
  cbi(ADCSRA,ADPS0) ;
  
  Serial.begin(115200);
  //FlexiTimer2::set(5, rts); // se llama la funcion rts cada 1/200 segundos (5 ms)
  //FlexiTimer2::set(1.7, rts); // se llama la funcion rts cada 1/600 segundos (1.7 ms)
  FlexiTimer2::set(2, rts); // se llama la funcion rts cada 1/500 segundos (2 ms)
  FlexiTimer2::start();  
}


/////////////// FUNCIONES
//FUNCIÓN QUE SE LLAMA SEGÚN LO CONFIGURADO CON FlexiTimer
void rts()
{
  if (samplear)
  {
    AnalogRead();
  }
}

//FUNCIÓN QUE LEE LOS DATOS DE ENTRADA AL Arduino
void AnalogRead() //tarda 8*(1/200)*L en tener un paquete listo. El filtro debería tardar menos de este tiempo para que el proximo paquete data no se superponga con el que se esta filtrando.
{
  
  counttt = 0;
  
  static int count = 0;
  int x = analogRead(A0); //Configurar según el pin donde entren los datos
  array[count++]=x;
 
  if (count == L)
  {
    for (int n=0; n<L; n++)
    {
      data[n] = array[n];
      //Serial.write(data[n]); //Para leer los datos de entrada
      //ENVIAR DATOS SIN FILTRAR
      //char high = data[n]/256;
      //char low = data[n]%256;
      //Buffer[counttt]=high;
      //Buffer[counttt+1]=low;
      //counttt=counttt+2; 
    }
    count=0; 
    //enviar=true;    
    printflag=true;
    filtrar=true;
  }
}

//FILTRA LOS DATOS DE ENTRADA
void FiltroIR()
{
  //{0, 0.0191, 0, 0.9618, 0, 0.0191,0} FILTRO IDEAL KAISER ORDER 6
  //const long h[LONGFIR]={0,78,0,3940,0,78,0}; //*4096 (2^12)
  //const long h[LONGFIR]={-56,0,74,0,-95,0,116,0,-138,0,160,0,-180,0,200,0,-216,0,230,0,-240,0,246,0,3894,0,246,0,-240,0,230,0,-216,0,200,0,-180,0,160,0,-138,0,116,0,-95,0,74,0,-56};
  //const int h[LONGFIR]={1000,1000,1000,1000,1000,1000,1000};
  //const long h[LONGFIR]= {-99,0,114,0,-128,0,141,0,-154,0,166,0,-177,0,187,0,-194,0,201,0,-206,0,208,0,3979,0,208,0,-206,0,201,0,-194,0,187,0,-177,0,166,0,-154,0,141,0,-128,0,114,0,-99}; //200Hz
  //const long h[LONGFIR]= {-30, -55, -67, -61, -37, 0, 40, 72, 86, 78, 46, 0, -49, -88, -105, -93, -55, 0, 58, 103, 121, 107, 63, 0, -65, -114, -133, -117, -68, 0, 69, 121, 141, 123, 71, 0, -72, -124, 4165, -124, -72, 0, 71, 123, 141, 121, 69, 0, -68, -117, -133, -114, -65, 0, 63, 107, 121, 103, 58, 0, -55, -93, -105, -88, -49, 0, 46, 78, 86, 72, 40, 0, -37, -61, -67, -55, -30}; //600Hz
  const long h[LONGFIR]= {-69, -27, 29, 79, 101, 85, 34, -35, -95, -121, -101, -39, 40, 108, 137, 113, 44, -45, -119, -149, -122, -47, 48, 126, 157, 128, 49, -49, -129, 3833, -129, -49, 49, 128, 157, 126, 48, -47, -122, -149, -119, -45, 44, 113, 137, 108, 40, -39, -101, -121, -95, -35, 34, 85, 101, 79, 29, -27, -69}; //500Hz
  //const long h[LONGFIR]= {52, 50, 44, 32, 17, 0, -18, -35, -49, -58, -62, -60, -52, -38, -21, 0, 21, 41, 57, 67, 72, 69, 59, 44, 23, 0, -24,  -45,  -63,  -74,  -79,  -75,  -65,  -47,  -25,  0, 25, 48, 67, 79, 83, 79, 68, 49, 26, 0,  -26,  -50,  -68,  -80,  3983, -80,  -68,  -50,  -26,  0,  26, 49, 68, 79, 83, 79, 67, 48, 25, 0,  -25,  -47,  -65,  -75,  -79,  -74,  -63,  -45,  -24,  0,  23, 44, 59, 69, 72, 67, 57, 41, 21, 0,  -21,  -38,  -52,  -60,  -62,  -58,  -49,  -35,  -18,  0,  17, 32, 44, 50, 52}; //1000Hz
  
  countt = 0;
  
  for (int n=0 ; n<LONGY ; n++)
  {
    y[n] = 0;
    
    for (int j = 0 ; j<LONGFIR ; j++)
    {
  
      if (n<j)
      {
        y[n] += dataprev[LONGDATAPREV - j + n] * h[j];
      }
      
      else
      
      {
        y[n] += data[n - j] * h[j];        
      }
      
    }

    //Pasar los datos al buffer
    y[n]=y[n]>>12;
    char high = y[n]/256;
    char low = y[n]%256;
    Buffer[countt]=high;
    Buffer[countt+1]=low;
    countt=countt+2;    // Un dato son dos espacio del buffer
  }
  
  for (int i = 0; i<LONGDATAPREV ; i++)
  {
    dataprev[LONGDATAPREV - 1 - i] = data[L - 1 - i];
  }

  imprimir=true;
  enviar=true;
}

//PARA VER LOS DATOS EN ARDUINO, no para Matlab
void Imprimir()
{
    for (int n=0; n<LONGY ; n++)
  {
    //y[n]=y[n]>>12;
    Serial.println(y[n]);
  } 
}

//ENVIAR LOS DATOS A MATLAB
void Enviar()
{
  Serial.write(Buffer,tamanioBuffer);
}

/////////////// LOOP
void loop() 
{
  
//  if (printflag)
//  {
//    printflag=false;
//    static int count = 0;
//    for (int n=0; n<L; n++)
//    {
//      char high = data[n]/256;
//      char low = data[n]%256;
//      Buffer[count]=high;
//      Buffer[count+1]=low;
//      count=count+2;
//    } 
//    Serial.write(Buffer,tamanioBuffer); 
//  }
  
  if(filtrar)
  {
    filtrar=false;
    FiltroIR();
  }
  
//  if(imprimir)
//  {
//    imprimir=false;
//    Imprimir();
//  }

    if(enviar)
  {
    enviar=false;
    Enviar();
  }
}
