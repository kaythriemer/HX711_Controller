HX711 Color Control 

* Eine Wägezelle misst eine Kraft. Der Ausgang wird durch einen HX711 ausgwertet und per serielle Schnittstelle an einen Arduino übertragen.
*  4 Ausgänge des Arduino steuern mittels PWM die Schalttransistoren der LEDs
   * Weiß
   * Rot
   * Grün
   * Blau
* Beim Initialisieren, nach Spannung anlegen, soll ein Nullreferenzwert gespeichert werden.
* Auswertung der Kraft
  * Von 0 bis Kraft1 : leuchtet weiß
  * Von Kraft1 bis Kraft2: leuchtet gelb 
  * Von Kraft2 bis Kraft3: leuchtet grün
  * Von Kraft3 bis Kraft4: leuchtet rot
  * Von Kraft4 bis Kraft5: blinkt rot / blau
  * Über Kraft5 wird ein Alarm geschaltet

* Arduino IDE 2.2.1 https://www.arduino.cc/en/software
* Library used: HX711_ADC 1.2.12 by Olav Kallhovd (https://github.com/olkal/HX711_ADC)
