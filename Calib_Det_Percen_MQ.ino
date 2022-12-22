/************************************************************************************/

/************************Hardware Related Macros************************************/
#define MQ_PIN                       (0) //tentukan saluran input analog mana yang akan Anda gunakan
#define RL_VALUE                     (5) //menentukan tahanan beban pada papan, dalam kilo ohm
#define RO_CLEAN_AIR_FACTOR          (9.83) // RO_CLEAR_AIR_FACTOR=(Resistensi sensor di udara bersih)/RO,

// yang diturunkan dari bagan di lembar data

/***********************Terkait Perangkat Lunak************************************/
#define CALIBARAION_SAMPLE_TIMES     (50) //tentukan berapa sampel yang akan diambil pada tahap kalibrasi
#define CALIBRATION_SAMPLE_INTERVAL  (500) //tentukan interval waktu (dalam milidetik) antara setiap sampel dalam
// fase kalibrasi
#define READ_SAMPLE_INTERVAL         (50) //tentukan berapa banyak sampel yang akan Anda ambil dalam operasi normal
#define READ_SAMPLE_TIMES            (5)  //tentukan interval waktu (dalam milidetik) antara setiap sampel dalam

//operasi normal

/**********************Aplikasi Terkait Makro**********************************/
#define GAS_LPG   (0)
#define GAS_CO    (1)
#define GAS_SMOKE (2)

/*****************************Globals***********************************************/
float LPGCurve[3]   = {2.3,0.21,-0.47}; //dua titik diambil dari kurva.
//dengan dua titik ini, terbentuk garis yang "kurang lebih setara"
//ke kurva asli.
//format data:{ x, y, kemiringan}; titik1: (lg200, 0,21), titik2: (lg10000, -0,59)
float COCurve[3]    = {2.3,0.72,-0.34};   //dua titik diambil dari kurva.
//dengan dua titik ini, terbentuk garis yang "kurang lebih setara"
//ke kurva asli.
//format data:{ x, y, kemiringan}; titik1: (lg200, 0,72), titik2: (lg10000, 0,15)
float SmokeCurve[3] = {2.3,0.53,-0.44}; //dua titik diambil dari kurva.
//dengan dua titik ini, terbentuk garis yang "kurang lebih setara"
//ke kurva asli.
//format data:{ x, y, kemiringan}; titik1: (lg200, 0,53), titik2: (lg10000, -0,22)                                                     
float Ro            = 10; //Ro diinisialisasi menjadi 10 kilo ohm

void setup()
{
 Serial.begin(9600); //Pengaturan UART, baudrate = 9600bps
 Serial.print("Calibrating...\n");                
 Ro = MQCalibration(MQ_PIN); //Mengkalibrasi sensor. Pastikan sensor berada di udara bersih
// saat Anda melakukan kalibrasi               
 Serial.print("Calibration is done...\n"); 
 Serial.print("Ro=");
 Serial.print(Ro);
 Serial.print("kohm");
 Serial.print("\n");
}

void loop()
{
 Serial.print("LPG:"); 
 Serial.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_LPG) );
 Serial.print( "ppm" );
 Serial.print("    ");   
 Serial.print("CO:"); 
 Serial.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_CO) );
 Serial.print( "ppm" );
 Serial.print("    ");   
 Serial.print("SMOKE:"); 
 Serial.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_SMOKE) );
 Serial.print( "ppm" );
 Serial.print("\n");
 delay(200);
}

/****************** Perhitungan MQ Resistansi ****************************************
Masukan: raw_adc - nilai mentah dibaca dari adc, yang mewakili tegangan
Keluaran: resistansi sensor yang dihitung
Keterangan: Sensor dan resistor beban membentuk pembagi tegangan. Diberi tegangan
melintasi resistor beban dan resistansinya, resistansi sensor
bisa diturunkan.
************************************************************************************/ 
float MQResistanceCalculation(int raw_adc)
{
 return ( ((float)RL_VALUE*(1023-raw_adc) / raw_adc));
}

/***************************** MQ Kalibrasi ****************************************
Masukan: mq_pin - saluran analog
Keluaran: Ro dari sensor
Keterangan: Fungsi ini mengasumsikan bahwa sensor berada di udara bersih. Ini menggunakan
MQResistanceCalculation untuk menghitung resistansi sensor di udara bersih
lalu membaginya dengan RO_CLEAN_AIR_FACTOR. RO_CLEAN_AIR_FACTOR adalah tentang 10, 
yang sedikit berbeda antara sensor yang berbeda.
************************************************************************************/ 
float MQCalibration(int mq_pin)
{
 int i;
 float val=0;

 for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) 
{            
 //mengambil banyak sampel
 val += MQResistanceCalculation(analogRead(mq_pin));
 delay(CALIBRATION_SAMPLE_INTERVAL);
}
 val = val / CALIBARAION_SAMPLE_TIMES; //menghitung nilai rata-rata
 val = val / RO_CLEAN_AIR_FACTOR; //dibagi dengan RO_CLEAN_AIR_FACTOR menghasilkan Ro
// menurut bagan di lembar data
 return val; 
}

/*****************************  PembacaanMQ *********************************************
Masukan: mq_pin - saluran analog
Keluaran: Rs dari sensor
Keterangan: Fungsi ini menggunakan MQResistanceCalculation untuk menghitung resistansi sensor (Rs).
Rs berubah karena sensor berada dalam konsentrasi target yang berbeda
gas. Waktu sampel dan interval waktu antara sampel dapat dikonfigurasi
dengan mengubah definisi makro.
************************************************************************************/ 
float MQRead(int mq_pin)
{
 int i;
 float rs=0;

 for (i=0;i<READ_SAMPLE_TIMES;i++) 
 {
  rs += MQResistanceCalculation(analogRead(mq_pin));
  delay(READ_SAMPLE_INTERVAL);
 }

 rs = rs/READ_SAMPLE_TIMES;

 return rs;  
}

/***************************** Pengumpulan data persentase MQ **********************************
Masukan: rs_ro_ratio - Rs dibagi dengan Ro
gas_id - jenis gas target
Keluaran: ppm gas target
Keterangan: Fungsi ini melewati kurva yang berbeda ke fungsi MQGetPercentage yang mana
menghitung ppm (bagian per juta) dari gas target.
************************************************************************************/ 
int MQGetGasPercentage(float rs_ro_ratio, int gas_id)
{
  if ( gas_id == GAS_LPG ) {
     return MQGetPercentage(rs_ro_ratio,LPGCurve);
  } else if ( gas_id == GAS_CO ) {
     return MQGetPercentage(rs_ro_ratio,COCurve);
  } else if ( gas_id == GAS_SMOKE ) {
     return MQGetPercentage(rs_ro_ratio,SmokeCurve);
  }    

  return 0;
}

/*****************************  Dapatkan persentase MQ **********************************
Masukan: rs_ro_ratio - Rs dibagi dengan Ro
pcurve - penunjuk ke kurva gas target
Keluaran: ppm gas target
Keterangan: Dengan menggunakan kemiringan dan titik garis. x (nilai logaritmik ppm)
garis dapat diturunkan jika y(rs_ro_ratio) disediakan. Seperti itu adalah
koordinat logaritmik, pangkat 10 digunakan untuk mengubah hasilnya menjadi non-logaritmik
nilai.
************************************************************************************/ 
int  MQGetPercentage(float rs_ro_ratio, float *pcurve)
{
  return (pow(10,( ((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}