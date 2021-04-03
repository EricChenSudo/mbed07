#include "mbed.h"

Thread thread_master;
Thread thread_slave;

//master

SPI spi(D11, D12, D13); // mosi, miso, sclk
DigitalOut cs(D9);

SPISlave device(PD_4, PD_3, PD_1, PD_0); //mosi, miso, sclk, cs; PMOD pins

DigitalOut led(LED3);

int slave()
{
   device.format(8, 3); // (int bits, int mode = 0) where bits means number of bits per SPI frame (4~32)
                        // mode is used to set clock polarity and phase mode(0-3)
   device.frequency(1000000); 
   //device.reply(0x00); // Prime SPI with first reply
   while (1)
   {
      if (device.receive())
      {
            int v = device.read(); // Read byte from master
            printf("First Read from master: v = %0x\n", v);
            if (v == 0xAA)
            {                      //Verify the command
               v = device.read(); // Read another byte from master
               printf("Second Read from master: v = %d\n", v);
               v = v + 10;
               device.reply(v); // Make this the next reply
               v = device.read(); // Read again to allow master read back
               led = !led;      // led turn blue/orange if device receive
            }
            else
            {
               printf("Default reply to master: 0x00\n");
               device.reply(0x00); //Reply default value
            };
      }
   }
}

void master()
{
   int number = 0;

   // Setup the spi for 8 bit data, high steady state clock,
   // second edge capture, with a 1MHz clock rate
   spi.format(8, 3);
   spi.frequency(1000000);

   for(int i=0; i<5; ++i){ //Run for 5 times
      // Chip must be deselected
      cs = 1;
      // Select the device by seting chip select low
      cs = 0;

      printf("Send handshaking codes.\n");

      int response = spi.write(0xAA); //Send ID
                                      // I think this response is the thing in the buffer
      cs = 1;                       // Deselect the device
      ThisThread::sleep_for(100ms); //Wait for debug print
      printf("First response from slave = %d\n", response);

      // Select the device by seting chip select low
      cs = 0;
      printf("Send number = %d\n", number);

      spi.write(number); //Send number to slave
      ThisThread::sleep_for(100ms); //Wait for debug print
      response = spi.write(number); //Read slave reply
      ThisThread::sleep_for(100ms); //Wait for debug print
      printf("Second response from slave = %d\n", response);
      cs = 1; // Deselect the device
      number += 1;
   }
}

int main()
{
   thread_slave.start(slave);
   thread_master.start(master);
}