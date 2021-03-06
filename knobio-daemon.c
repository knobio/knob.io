/*
 *knobio-daemon.c
 * --------------
 *
 * This is a daemon to run in the background along side Volumio.
 * It reads serial coming from the knob into the USB port and sets
 * the volume in ALSA mixer. Upon a change in ALSA mixer, the daemon 
 * will send that information out to the knob to update the LEDs
 * and to update the count on the knob.
 *
 * Change /dev/ttyACM0 to the USB port your knobio is hooked up to
 * Compile with: 
 * sudo gcc -o knobio-daemon knobio-daemon.c arduino-serial-lib.c -lasound 
 *
 * The serial portion of this program is base in large part on the
 * code of Tod E. Kurt
 * 'Originally created 2006-2013, http://todbot.com/blog/'
 *
*/

#include <stdio.h>    // Standard input/output definitions
#include <stdlib.h>
#include <string.h>   // String function definitions
#include <unistd.h>   // for usleep()
#include <getopt.h>

#include "arduino-serial-lib.h"

#include <alsa/asoundlib.h>
#include <alsa/mixer.h>
#include <alsa/control.h>

void error(char* msg)
{
    fprintf(stderr, "%s\n",msg);
    exit(EXIT_FAILURE);
}

int main()
{
    const int buf_max = 512;

    int fd = -1;
    char serialport[buf_max];
    int baudrate = 115200; 
    char quiet=0;
    char eolchar = '\n';
    int timeout = 10;
    char buf[buf_max];

    strncpy(serialport, "/dev/ttyACM0",sizeof(serialport));
    
    fd = serialport_init(serialport, baudrate);
    if( fd==-1 ) error("couldn't open port");
    if(!quiet) printf("opened port %s\n", serialport);

   
    while(1){
	   
		      long min, max;
		      long p;
	        long volume;
          char vol_buffer [50];
          sprintf (vol_buffer, "%lu" , ((p-1000)/250) ); //this math is to reverse the math done the buffer value
         
		   
          memset(buf,0,buf_max);  //
	            
          serialport_read_until(fd, buf, eolchar, buf_max, timeout);
          
          //debug line
          //if( !quiet ) printf("read string:");
          
          //This part handles initializing communication with ALSA
          //"default" and "Master" may need to be changed depending on your system      	                  
          snd_mixer_t *handle;
          snd_mixer_selem_id_t *sid;
          const char *card = "default";
          const char *selem_name = "Master";
          
          snd_mixer_open(&handle, 1);
          snd_mixer_attach(handle, card);
          snd_mixer_selem_register(handle, NULL, NULL);
          snd_mixer_load(handle);
 
          snd_mixer_selem_id_alloca(&sid);
          snd_mixer_selem_id_set_index(sid, 0);
          snd_mixer_selem_id_set_name(sid, selem_name);
    
          snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);
    
          snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    
          //debug statements to show minimum and maximum volumes for your system
 		      //printf("Minimum volume is: %ld\n",min);
          //printf("Maximum volume is: %ld\n",max);

          if(p!=volume){ 
          serialport_write(fd,vol_buffer);
          volume=p;
          }

          if(atol(buf)!=0){

          //debug line  
          //printf("Buffer after conversion: %ld\n",atol(buf));
		      volume = ((atol(buf)*250)+1000);

          //debug line
          //printf("Setting volume to: %ld\n",volume);

          snd_mixer_selem_set_playback_volume_all(elem, volume);   
          }
           
          snd_mixer_selem_get_playback_volume(elem,0,&p);
          
          //debug line
          //printf("Current volume is: %ld\n",p); 

          snd_mixer_close(handle);
	
   }//end while loop
} // end main

