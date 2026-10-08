# PC Monitor + Media Player — ESP32-2432S028R ("CYD")

PC-r live stats (CPU, RAM, GPU, disk, network, temperature, tasks, clock) ekta 2.8" touch screen-e dekhay, ar SD card theke
image / GIF / video full screen-e play kore. Shob kichu browser theke control kora jay (upload, play, delete, settings).

> **Status (sotti kotha):** ei code ESP32-S3 + ILI9488 version theke CYD-r jonno port kora. Ami (Claude) likhechi, kintu
> real board-e test kora hoyni. Prothom build-e choto-khato adjust lagte pare (colour, touch direction, SD speed).
> Troubleshooting section-e shob known fix deya ache.

---

## 1. Ki ki pabe

| Feature | Details |
|---|---|
| 10 ta page | Overview, Temperature, Tasks, Disk, Activity, Network, GPU, Clock, Dev, Media |
| Web control panel | `http://<board-ip>/` ba `http://pcmonitor.local/` |
| Touch | Tap / swipe / drag / long-press diye page, brightness, auto-rotate |
| SD media | Image, GIF, MJPEG video — alada folder-e save, full screen play, delete |
| WiFi setup | Prothom boot-e hotspot `PC-Monitor-Setup`, ba code-e router-er nam/password likhe dao |
| Playlist | SD-r shob file ekshathe cholano (web theke "Play all") |

---

## 2. Hardware (jinish ja lagbe)

| # | Component | Kaj | Notes |
|---|---|---|---|
| 1 | **ESP32-2432S028R (CYD)** board | Main board (ESP32-WROOM-32, 4 MB flash, 520 KB SRAM, **PSRAM nei**) | ST7789 + SD card version |
| 2 | 2.8" TFT, **ST7789**, 240×320 | Display (board-e built-in) | Landscape 320×240 use hoy |
| 3 | **XPT2046** resistive touch | Touch (built-in) | Alada pin-e, library lage na |
| 4 | **microSD card** (FAT32, 2–32 GB, Class 10) | Media rakhar jonno | Board-er back-e slot ache |
| 5 | USB cable (data wala, bhalo quality) | Power + flash | Kharap cable = brown-out / WiFi problem |
| 6 | PC (Windows/Linux/macOS) | Stats dey (`pc_agent.py`) ar build kore | Board ar PC **ek-i WiFi network-e** thakte hobe |
| 7 | *(optional)* Rotary encoder (KY-040) | Page change / click | Pin: 22, 27, 35 (niche dekho) |
| 8 | *(optional)* 10 kΩ resistor | Encoder button pull-up | GPIO35-e internal pull-up nei |

Board-e thaka kintu **ei project-e use hoy na:** LDR (GPIO34), RGB LED (GPIO4/16/17), speaker (GPIO26), Bluetooth.

### Pin map (CYD-te fixed, bodlano jabe na)

| Function | GPIO |
|---|---|
| TFT SCK / MOSI / DC / CS | 14 / 13 / 2 / 15 |
| TFT RST | EN-ar sathe jora (code-e `-1`) |
| TFT backlight | 21 (kono revision-e **27**) |
| Touch CLK / MOSI / MISO / CS / IRQ | 25 / 32 / 39 / 33 / 36 |
| SD SCK / MISO / MOSI / CS | 18 / 19 / 23 / 5 |
| BOOT button (page button) | 0 |
| Encoder A / B / SW *(optional)* | 22 / 27 / 35 |

TFT **HSPI**-te, SD **VSPI**-te, touch bit-bang — tai tin-ta ek-sathe jhamela chhara chole.

---

## 3. Software ar libraries

### Tools (PC-te install korba)
| Tool | Kothay | Keno |
|---|---|---|
| **VS Code** | https://code.visualstudio.com | Editor |
| **PlatformIO IDE** extension | VS Code → Extensions → "PlatformIO IDE" | Build + upload |
| **USB driver** (CH340 ba CP210x) | Board-er chip onujayi | PC-te COM port dekhar jonno |
| **ffmpeg** | `winget install Gyan.FFmpeg` | Video/photo convert (optional, web-e convert hole lage na) |
| **Python 3** + `pc_agent.py` | https://python.org | PC stats pathay |

### Libraries (PlatformIO auto-download kore — `platformio.ini`)
| Library | Version | Kaj |
|---|---|---|
| `lovyan03/LovyanGFX` | ^1.1.16 | Display driver (ST7789), fonts, DMA |
| `bblanchon/ArduinoJson` | ^7.0.4 | PC `/stats` JSON + web API |
| `bodmer/TJpg_Decoder` | ^1.1.0 | JPEG / MJPEG video decode |
| `bitbank2/AnimatedGIF` | ^2.1.1 | GIF decode (SD theke stream) |
| Arduino-ESP32 built-in | — | WiFi, WebServer, DNSServer, ESPmDNS, SD, SPI, Preferences, HTTPClient |

Platform: `espressif32`, board `esp32dev`, framework `arduino`, partition `huge_app.csv` (4 MB flash, ~3 MB app).

---

## 4. Project structure

```
cyd/
├─ platformio.ini            ← build config + shob env (variant)
├─ README.md                 ← ei file
├─ src/
│  ├─ main.cpp               ← pura firmware (pages, WiFi, web API, SD player)
│  ├─ display.h              ← ST7789 driver + 480x320→320x240 scaling layer
│  ├─ touch.h                ← XPT2046 bit-bang touch driver
│  └─ webui.h                ← browser control panel (HTML/JS)
└─ tools/
   └─ convert_for_cyd.bat    ← photo/video/gif → CYD format (ffmpeg)
```
> `src/`-er **4-ta file-i** thaka lagbe. `webui.h` na thakle build fail kore.

---

## 5. Step-by-step setup

### Step 1 — PC prepare
1. VS Code install koro → Extensions theke **PlatformIO IDE** install koro → VS Code restart.
2. Board USB diye PC-te lagao. Windows-e *Device Manager → Ports (COM & LPT)* e port dekha jabe. Na dekhale USB driver (CH340 / CP210x) install koro, ar data-capable cable use koro.

### Step 2 — Project open
1. `cyd.zip` extract koro (jemon `D:\cyd`).
2. VS Code → **File → Open Folder** → `cyd` folder (jeikhane `platformio.ini` ache).
3. PlatformIO kichukkhon library download korbe — shesh howa porjonto wait koro.

### Step 3 — Router-er WiFi code-e dao *(shobcheye shoja, hotspot lagbe na)*
`src/main.cpp`-te line ~42:
```cpp
#define DEF_SSID  "TomarWiFiNam"
#define DEF_PASS  "TomarPassword"
```
Khali rakhle prothom boot-e hotspot cholbe (Step 6 dekho). PC-r IP-o ekhane (`DEF_PC_IP`) dite paro, ba pore web theke.

### Step 4 — Build
Niche status bar-e **✓ (Build)** click koro, ba terminal-e:
```
pio run -e cyd
```
Default env `cyd` (ST7789). Kon env kokhon — section 7 dekho.

### Step 5 — Upload
1. Status bar-e **→ (Upload)** click koro, ba: `pio run -e cyd -t upload`
2. Upload shuru na hole: **BOOT button chepe dhore** thako, EN/RESET ekbar chap, tarpor BOOT chhere dao.
3. Serial monitor: `pio device monitor` (115200 baud). Eikhane `boot:`, `softAP`, `heap=...` lines dekhbe.

### Step 6 — WiFi connect
**A) Code-e SSID dile:** board nije router-e connect hoy, screen-e IP dekhay.

**B) Hotspot diye:**
1. Phone/laptop-e WiFi `PC-Monitor-Setup` e connect koro (password nei).
2. Browser-e `http://192.168.4.1` kholo.
3. WiFi list theke tomar router select koro, password dao, Save.
4. Board reboot hoye router-e connect hobe; screen-e ba serial-e notun IP dekhay.

Pore browser-e `http://<IP>` ba `http://pcmonitor.local` dilei panel khulbe.
> Setup screen-e "AP clients: N, free RAM: K" dekhay. Free RAM 40K-r niche gele memory somossha.

### Step 7 — PC agent
Board PC-r `http://<PC-IP>:8080/stats` URL theke JSON pore. Tomar `pc_agent.py` PC-te chalao:
```
pip install psutil
python pc_agent.py
```
Tarpor web panel-e **PC IP** (ar port 8080) set koro. Firewall-e port 8080 allow koro.

`/stats` JSON-er field (jeta board expect kore; je field nei seta skip hoy):
`host, name, cpu, freq, temp, ram, ram_used, ram_total, disk, up, down, uptime, nproc, gpu, gpu_temp, gpu_mem_used, gpu_mem_total, dr, dw, swap, ping, tx_gb, rx_gb, ip, bat, plug, win, wapp, time, date, cores[], procs[{n,c,m}], drives[{n,p,u,t}], rprocs[{n,m}], dprocs[{n,r}]`

### Step 8 — SD card
1. Card **FAT32** format koro (Windows: right-click → Format → FAT32; 32 GB-er boro hole `guiformat` use koro).
2. Card board-e lagao, board reboot dao. Folder auto toiri hoy: `/videos`, `/gifs`, `/images`.
3. Web panel → **Media** tab → upload.

---

## 6. Controls

### Touch
| Gesture | Kaj |
|---|---|
| Screen-er **bam 35%** tap | Ager page |
| Screen-er **dan 35%** tap | Porer page |
| **Majhkhane** tap | Overview / alert snooze |
| Bam-dan **swipe** | Porer / ager page |
| Upor-niche **drag** | Brightness |
| **Bam edge long-press** (0.6 sec) | **Ager media file** (video / GIF / photo) — jekono page theke |
| **Dan edge long-press** (0.6 sec) | **Porer media file** — jekono page theke |
| **Majhkhane long-press** (0.8 sec) | Auto-rotate on/off |
| **Majhkhane 2.5 sec** | Night mode |

Media file-gulo `/videos`, `/gifs`, `/images` theke ek-sathe alphabetical order-e ghure (last-er por abar prothom-e). Play hole file-er nam screen-e kichukkhon dekhay.

Touch kaj korche kina dekhte: Serial Monitor (115200) e screen chhunle `touch raw=... -> x=.. y=..` lekha ashbe.
Touch-er jaygay vul hole `TOUCH_X_MIN/MAX`, `TOUCH_Y_MIN/MAX` (touch.h) ba `TOUCH_INV_X/Y/SWAP_XY` flag bodlao. IRQ pin diye detect korte chaile `-DTOUCH_USE_IRQ` (default: pressure polling, beshi reliable).

### BOOT button (GPIO0)
Short press = porer page · 1–4 sec = auto-rotate on/off · 4 sec+ = WiFi setup hotspot.

### Encoder *(optional)*
Ghurano = page change · click = auto-rotate · long press = night mode.
Wiring: A→GPIO22, B→GPIO27, SW→GPIO35 (+10 kΩ pull-up 3.3V-e), +/GND → 3V3/GND (CN1/P3 connector).

---

## 7. Build environments (`platformio.ini`)

| Env | Kokhon use korba |
|---|---|
| `cyd` | **Default.** ST7789, 40 MHz |
| `cyd_ili9341` | Purono 1-USB CYD (ILI9341 panel) |
| `cyd_20mhz` | Screen-e noise / sparkle |
| `cyd_80mhz` | Video aro fast (noisy hole `cyd`-e fira) |
| `cyd_bl27` | Screen kalo kintu board alive (backlight GPIO27) |
| `color_bgr` | Lal ar neel ulta |
| `color_invert` | Rong negative dekhale |
| `cyd_bitmap_fonts` | Smooth font-e overlap / blocky text |
| `cyd_flip` | Ulta / mirrored picture |
| `cyd_touch_inv_x` | Touch swipe ulta |
| `cyd_nobrownout` | WiFi chalu hole board reset (power test) |
| `cyd_sprite` | RAM sprite diye smooth redraw (RAM beshi lage, WiFi-te somossha hote pare) |

Build flag-er list:

| Flag | Default | Mane |
|---|---|---|
| `DISP_SPI_HZ` | 40000000 | TFT SPI speed |
| `DISP_INVERT` | 0 | Colour inversion |
| `DISP_RGB_ORDER` | 0 | 1 = BGR |
| `PIN_TFT_BL` | 21 | Backlight pin |
| `SD_SPI_HZ` | 40000000 | SD speed (mount na hole 25000000 / 20000000) |
| `DEF_SSID` / `DEF_PASS` | "" | Router WiFi |
| `ENC_A/ENC_B/ENC_SW` | 22/27/35 | Encoder pin |
| `CYD_USE_SPRITE` | off | RAM sprite mode |
| `CYD_BITMAP_FONTS` | off | Classic bitmap font |
| `CYD_NO_BROWNOUT` | off | Brown-out detector bondho |
| `TOUCH_INV_X/Y`, `TOUCH_SWAP_XY`, `TOUCH_USE_IRQ` | off | Touch tuning |

---

## 8. Media (image / GIF / video)

### SD card folder
| Folder | File |
|---|---|
| `/videos` | `.avi` (MJPEG), `.mjv` |
| `/gifs` | `.gif` |
| `/images` | `.jpg` / `.jpeg` (baseline), `.rgb` |

### Option A — Web theke upload (shobcheye shoja)
Browser-e Media tab → file dao (MP4, MKV, MOV, WebM, GIF, PNG, WebP, JPG...). **Browser nije convert kore**
320×240 e, SD-te save kore, play kore. Photo-te **Floyd–Steinberg dithering** hoy (colour banding nei), video-te high JPEG quality. Video preset:

| Preset | Size | FPS |
|---|---|---|
| **BEST quality** | 320×240 (JPEG q 0.90) | 10 |
| High | 320×240 (q 0.82) | 12 |
| Smooth | 320×214 | 12 |
| Balanced | 280×210 | 15 |
| Fast | 240×160 | 20 (shobcheye smooth) |
| Wide | 320×180 | 15 |
| Small | 160×120 | 25 |

Web theke list, **play**, **play all**, **delete** kora jay.

### Option B — PC-te convert kore SD-te copy
1. `tools/convert_for_cyd.bat`-er upor photo/video/GIF **drag & drop** koro.
2. Eta file-er pashe `SD\videos\*.avi` ar `SD\images\*.jpg` banabe.
3. `SD` folder-er bhitor-er shob microSD-r root-e copy koro.

Manual ffmpeg:
```
# Video / GIF → AVI
ffmpeg -i input.mp4 -vf "scale=320:240:force_original_aspect_ratio=increase,crop=320:240,fps=15" -c:v mjpeg -pix_fmt yuvj420p -q:v 6 -an out.avi
# Photo → JPG
ffmpeg -i photo.png -vf "scale=320:240:force_original_aspect_ratio=increase,crop=320:240" -frames:v 1 -c:v mjpeg -pix_fmt yuvj420p -q:v 3 out.jpg
```

### Limit-gulo (honest)
- **Direct MP4 / H.264 SD-te rekhe play hobe na** — ESP32 H.264 decode korte pare na. Web-e dao ba `.bat` diye `.avi` banao.
- Max resolution **320×240**; boro file reject hoy.
- Quality-r kotha: panel RGB565 (65k colour) — eta hardware limit, tai 24-bit-er moto hobe na; dithering ar BEST preset-e jotota hoy toto bhalo. Video fps: 320×240-e ~10–12 fps, 240×160-e ~20 fps (ESP32 JPEG decode-er limit). AVI-te dhimi frame skip hoy, speed thik thake.
- JPG RAM-e fit hole-i khole (boro phone photo-r jonno `.bat` / web use koro).
- Live USB/WiFi stream ar "upload only to RAM" **cholbe na** (PSRAM nei).
- GIF choto hole screen fill kore scale-up hoy, boro hole crop hoy.

---

## 9. Troubleshooting

| Somossha | Karon / Fix |
|---|---|
| Build: `webui.h: No such file` | `src/`-e 4-ta file-i thakte hobe (`main.cpp display.h touch.h webui.h`) |
| Build: font / LovyanGFX error | `cyd_bitmap_fonts` env diye build koro, ar full error pathao |
| Screen kalo | `cyd_bl27` env; USB power check |
| Rong negative | `color_invert` env |
| Lal-neel ulta | `color_bgr` env |
| Picture-e noise | `cyd_20mhz` env |
| Picture ulta | `cyd_flip` env |
| Touch ulta / vul jaygay | `TOUCH_INV_X`, `TOUCH_INV_Y`, `TOUCH_SWAP_XY` |
| Hotspot-e connect hoy na / board reset | Bhalo USB cable/port; `cyd_nobrownout` env; Serial log dekho |
| IP-te web page khole na | Setup screen-e "free RAM" dekho (40K-r niche = problem); router-e connect kore `DEF_SSID` dao; `http://pcmonitor.local` try koro |
| PC stats ashe na | `pc_agent.py` cholche? PC IP/port thik? Firewall port 8080? Board-PC ek network-e? |
| "SD card not ready" | FAT32? Card firmly ache? `SD_SPI_HZ=20000000` dao |
| "Frame too big" | Fast / Small preset use koro |
| "clip too big" | Purono boro clip; notun kore 320×240 e convert koro |
| Video dhire / atke | Fast preset; `cyd_80mhz`; SD Class 10 card |
| Text overlap | `cyd_bitmap_fonts` env; kon page-e hocche screenshot dao |
| Upload shuru hoy na | BOOT chepe dhore RESET chap |

### Serial log (115200)
`boot: heap=...` → shuru-r RAM · `softAP "PC-Monitor-Setup" OK` → hotspot ready · `heap=... wifi=3` → router-e connected (6 = connect hoyni).

---

## 10. Web API (advanced)

`GET /api/state` · `POST /api/settings` · `GET /api/next` `/api/prev` `/api/page` · `GET /api/scan` · `POST /api/wifi` ·
`GET /api/sd/list` · `POST /api/sd/play` `/api/sd/playall` `/api/sd/delete` `/api/sd/mount` · `POST /api/reboot` · `POST /api/reset`
