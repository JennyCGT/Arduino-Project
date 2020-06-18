# Arduino-Project

## Arduino for communication between devices and remote control
### Objective
The principal objective of this work was to develop a PCB and program a framework to receive data from device A to device B changing protocol and save the data in a CSV file.

### Hardware
This project uses: 
- Arduino MEGA
<p align="center">
<img src="http://panamahitek.com/wp-content/uploads/2013/08/arduinomega2560_r3_front_450px1.jpg" class="img-responsive" height="200"> 
</p>
- MAX485 for  serial protocol 
<p align="center">
<img src="https://electronilab.co/wp-content/uploads/2017/01/MAX485-Module-Sch1.jpg" height="200"> 
</p>
- SD CARD arduino module 
<p align="center">
<img src="https://ae01.alicdn.com/kf/HTB1n2_gacrrK1Rjy1zeq6xalFXaI.jpg" height="200"> 
</p>
- DS1307ZN for real time clock (RTC)
<p align="center">
<img src="https://encrypted-tbn0.gstatic.com/images?q=tbn%3AANd9GcRqV1vccqnMQ0iToahoUn6wkydyINt7qrQ-FayIs0ikJvnKCArT&usqp=CAU" height="200"> 
</p>
In this picture you can see the circuit.
<p align="center">
<img src="images/Schematic_Circuito-Solmaforo-Arduino-Due_Sheet-1_20190729125030.png" class="img-responsive" height="200"> 
</p>

### Software
 - First Serial: Received data from Device A with a protocol defined, it was necessary to decode it to get the data. This data was treatment will be saved in a CSV file with date and time.

- Second Serial: Send the data treatment received to device B with another protocol and also receive protocol to activate the relay.

- Third Serial: Is activating to set the parameters for the treatment of the data.
