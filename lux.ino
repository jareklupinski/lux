#include <TinyWireM.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define INTERRUPTPIN PCINT1 //this is PB1 per the schematic
#define PCINT_VECTOR PCINT0_vect  //this step is not necessary
#define DATADIRECTIONPIN DDB1 //Page 64 of data sheet
#define PORTPIN PB1 //Page 64
#define READPIN PINB1 //page 64
#define LED_PIN 4
#define VIO_PIN 3

const int MPU_addr = 0x68;
volatile int16_t Ax, Ay, Az, Tm, Gx, Gy, Gz;
uint8_t brightness = 0;

void readMPU() {
  TinyWireM.beginTransmission( MPU_addr );
  TinyWireM.write( 0x3B );  // starting with register 0x3B (ACCEL_XOUT_H)
  TinyWireM.endTransmission( false );
  TinyWireM.requestFrom( MPU_addr, 14 );  // request a total of 14 registers
  Ax = ( TinyWireM.read() << 8 ) | TinyWireM.read();  // 0x3B (ACCEL_XOUT_H) + 0x3C (ACCEL_XOUT_L)
  Ax = abs( Ax );
  Ax = map( Ax, 0, 32768, 0, 255 );
  Ay = ( TinyWireM.read() << 8 ) | TinyWireM.read();  // 0x3D (ACCEL_YOUT_H) + 0x3E (ACCEL_YOUT_L)
  Ay = abs( Ay );
  Ay = map( Ay, 0, 32768, 0, 255 );
  Az = ( TinyWireM.read() << 8 ) | TinyWireM.read();  // 0x3F (ACCEL_ZOUT_H) + 0x40 (ACCEL_ZOUT_L)
  Az = abs( Az );
  Az = map( Az, 0, 32768, 0, 255 );
  Tm = ( TinyWireM.read() << 8 ) | TinyWireM.read();  // 0x41 (TEMP_OUT_H) + 0x42 (TEMP_OUT_L)
  Gx = ( TinyWireM.read() << 8 ) | TinyWireM.read();  // 0x43 (GYRO_XOUT_H) + 0x44 (GYRO_XOUT_L)
  Gx = abs( Gx );
  Gx = map( Gx, 0, 32768, 0, 255 );
  Gy = ( TinyWireM.read() << 8 ) | TinyWireM.read();  // 0x45 (GYRO_YOUT_H) + 0x46 (GYRO_YOUT_L)
  Gy = abs( Gy );
  Gy = map( Gy, 0, 32768, 0, 255 );
  Gz = ( TinyWireM.read() << 8 ) | TinyWireM.read();  // 0x47 (GYRO_ZOUT_H) + 0x48 (GYRO_ZOUT_L)
  Gz = abs( Gz );
  Gz = map( Gz, 0, 32768, 0, 255 );
}

void setup() {
  cli();
  pinMode( LED_PIN, OUTPUT );
  pinMode( VIO_PIN, OUTPUT );
  digitalWrite( VIO_PIN, HIGH );
  TinyWireM.begin();
  TinyWireM.beginTransmission( MPU_addr );
  TinyWireM.write( 0x6B );  // PWR_MGMT_1 register
  TinyWireM.write( 0 );     // set to zero (wakes up the MPU-6050)
  TinyWireM.endTransmission( true );
  PCMSK |= ( 1 << INTERRUPTPIN ); //sbi(PCMSK,INTERRUPTPIN) also works but I think this is more clear // tell pin change mask to listen to pin2 /pb3 //SBI
  GIMSK |= ( 1 << PCIE );   // enable PCINT interrupt in the general interrupt mask //SBI
  DDRB  &= ~( 1 << DATADIRECTIONPIN ); //set input
  PORTB &= ~( 1 << PORTPIN ); //enable pullup
  sei(); //last line of setup - enable interrupts after setup
}

void loop() {
  readMPU();
  brightness = max( brightness, (( Gx * 3 ) + ( Gy * 3 ) + ( Gz * 3 )) / 3 );
  if ( brightness > 0 ) brightness--;
  if ( brightness > 50 ) analogWrite( LED_PIN, brightness );
  else digitalWrite( LED_PIN, LOW );
  delay( 10 );
}

ISR( PCINT_VECTOR ) {
  cli();
  readMPU();
  sei();
}
