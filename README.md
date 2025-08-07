<p style="color: #87CEEB"> DISCLAIMER: THE DESIGN FILES IN THIS REPOSITORY HAVE BEEN CLEARED OF ANY TRADEMARK-PROTECTED LOGOS.</p>

<p align="center"><a href="https://www.youtube.com/watch?v=Ej3K-IHdLqs"><img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/youtube.png" width="100%"></a></p>

<h1>Less, but better</h1>
Legendary German industrial designer Dieter Rams is best known for his tenure at Braun, where he developed a design philosophy that emphasized simplicity, functionality, and clarity. His work was guided by the principle "Less, but better" (“Weniger, aber besser”), aiming to strip objects down to their essential purpose without unnecessary embellishment. Rams believed that good design is as little design as possible, and this ethos permeated his creations at Braun. His iconic "Ten Principles of Good Design" laid a foundation not only for Braun's product identity but also for the evolution of modern design aesthetics in consumer electronics globally.<br>
<br><br>
<p align="center"><img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/sk3.jpeg" width="70%"><br>
Braun SK2 radio</p><br>
Among Rams' most influential designs are the SK2 radio, the TP1, and the T3 Pocket Radio. The SK2, introduced in 1955, featured a minimal white casing with a clear, intuitive layout, embodying the clarity and honesty Rams championed. The TP1 combined a record player and radio into one compact, portable device—a groundbreaking integration at the time—highlighting Rams’ commitment to multifunctional utility and user-friendly design. The T3 Pocket Radio, released in 1958, took these values even further: it was compact, elegant, and remarkably prescient, widely regarded as a spiritual predecessor to the Apple iPod due to its minimalist front face and clear functional layout.<br>
<br>
<p align="center"><img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/tp1.jpg" width="70%"><br>
Braun TP1 portable record player</p><br>
Rams' influence on the design industry, particularly on companies like Apple, is unmistakable. Jony Ive, Apple’s former chief design officer, openly credited Rams as a major inspiration for the look and feel of Apple products. The restrained elegance and user-centric functionality of Braun devices under Rams’ direction clearly echo in Apple’s hardware—from the clean lines of the iPhone to the intuitive control layout of the iPod. Through designs like the SK2, TP1, and T3, Rams didn't just shape Braun’s identity; he helped define what good design means in the modern world, leaving a legacy that continues to shape the aesthetics and usability of consumer technology today.<br>
<br>
<p align="center"><img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/rams2.jpg" width="70%"><br>
 Braun T3 portable radio</p>
<br>
After a glorious period of minimalist designwork, Dieter Rams' left Braun in the mid-1990s and the company gradually shifted away from its legacy of minimalist consumer electronics toward high quality personal care products, such as electric shavers, hair dryers, and epilators. <br>
<br>
Dieter Rams' design legacy lives on in many contemporary products, such as the portable recorders and synthesizers by Teenage Engineering.<br><br>
<p align="center"><img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/teenage.jpg" width="70%"><br>
Teenage Engineering TP-7</p>

<h1>Something new</h1>
<p>Inspired by Braun’s iconic designs, particularly the T3, I started from the familiar speaker hole pattern and gradualy added elements that worked visually and physically (would fit the electronics). The first attempt was a direct copy of the T3 with an added square display (much like the iPod). Albeit the design worked reasonably well, it also felt overused and a lost opportunity in creating something new.<br>
 <p align="center"><img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/t3_clone2.png" width="70%"><br>
First concept based on T3 shape and TP1 hole pattern</p>
I kept experimenting with different shapes until a cylindrical form emerged, driven by the original hole pattern and the dimensions of the speaker. The top section simply extends the pattern into the 3rd dimensions, effectively serving as a 360 degree speaker grill. To complete the design, I also added the distinctive top/bottom gap reminiscent of the classic T3 that ties everything together.</p>
<p>Initially, I opted for a rectangular ultra–low-power monochrome display to maximize battery life. However, I soon realized that a) it looks rather hacked together b) such displays lack backlighting, making them practically unusable in an alarm clock / low-light kind of situation. So might as well use nice a round IPS LCD and turn it off during sleep. Not only does it complement the circular design of the case far more naturally, but it also offers a significantly better user experience thanks to its higher resolution and contrast.</p>
<p align="center"><img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/PXL_20250604_145024065.jpg" width="70%"><br>
Rectangular ultra-low-power display</p>
<p>So how do we operate this thing? Although the GC9A01 also comes in a "touch" variant, I decided that a physical interface would work better for this build. I started off with two encoder knobs (one for station, one for volume), but reduced it later to a single knob angled at 45-degrees. This choice aligned much better with rest of the design and effectively simplified the user interface.<br>
 <br>Battery life is reasonably good too. The radio has a current consumption of about 800uA during sleep and 200mA during radio operation which equates to 220 days of standy or 22h of radio play using two 2200mAh 18650 cells. If you use it as a alarm clock, that's about 1.5 months of battery life, given that you need 30min to get out of bed each morning.</p>
<br>
<p align="center"><img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/Radio_Clock v40_top_white.png" width="100%"><br>
Final design</p>

<h1>Build Overview</h1>
 <p style="text-align:center;">The IR7 is built around a few key components:</p>
 
<table align="center">
<tr><th>Part list</th></tr>
<tr><td>Lolin32 Lite / ESP32 microcontroller with onboard battery charging- 3.80€</td></tr>
<tr><td>GC9A01 round IPS LCD display - 3.60€</td></tr>
<tr><td>DS3231 real-time clock - 1.10€</td></tr>
<tr><td>One or two 18650 batteries - 2.00€</td></tr>
<tr><td>VS1053 MP3 decoder board - 5.20€</td></tr>
<tr><td>PAM8403 audio amplifier - 0.40€</td></tr>
<tr><td>600:600 Ohm isolation transformer for cleaner audio - 0.50€</td></tr>
<tr><td>2–2.5 inch wideband speaker (e.g., Visaton FR7) - 7.00€</td></tr>
<tr><td>Rotary encoder - 0.20€</td></tr>
<tr><td>TS3134 P-Channel MOSFET - 0.10€</td></tr>
<tr><td>Miscellaneous resistors and plastic screws - 0.10€</td></tr>
<tr><td>3D printed parts (optionally from JLCPCB) - 2.00€ (15.00€)</td></tr>
<tr><td><b>TOTAL - 27.00€ (40.00€)</b></td></tr>

 
</table>
<br>
First, let’s get started by preparing the 3D-printed components. You can either print the four plastic parts yourself — the lower shell, upper shell, resonator, and knob — or order them from a 3D printing service of your choice.<br>
<br>
While my FDM printer handled the case fairly well, it struggled with producing perfectly round vertical holes — a common limitation of that printing method. To get the best quality, I chose to have the parts SLA-printed by JLCPCB, and the results were excellent.<br>
<br>
<p align="center">
  <img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/PXL_20250501_132053099.jpg" width="70%"><br>
<img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/PXL_20250501_132123846.jpg" width="70%"><br>
Plastic parts</p>
<br>
Begin by gluing the ESP32 board and rotary encoder into their designated locations. Avoid using thermoplastic (e.g. hot) glue, as the microcontroller can heat up significantly during charging and will delaminate.<br><br>
<p align="center"><img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/PXL_20250501_114952391.jpg" width="70%"><br>
Step 1 - ESP32 & encoder</p>
Next, insert the speaker into the resonator tube. I’ve included cutout sizes for both 2-inch and 2.5-inch speakers. Use plastic screws to secure the speaker. If the fit is tight, you may need to carefully trim the edges. Finally, solder the speaker wires to the terminals.<br>
<br>
<p align="center"><img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/PXL_20250501_132210479.jpg" width="70%"><br>
  <img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/PXL_20250501_132229687.jpg" width="70%"><br>
<img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/PXL_20250501_132519935.jpg" width="70%"><br>
  <img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/PXL_20250501_132726693.jpg" width="70%"><br>
<img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/PXL_20250501_132833551.jpg" width="70%"><br>
Step 2 - Speaker</p>
Since the VS1053/VS1003 audio output is only suitable to drive headphones, an audio amplifier is needed. However, directly connecting the amplifier to the VS1053 introduces noticeable noise. The solution: use an audio isolation transformer. I soldered mine directly onto the amplifier board to keep the design compact and reduce cable length, which minimizes noise pickup. Connect the amplifier and transformer to the VS1053 using two 1kΩ resistors. Note that you want to use a geeen VS1053 board commonly available on Ebay and Aliexpress. The blue board shown in the picture actually uses an VS1003 chip which is somewhat compatible but supports less codecs and has a few other quirks.<br>
<br>
<p align="center"><img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/PXL_20250501_133629600.jpg" width="70%"><br>
<img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/PXL_20250501_134841618.jpg" width="70%"><br>
Step 3 - Audio</p>
Solder thin signal wires to all exposed pins on the LCD display board. Insert the display into the upper shell and secure it with glue (hot glue works fine here). Feed the signal wires through the small hole in the resonator, then slide the speaker-resonator assembly into the upper shell and secure it in place with some glue. Finally, connect the signal wires and VCC to the ESP32.<br>
<br>
<p align="center"><img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/PXL_20250501_141458646.jpg" width="70%"><br>
<img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/PXL_20250501_173942662.jpg" width="70%"><br>
Step 4 - Display</p>
Even though most components (except the amp) support low-power modes, I found that the sleep current still hovers around 1.5 mA. To truly optimize battery life, a TS3134 MOSFET is used to cut power to all peripherals during sleep. Connect it between battery ground and the ground lines for the display, VS1053, and amplifier. <br>
<br>
<p align="center"><img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/PXL_20250501_182227144.jpg" width="70%"><br>
Step 5 - Low power</p>
Solder two power wires to the battery terminals. To monitor battery voltage add a voltage divider made of two 10kΩ resistors to bring the voltage down to a safe 0–2.2V range for the ESP32.
<br>
<br> 
<p align="center"><img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/PXL_20250501_183034954.jpg" width="70%"><br>
 Step 6 - Battery</p>
 <br>
The ESP32's built-in RTC is notoriously unreliable. To ensure accurate timekeeping and enable timed wake-ups, we use a DS3231 Real-Time Clock module. It keeps time while the system is asleep and can trigger an interrupt to wake the ESP32 at a precise time. On some modules, the interrupt pin isn’t exposed — in that case, you’ll need to solder a wire directly to the chip (see picture).
<p align="center">
  <p align="center"> <img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/PXL_20250501_185822012.jpg" width="70%"><br>
 Step 7 - Real Time Clock</p>
The final component to wire up is the rotary encoder. After connecting all GPIO and power wires, secure any loose parts — including the RTC, battery, MOSFET, etc. — with glue to keep everything neat and stable inside the case.<br>
<br>
<table align="center">
    <tr><th>GPIO map</th></tr>
    <tr><td>SPI_MISO_PIN GPIO_NUM_12</td></tr>
    <tr><td>SPI_MOSI_PIN GPIO_NUM_25</td></tr>
    <tr><td>SPI_CLK_PIN GPIO_NUM_33</td></tr>
    <tr><td>VS1053_DREQ GPIO_NUM_23</td></tr>
    <tr><td>VS1053_CS GPIO_NUM_17</td></tr>
    <tr><td>VS1053_DCS GPIO_NUM_16</td></tr>
    <tr><td>VS1053_RST GPIO_NUM_18</td></tr>
    <tr><td>LCD_DC GPIO_NUM_26</td></tr>
    <tr><td>LCD_CS GPIO_NUM_27</td></tr>
    <tr><td>LCD_BL GPIO_NUM_4</td></tr>
    <tr><td>MOSFET_EN GPIO_NUM_32</td></tr>
    <tr><td>ENC1_BUT GPIO_NUM_13</td></tr>
    <tr><td>ENC1_A GPIO_NUM_15</td></tr>
    <tr><td>ENC1_B GPIO_NUM_2</td></tr>
    <tr><td>DS3231_CLK GPIO_NUM_0</td></tr>
    <tr><td>DS3231_SDA GPIO_NUM_5</td></tr>
    <tr><td>VCC_PIN 34</td></tr>
 </table>
<br>
<p align="center"><img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/PXL_20250501_190320371.jpg" width="70%"><br>
 Step 8 - Encoder wiring and finishing touches</p>
<br>
<p align="center"><img src="https://github.com/2dom/IR7_streaming_radio/blob/main/Images/PXL_20250608_140712961-EDIT (1).jpg" width="100%"><br></p><br> 



<h1>Uploading the Software to the ESP32</h1>

Once your hardware is assembled, it’s time to upload the firmware and web assets to the ESP32. You can use either the **Arduino IDE** or **PlatformIO**. Both work—choose whichever you're more comfortable with.

---

### 🧰 Arduino IDE

1. **Install Required Boards and Libraries**
   - Install the **ESP32 by Espressif Systems** board via the **Boards Manager**.
   - Use the **Library Manager** to install:
     - `TFT_eSPI`
     - `VS1053_ext` (from this repo or local import)
     - `AiEsp32RotaryEncoder`
     - `RTClib`
     - `ESP32Time`

2. **Select Your Board**
   - Go to `Tools > Board` and choose your ESP32 model (e.g., **ESP32 Dev Module**).

3. **Configure TFT_eSPI**
   - The IR7 uses a **GC9A01 240x240 round display**.  
     Replace the contents of `TFT_eSPI/User_Setup.h` with:
     ```cpp
     #define GC9A01_DRIVER

     #define TFT_WIDTH  240
     #define TFT_HEIGHT 240

     #define TFT_CS   27  // Chip select
     #define TFT_DC   26  // Data/command
     #define TFT_RST  -1  // Reset pin (set to -1 if not used)

     #define TFT_SCLK 33
     #define TFT_MOSI 25

     #define LOAD_GLCD
     #define LOAD_FONT2
     #define LOAD_FONT4
     #define LOAD_FONT6
     #define LOAD_FONT7
     #define LOAD_FONT8

     #define SMOOTH_FONT

     #define SPI_FREQUENCY  40000000
     ```
   - Alternatively, use the `User_Setup.h` file provided in the `lib/TFT_eSPI/` folder of this repo.

4. **Upload the Code**
   - Open `main.ino`, select the correct port, and click **Upload**.

5. **Upload SPIFFS Data (Stations & Wi-Fi)**
   - Install the [ESP32 Sketch Data Upload tool](https://github.com/me-no-dev/arduino-esp32fs-plugin).
   - Place `stations.txt` and `wifi.txt` inside a folder named `data/`.
   - Then go to **Tools > ESP32 Sketch Data Upload** to flash the SPIFFS filesystem.

---

### 🧰 PlatformIO (VS Code)

1. **Open the Project**
   - Use **VS Code** with the **PlatformIO** extension and open the IR7 project folder.

2. **Dependencies**
   - These libraries are automatically handled via `platformio.ini`, but ensure these are present:
     - `TFT_eSPI`
     - `VS1053_ext`
     - `AiEsp32RotaryEncoder`
     - `RTClib`
     - `ESP32Time`

3. **Configure TFT_eSPI**
   - Place the same `User_Setup.h` as shown above into `lib/TFT_eSPI/` or override it using `platformio.ini`:
     ```ini
     build_flags =
       -DUSER_SETUP_LOADED=1
       -include src/User_Setup.h
     ```
   - Then place the `User_Setup.h` file into the `src/` directory.

4. **Upload Firmware**
   ```bash
   pio run --target upload

<h1>Device Setup: Wi-Fi & Station Configuration</h1>

### Entering Setup Mode (Configuration/AP Mode)

To configure Wi-Fi and radio stations, you need to enter **setup mode** (also called AP mode):

1. **Hold the Encoder Button**  
   - While powering up (or rebooting) the device, **press and hold the rotary encoder button**.  
   - Keep holding until the screen displays “Braun IR7 Config” and “Connect to Wifi:” (the device will now act as a Wi-Fi Access Point).

2. **Connect to the Device’s Wi-Fi**  
   - On your phone, tablet, or computer, search for the Wi-Fi network named:  
     **Braun IR7 Config**  
   - Connect to this network (no password required).

3. **Access the Web Interface**  
   - Open a web browser and go to:  
     **http://192.168.4.1**  
     *(You’ll be automatically redirected if you try any URL.)*

### Entering Wi-Fi and Password Information

- On the configuration page, you’ll see a field labeled:
  **Wi-Fi Credentials (SSID;password per line)**
- Enter your Wi-Fi credentials in this format:  
your_wifi_ssid;your_wifi_password another_ssid;another_password

Code
*(Each line corresponds to one network. You can add multiple networks if desired.)*

### Adding, Removing, or Editing Radio Stations

- Below the Wi-Fi section, there’s a field labeled:
**Radio Stations (name@url per line)**
- Enter your favorite streaming radio stations in this format:
Radio Paradise@http://stream.radioparadise.com/mp3-128 BBC World@http://bbcmedia.ic.llnwd.net/stream/bbcmedia_radio1_mf_p

Code
*(Each line should have a station name, then an `@`, then the stream URL.)*

- **To add a station:**  
Just add a new line.
- **To edit/remove a station:**  
Modify or delete the relevant line.

### Save & Reboot

- Once you’ve entered your Wi-Fi and station details, **click “Save & Reboot”**.
- The device will store your settings, reboot, and attempt to connect to the Wi-Fi network(s) you’ve provided.
- After setup, the device will start streaming from your list of stations.

