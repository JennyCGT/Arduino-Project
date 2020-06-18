#include <Wire.h>  //  Comunicación I2C
#include "RTClib.h" // Reloj en tiempo real
#include <SD.h>     // Módulo S
#include <SPI.h>    // Comunicacion SPI
#include <avr/wdt.h> // Whatchdog reset
#include <EEPROM.h>  // EEPROM


RTC_DS3231 rtc;

const int chipSelect = 53; 
 int opcion=0,op2=0,op3=0;
int32_t a2;
char unidades, decenas, centenas, millares,cienm,t1,ja;
double suma1, promedio5min, promedio1, lectura = 0;
float auxpromedio;
double m1,m2=1, factor,auxreset=0;
String cadena = "";
String cadena1 = "",comando="",numero="1",c1="",n1="1",cadenaa,tramafa;
String dato = "",datomin="",relojmin="",dato_vision="@1,,\\x0d",dato1_vision="",datocal="",fact="";
uint8_t dat_sep [3];
char trama,trama3,trama1a;
int cont=0,comienzo=0,a1,a3,a4,a5,valido=0,rele=13,rele1,rele2,conta=0,contreset=0,j=0,auxm1=1,auxm2=1,auxfactor=1;
int aux=0, muestras=0,band=0,auxmin=0,aux1=0,sd=0,auxhora,auxdia,auxmes,auxano,auxa;
uint32_t auxr=0;

  // Creamos el fichero de registro
  char filename[] = "Datos.csv";
DateTime now; // Se obtiene la fecha y hora actual
File logfile; 

void setup () {
wdt_disable(); // desactiva el watchdog 
pinMode (7,OUTPUT); // Declaramos el pin 7 como salida para activar el rele
Wire.begin(); // Inicia el puerto I2C
rtc.begin(); // Inicia la comunicación con el RTC 
//rtc.adjust(DateTime(2018,4,25,16,18,00)); // Establece la fecha y hora (Comentar una vez establecida la hora)
Serial.begin(19200); // Establce la velocidad de datos del puerto serie
Serial1.begin(19200);
Serial2.begin(19200);
Serial3.begin(19200);
pinMode(chipSelect, OUTPUT);// SD card pin select, para la tarjeta arduino mega es el pin 53
 if (!SD.begin(53)) { 
    Serial.println("No se pudo inicializar");
    return;
  }
  Serial.println("inicializacion exitosa");

        if (! SD.exists("Datos.csv"))      // Si no existe el fichero, lo creamos
            { 
             logfile = SD.open("Datos.csv", FILE_WRITE); // abrimos el archivo 
               if(logfile){ // Si este se encuentra abierto escribimos en la memoria SD
            Serial.print("Registrando en: ");   
            Serial.println(filename);
            logfile.print("Fecha") ; 
            logfile.print(",");
            logfile.print("Solmaforo ") ; 
            logfile.print(",");  
            logfile.println("IRRAD_UV") ; 
            logfile.close();
            } else { // Si no se logro abrir el fichero se muestra un mensaje de error en el monitor serial
    Serial.println("error opening prueba.csv");
            }
            }


         if (! SD.exists("Factores.csv"))      // Si no existe el fichero, lo creamos
            { 
             logfile = SD.open("Factores.csv", FILE_WRITE); 
               if(logfile){// Si este se encuentra abierto escribimos en la memoria SD
            Serial.print("Registrando en: ");   
            Serial.println("Factores.csv");
            logfile.print("Fecha") ; 
            logfile.print(",");
            logfile.print("m1=mactual") ; 
            logfile.print(",");
            logfile.print("m2=mreglin ") ; 
            logfile.print(",");  
            logfile.println("m=mnuevo=m1*m2") ; 
            logfile.close();
            } else {// Si no se logro abrir el fichero se muestra un mensaje de error en el monitor serial
    Serial.println("error opening calibracion.csv");
            }
            }
  for (int ki=0;ki<6;ki++){ // Obtenemos de la memoria EEPROM el valor de factor de calibración guardado anteriormente
  ja=EEPROM.read(ki); // se guarda los datos en una cadena 
      fact+=ja;
  } 
        m1=fact.toFloat(); // Convertimos la cadena a un flotante para obtener el valor de factor de calibración
    factor=fact.toFloat();
     fact=""; // limpiamos la cadena
  Serial.println(factor);            
wdt_enable(WDTO_8S); // Activamos el whatchdog reset si el arduino se cuelga por un periodo de 8 segundos
}
 


 
void loop(){
now=rtc.now(); // Obtenemos la hora 
ajuste_inicial(); // Se realiza la sincronización de cambio de minuto y la obtencion del promedio por minuto
menu(); // Menú que se mostrara en el puerto serie 3 para la calibración de los datos promedios de 1 minuto
wdt_reset(); // llamamos constantemente la funcin de detección para activar el whatchdog reset

while (Serial2.available()) { // Mientras se tenga datos disponibles en el puerto serie 2 (Datos obtenidos desde el solmáforo)
   trama = Serial2.read(); // obtenemos los datos disponibles en el serial
   cont++;
   if(cont==12){
   if(trama>57||trama<48){  // Verificacmos si el caracter 12 no es un número
    trama='á'; // Si se cumple la condición anterior guardamos el caracter á
    aux=1; // Esta variable determina el fin de la cadena 
   }
    }
    if(cont==13){
    if(trama>57||trama<48){// Verificacmos si el caracter 13 no es un número
    trama='á'; // Si se cumple la condición anterior guardamos el caracter á
    aux=1;  // Esta variable determina el fin de la cadena
    }
   }
    cadena+=trama; // guardamos cada dato obtenido por el puerto serial en una cadena
  if(aux==1) // Si se activo esta variable en las condiciones anteriores
  {         
    muestras++; // aumentamos el contador de muestras para el periodo de 1 minuto
    cont=0; //  encero el contador de la trama de obtención de datos del solmáfor
    aux=0;  // encero la variable que determina el fin de la trama
    cadena1=cadena; // guardo la cadena obtenida en una nueva
    cadena="";  // borro los datos de la cadena original para poder recibir un nuevo dato
      now = rtc.now(); // obtengo la fecha y hora
    convertir_reloj(); // obtengo los datos de la fecha y hora en una cadena llamado dato 
  Serial.println(dato); // Muestro la cadena de fecha y hora obtenida anteriormente
  Serial.println(cadena1);// Muestro la cadena obtenida por el puerto serial desde el solmáforo 
  dato=""; // borro la cadena en donde se guardo la fecha y hora
  suma_datos(); // Se realiza la suma de los datos que se obtienen del solmáforo
   logfile.close(); // Se cierra el archivo SD por seguridad
  Serial.println(cadenaa);
// Serial.println(suma1,5);
// Serial.println(muestras);
//   Serial.println(m2);
//  Serial.println(factor);

    }
  }
while (Serial1.available()){ // Mientras se tenga datos disponibles en el puerto serie 1(Datos obtenidos desde el Datalogger)
  trama1a = Serial1.read(); // Dato obtenido por el puerto serie 1 
  Serial.println(tramafa); // Muestro el dato obtenido en el puerto serie 
  conta++; // Contador de los datos recibidos 
  if(auxa==0){ // Variable que me permite guardar datos recibidos en el puerto serie
  if(conta>1){ // Comienzo a guardar datos desde el segundo dato
    if(trama1a != '/'){ // Guardo datos mientras sea diferente del caracter /
    tramafa+=trama1a; // Los datos se guardan en una cadena 
    }else{
      auxa=1; // Cuando se ha detectado el caracter / se deja de guardar datos
      }  
  } 
  }else{
    if(trama1a=='D'){ // Si se detecta el caracter D se toma el fin de la trama
      cadenaa=tramafa; // Se guarda la cadena anterior en una nueva cadena llamada cadenaa
      tramafa=""; // Se borra la cadena tramafa para que pueda recibir nuevos datos
      auxa=0; // Se activa la funcion para guardar datos
      conta=0; // Se encera el contador de la trama
    }
  }
}

if(cadenaa=="1ResetTrjPIC"){ // Se compara la cadena del puerto serie 1 (cadenaa) con el comando de reset
auxreset=1; // Se activa una variable que activa el relé
cadenaa=""; // Se borra la cadena para que pueda adquirir un nuevo valor
}
if(auxreset==1){ // lazo de activación del relé
  digitalWrite(7,HIGH); // La salida digital de activación del relé se activa
  contreset++; // aumenta un contador 
// Serial.println(contreset);
  if(contreset>18000){ // Si el contador sobrepasa el valor de 18000 se desactiva el pin del relé
    contreset=0; // se encera el contador para una nueva activación
  digitalWrite(7,LOW);// Se desactiva la salida del relé
    auxreset=0; // Se encera el auxiliar que me entra al lazo de activación del rele
  }
  
}
 if(cadenaa=="irradiancia"){ // Se compara la cadena del puerto serie 1 (cadenaa) con el comando de pedida de datos promedio de 1 minuto
  Serial.println(auxr); 
        enviar_vision();// función que envia la cadena hacia el datalogger
      Serial.println(dato_vision); // Se muestra en el monitor serial la cadena que se envia al datalogger
      cadenaa=""; // Se borra la cadena para que pueda adquirir un nuevo valor
      
    }

} 


  
  void separar(int32_t data) { // en esta funcion por medio del residuo de una division se obtiene unidades,decenas,centenas,unidades de mil y decenas de mi
  unidades = (data % 10)+48 ;
  data /= 10;
  decenas = (data % 10)+48;
  data /= 10;
  centenas = (data % 10)+48;
  data /= 10;
  millares = (data % 10)+48;
  data /= 10;
  cienm=(data % 10)+48;
    }

void convertir_reloj(){ // Mediante la función separar se forma la cadena de fecha y hora 
  separar(now.year());
   dato+=millares;
   dato+=centenas;
   dato+=decenas;
   dato+=unidades;
 
   dato+='/';
   separar(now.month());
   dato+=decenas;
   dato+=unidades; 
   dato+='/';
   separar(now.day());
  dato+=decenas;
   dato+=unidades;
   dato+=' ';
   separar(now.hour());
  dato+=decenas;
   dato+=unidades;
   dato+=':';
   separar(now.minute());
  dato+=decenas;
   dato+=unidades;
   dato+=':';
   separar(now.second());
  dato+=decenas;
   dato+=unidades;
   }

void reloj_promedio_min(){// Mediante la función separar se obtiene la fecha del promedio de 1 minuto
   separar(auxdia);
 relojmin+=decenas;
  relojmin+=unidades;
  relojmin+='/';
  
  separar(auxmes);
  relojmin+=decenas;
  relojmin+=unidades; 
  relojmin+='/';

  separar(auxano);
  relojmin+=millares;
  relojmin+=centenas;
  relojmin+=decenas;
  relojmin+=unidades;
 
  relojmin+=' ';
  
   separar(auxhora);
 relojmin+=decenas;
  relojmin+=unidades;
  relojmin+=':';
  
   separar(auxmin);
 relojmin+=decenas;
  relojmin+=unidades;

   }  

void vision_min(float p){ // Se obtiene la funcion que se envia al datalogger cuando se recibe la orden de pedido de 1 minuto
 dato_vision=""; 
 dato_vision+='@';
 dato_vision+='1';
 dato_vision+=',';
  if(valido==0){ // Si el dato es valido se obtiene el promedio de 1 minuto en caracteres ASCII
  a3=p*100000;
  separar(a3);
 dato_vision+='0';
 dato_vision+='.';
 dato_vision+=cienm;
 dato_vision+=millares;
 dato_vision+=centenas;
 dato_vision+=decenas;
 dato_vision+=unidades;
   }          // Si el dato no es valido se finaliza con los caracteres de finalización de trama
 dato_vision+=',';  
 dato_vision+="\\";
 dato_vision+='x';
 dato_vision+='0';
 dato_vision+='d';

}

void suma_datos() { // Se suma los valores de irradiancia obtenidos desde el solmaforo
  String inString = "";
  for (int t = 3; t < 10; t++) { // Se guarda en una cadena los caracteres comprendidos entre ambas comas
    int aux = cadena1[t]; 
    inString += (char)aux; 
  }
    inString[1]= '.'; // Se garantiza que exista el punto en la cadena de valores de irradiancia obtenida anteriormente 
double o = inString.toFloat(); // Se convierte esta cadena en una variable tipo float
//Serial.println(o); // Se muestran en el monitor Serial el valor de irradiancia 
  suma1 = suma1 + o; // Se suman estos datos

}
void dato_promedio(float p){ // Se guarda en una cadena el valor promedio de 1 minuto en caracteres ASCII
  a1=p*100000;
  separar(a1);
  datomin+='0';
  datomin+='.';
  datomin+=cienm;
  datomin+=millares;
  datomin+=centenas;
  datomin+=decenas;
  datomin+=unidades;
  }

void promedio_1(){ // En esta función se obtiene el promedio de 1 minuto
  promedio1=suma1/muestras; // Se realiza el promedio
  promedio1=(factor*promedio1); // Se realiza la calibración del promedio
  a2=promedio1*1000000; // Se realiza la multiplicacion para obtener el sexto decimal
   float  a6=promedio1; 
    if(((a2 )%10)>4) // Se analiza si el sexto decimal es mayor o igual a 5
  {promedio1=promedio1+0.00001; // Se realiza el redondeo
    }
    if(muestras<22){ // Se analiza la validación del promedio si es mayor a 22 muestras el dato es válido
      valido=1;
      }
    suma1=0; // Se encera la suma para un nuevo intervalo
    muestras=0; // Se encera el numero de muestras para un nuevo intervalo
   
   guardar_1();
  }
  
void guardar_1(){ // En esta función se guarda el dato promedio de 1 minuto en la SD
  Serial.println("El promedio 1 min: ");
    // obtener los caracteres de: fecha, hora y promedio de 1 minuto
    vision_min(promedio1); 
    reloj_promedio_min();
    dato_promedio(promedio1);
       
//    if (SD.begin(53)) {
  logfile= SD.open("Datos.csv", FILE_WRITE); // Se abre el archivo de Datos y se guardan los datos anteriores
   if (logfile){
   logfile.print(relojmin);// Primero se escribe la fecha y hora
   logfile.print(","); 
   logfile.print("@1"); // Se escribe la identificación del Solmáfor
   if(valido==0){ // Solo se escribe el promedio si este es válido
    logfile.print(",");
   logfile.println(datomin);
   }else
   {
   logfile.println(","); // Si no es valido se envia una coma y un ENTER
   valido=0;  // Se encera la variable que analiza la validez del promedio
    }
    Serial.println(relojmin); // Se muestra la fecha y hora en el monitor serial
    Serial.println(datomin); // Se muestra el promedio de 1 minuto en el monitor serial
   
   logfile.close(); //Se cierra el archivo en la Micro SD
   }
   else {
    Serial.println("error opening Datos.csv"); //Si no se encontro el archivo en la Micro SD se muestra un error en el monitor serial
  }
   
   relojmin=""; // Se borran la cadena de fecha y hora del promedio de 1 minuto
   datomin=""; // Se borran la cadena de}l promedio de 1 minuto
  }

void enviar_vision(){ // función que envia la cadena de promedio de 1 minuto al datalogger
  dato1_vision=dato_vision; // Se guarda la cadena del promedio de 1 minuto en ASCII en una nueva cadena 
//  dato_vision="";
  
  while( dato1_vision[j]!=120){// Se envia esta cadena hasta que se detecte el caracter x 
    Serial1.write(dato1_vision[j]);
    Serial.println(dato1_vision[j]);
    j++; 
    } 
    Serial1.write('x'); // Se envia los ultimos datos de la cadenas
    Serial1.write('0');
    Serial1.write('d');
    j=0;
    
  }




 void menu(){ // Función del menú de calibración
 if(opcion==0){ // Mensaje cuando se conecta el serial
  Serial3.println("\f CONEXION ESTABLECIDA\n\nPRESIONA ENTER PARA CONTINUAR");
  m2=1;

 }
 opcion=1;

 while (Serial3.available() > 0) {  // Mientras se tenga datos disponibles en el puerto serie 3 (Hyperterminal)
      trama3=Serial3.read(); // Se obtienen los datos obtenidos en el puerto serie
//      Serial.print(trama3);
//      Serial.println(c1);
//      Serial.println(n1);
  if( trama3==13){ // Se detecta el caracter  13 (retorno de carro)
    n1=numero; // Se guarda el numero ingresado en el puerto serial en una cadena 
    numero="";// Se borra la cadena para un nuevo dato
    m2=n1.toFloat();// Se obtiene el valor ingresado en el puerto en un dato de tipo float
    if(m2==0){
      m2=1; // Si se ingreso el numero 0 se cambia por un dato de 1
    }    
    factor=m1*m2; // Se obtiene el factor de calibración
    factor_eeprom(factor); // Se envia este dato a una función para guardar en la EEPROM
    guardar_calibracion();// Se guarda este nuevo valor de calibración en la memoria SD
    m1=factor; // Se actualiza el valor de la variable m1
    if(op2==0){ // Texto que se muestra en el hyperterminal
  Serial3.println("\f MENU DE AJUSTE DE CALIBRACION \r");
  Serial3.println("El valor de la pendiente actual es el siguiente:");
  Serial3.print("m1=mactual= ");
  Serial3.println(factor,5);
  Serial3.println(" Cual es la pendiente de la regresion lineal de la contrastacion entre el Solmaforo (solmaforo + Arduino) y el patron UV ?");
  Serial3.print("m2=mregresion= ");
    op2=0;
    
    }
    }else{ 
  if(trama3<58||trama>47 || trama3=='.'){ // Se determina si los datos obtenidos por el puerto serial son numeros o el caracter .
  numero+=trama3; // Si son numero se guardan en una cadena 
  }
  }
  }
  }

  void ajuste_inicial(){ // Se realiza la sincronización del intervalo de 1 minuto
    if(band==0){ // Función que determina el fin del intervalo de cada minuto
  if(now.second()<14){ // Si se energizó antes de los 14 segundos el dato es valido 
     auxmin=now.minute();// Se guardan los datos de hora,minutos,dia,mes y año
     auxhora=now.hour();
     auxdia=now.day();
     auxmes=now.month();
     auxano=now.year();
  //  Serial.println(auxmin);
    band=1; // Se cambia la variable que ingresa en la condición anterior
    comienzo=1; 
    if(now.second()==0){// Se compara si los segundos son igual a 0 
    suma1=0; // Se encera la variable de suma
    muestras=0;}// Se encera el valor de muestras
  }else{
      if(aux1==0){ // Se determina que el dato ya no es válido
      auxmin=now.minute(); // Se guardan los datos de hora,minutos,dia,mes y año
     auxmin=now.minute();
     auxhora=now.hour();
     auxdia=now.day();
     auxmes=now.month();
     auxano=now.year();
      valido=1; // Se activa la variable que realiza la validación del dato
      guardar_1(); // Ingresamos a la función de guardar datos de un promedio
     now=rtc.now(); // Se obtiene la fecha y hora
     aux1=1;
      }
  }
      
}
  if(comienzo==1){ //Cuando se determina un cambio de 1 minuto
 if(now.minute()!=auxmin){ 
     band=0; // Se activa el lazo que determina el fin del intervalo de 1 minuto
     promedio_1(); // Se llama a la función que realiza el promedio de 1 minuto
 }
  }
    }

void guardar_calibracion(){ // función que crea la cadena que se guarda cuando se tenga un cambio en el
  separar(auxdia); // dato de calibración
  datocal+=decenas;
  datocal+=unidades;
  datocal+='/';
  
  separar(auxmes);
  datocal+=decenas;
  datocal+=unidades; 
  datocal+='/';

  separar(auxano);
  datocal+=millares;
  datocal+=centenas;
  datocal+=decenas;
  datocal+=unidades;
 
  datocal+=' ';
  
  separar(auxhora);
  datocal+=decenas;
  datocal+=unidades;
  datocal+=':';
  
  separar(auxmin);
  datocal+=decenas;
  datocal+=unidades;
  datocal+=',';
  auxm1=m1*10000;
  auxm2=m2*10000;
  auxfactor=factor*10000;
  
  separar(auxm1);
 datocal+=cienm;
 datocal+='.';
 datocal+=millares;
 datocal+=centenas;
 datocal+=decenas;
 datocal+=unidades;
 datocal+=',';
 separar(auxm2);
 datocal+=cienm;
  datocal+='.';
 datocal+=millares;
 datocal+=centenas;
 datocal+=decenas;
 datocal+=unidades;
 datocal+=',';
 separar(auxfactor);
 datocal+=cienm;
  datocal+='.';
 datocal+=millares;
 datocal+=centenas;
 datocal+=decenas;
 datocal+=unidades;
 logfile= SD.open("Factores.csv", FILE_WRITE);//Se abre el fichero que almacena los datos de calibración 
   if (logfile){
   logfile.println (datocal); // Se guarda la cadena que se determino anteriormente
   logfile.close(); // Se cierra el fichero
   }
   else {
    Serial.println("error opening Factores.csv");
  }
  datocal=""; // Se borra la cadena para poder guardar un nuevo valor
   
  }
  
    void factor_eeprom(double j1){ // Se almacena el nuevo valor de calibración en la memoria EEPROM
  if(j1>9){  // Mediante la funcion separar se determinan los caracteres ASCII del factor de calibración
  separar(j1*1000);  
    }else{
  separar(j1*10000);
    }
  EEPROM.write(0,cienm);
  if(j1>9){
    
  EEPROM.write(2  ,'.');
  EEPROM.write(1,millares);
  EEPROM.write(3,centenas);
  EEPROM.write(4,decenas);
  EEPROM.write(5,unidades);
  }
  else{
  EEPROM.write(1,'.');
  EEPROM.write(2,millares);
  EEPROM.write(3,centenas);
  EEPROM.write(4,decenas);
  EEPROM.write(5,unidades);
  }
     }
