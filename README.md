
# water_ESP_part
Esp part for Toon Water

ESPEasy flashen
De beschrijving hieronder is voor een computer die draait onder Windows.
Onder Linux of MacOS kan gebruik worden gemaakt van ESPTool.py.

Het flashen
De software waarmee de D1 wordt geflasht wordt gedownload vanaf de site van ESP-Easy.

Heeft al eerder software op de ESP gestaan, flash dan eerst de balnco 4MB om ook het interne geheugen helemaal leeg te maken.

Download esp-easy en pak het zip-bestand uit in een directory naar keuze.
https://github.com/letscontrolit/ESPEasy/releases

Koppel de D1 met een micro-USB kabel aan je computer.
Neem hiervoor een kwalitatief goede kabel, om problemen tijdens het flashen te voorkomen.
In Windows 8 en hoger wordt de D1 normaal gesproken herkend en worden de drivers automatisch geladen. Is dat niet het geval, dan kan je hier de juiste USB-drivers dowloaden en dan installeren. Daarna zou het herkennen wel moeten lukken.

De D1 wordt aan de pc gekoppeld als com-poort. Kijk in Device Manager (Apparatenbeheer) van Windows welke COM poort in gebruik is genomen door de D1.
De aanduiding CH340 zal bij de gezochte poort staan.

Zoek in de directory waarin de ESPEasy software is uitgepakt naar FlashESP8266.exe en start dit programma.
Kies bij COM-Port de opgezochte poort (zie boven) en kies bij Firmware voor de .bin file uit deze Git.
Start de flashtool met de knop ‘Flash’. Als alles goed is gegaan verschijnt er een commandovenster en wordt de ESP8266 geflasht.
Zodra het flashen is geëindigd, dan wordt dit door de software gemeld.
Nu verbreek je de verbinding met de computer. Het beste kan je de D1 nu met de USB-kabel aan een eigen stroombron koppelen en niet meer aan de pc.

Start de Wemos D1 op en zoek naar wifi punt AutoConnectAP. Maak verbindeing en vul de gegevens in.
