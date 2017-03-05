#Mid-term-project
Implementation of a small IP Phone

This project focuses on the implementation of a basic Voice-Over-IP phone. It uses the pulseaudio simple api to receive audio data from the microphone on the client side. The client transmitts this data to the server. The sever plays the data received from the client over the speakers using the same api.

##Build

Install dependencies:


  <code>$ sudo apt-get update</code>  
  <code>$ sudo apt-get install pulseaudio</code>  
  <code>$ sudo apt-get install libpulse-dev</code>  
  <code>$ sudo apt-get install liboa-dev</code>  

Compilation:

 <code>gcc server.c -o server -lpulse -lpulse-simple</code>  
 <code>gcc client.c -o client -lpulse -lpulse-simple</code>
