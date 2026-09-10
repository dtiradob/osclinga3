# OSCLINGA 4.0 Firmware

ESP32-WROOM-32 firmware for the OSCLINGA custom PCB: OSC/UDP-controlled LED
strobes + DC motors, RTC-scheduled choreography, rotary-encoder input, Modbus
RTU (RS485) output, and a WiFi-AP-hosted web UI (SPIFFS) for editing/playing
CSV presets.

Cleaned-up, reorganized successor to `osclinga_3_3/` — same codebase and
fixes, single `src/` tree that targets either a generic dev board or a real
custom PCB (any BOARD_ID) purely via PlatformIO build environments, no
archived/duplicate copies to keep in sync.

## Folder layout

```
osclinga_4/
├── platformio.ini      PlatformIO project config: devboard / pcb_board1 / pcb_board2 envs
├── data/                SPIFFS content: index.html, coreo.csv, coreo_list.csv, P0-P7.csv
├── schematic.pdf         Custom PCB schematic
└── src/
    ├── globals.h         Shared includes / pin map / extern globals / cross-file prototypes / HAVE_* flags
    ├── osclinga_4.ino     setup()/loop(), encoders, LEDs, OSC handlers, Modbus, RTC/scheduling, web/CSV handlers
    ├── coreo_mirko.ino    coreoMirko(): time-based choreography sequence
    └── func_mirko.ino     presetN()/estorbo*()/apaga(): strobe/motor pattern functions
```

## Build environments (`platformio.ini`)

| Environment | Target | HAVE_RTC/ENCODERS/MODBUS | BOARD_ID | Upload speed |
|---|---|---|---|---|
| `devboard` | Generic ESP32-WROOM-32 devkit (CH340 USB-UART) + external SSD1306 OLED, no RTC/encoders/RS485 wired | 0 / 0 / 0 | 1 (fallback) | 921600 |
| `pcb_board1` | Real custom PCB unit #1 | 1 / 1 / 1 | 1 | 115200 |
| `pcb_board2` | Real custom PCB unit #2 | 1 / 1 / 1 | 2 | 115200 |

`BOARD_ID` drives both the AP SSID (`"osclinga" + BOARD_ID`) and a per-board
AP gateway IP (`192.168.<BOARD_ID>.1`), so multiple physical boards can run
simultaneously without colliding. Add a new `[env:pcb_boardN]` (copy an
existing one, bump `BOARD_ID`) for each additional physical PCB.

## ⚠️ Critical hardware finding: GPIO12 / MTDI strapping pin

`ENC1_CLK` (one of the rotary encoder clock lines) is wired to **GPIO12**,
which is the ESP32's **MTDI strapping pin** — sampled at reset/power-on to
select the flash voltage regime (1.8V vs 3.3V). Confirmed via the actual
schematic (annotated directly on the pin: boot fails if pulled high).

The rotary encoder module's pull-up holds CLK HIGH at rest, so at
power-on/reset the ESP32 can sample GPIO12 HIGH, select the wrong flash
voltage, and appear **permanently unable to boot** (a well-documented ESP32
gotcha, not a firmware bug — no amount of re-flashing fixes it, since it
happens before any firmware code runs).

`globals.h` pin defines are schematic-confirmed: `ENC1_CLK=12` / `ENC1_SW=13`,
and `UART2RX=16` / `UART2TX=17`.

A secondary, lower-severity strapping-pin risk: GPIO2 = `ENC2_SW` — this pin
must be floating/LOW at boot for **UART download mode** (flashing) to work
correctly; see "Flashing the real PCB" below.

Other known strapping-pin-adjacent issues already handled in firmware:
- `MODE` pin uses `INPUT_PULLUP` (GPIO15 is also a strapping pin — TDO/MTDO).
- `FAN_PWM` = GPIO3 (`U0RXD`, USB/serial RX) conflicts with `Serial` — fan
  output is currently unimplemented in firmware for this reason (flagged in
  `globals.h`).

## Build & flash

Requires [PlatformIO](https://platformio.org/) (`pip install platformio` if
not already on PATH).

```powershell
# Build (any environment)
pio run -e devboard
pio run -e pcb_board1
pio run -e pcb_board2

# Find the board's serial port
pio device list

# Flash firmware + filesystem
pio run -e <env> --target upload --upload-port COMx
pio run -e <env> --target uploadfs --upload-port COMx

# Serial monitor to confirm boot
pio device monitor --port COMx --baud 115200
```

A clean boot reaches these log lines with no crash/reboot loop:
```
Setting AP (Access Point)…AP IP address: 192.168.<BOARD_ID>.1
Starting UDP
Local port: 9000
...
FIN SETUP
```
A `E (nnn) ledc: ledc_get_duty(...): LEDC is not initialized` line during
setup is benign log noise from the first `ledcAttachPin()` call on
arduino-esp32 core 2.x — not an actual error.

## Flashing the real custom PCB (`pcb_board1` / `pcb_board2`)

**GPIO2 / `ENC2_SW` upload gotcha:** the real PCB's auto-reset circuit (a
standard 2-transistor DTR→EN / RTS→GPIO0 design, no manual jumpering needed)
correctly pulls GPIO0 low for UART download mode — but GPIO2 (`ENC2_SW`) is
a secondary ESP32 strapping pin that must also be floating/LOW at boot for
download mode to register, and the rotary-encoder module's pull-up holds it
HIGH at rest. Symptom: `esptool` connects but reports
`Wrong boot mode detected (0xa)` and the upload fails.

**Fix — no soldering/jumpers needed:** physically hold the `ENC2_SW` rotary
encoder pushbutton down while `pio run --target upload` (or `uploadfs`) is
connecting (through the `Connecting.......` line). You can release it once
write progress starts — the bootloader stays resident. If it still reports
"Wrong boot mode" on a given attempt, just retry the same command holding
the button again. This issue is intermittent — some flashes connect cleanly
without needing the button hold at all.

## OLED (SSD1306) blank-screen troubleshooting

If the display stays completely blank/off even though the serial log shows
`display.begin()` succeeding (see the diagnostic block in
`osclinga_4.ino`'s display setup, which logs which I2C address — `0x3C` or
`0x3D` — it initialized at), a successful `display.begin()` only proves the
module ACKed on I2C — it does **not** prove the panel itself is alive. A
physically dead/damaged SSD1306 module can still ACK the bus while showing
nothing. This was root-caused on a real unit by testing (and ruling out)
every software angle — wiring (SDA=21/SCL=22, confirmed against schematic),
I2C address (0x3C/0x3D fallback already in code), and VCC/charge-pump mode
(`SSD1306_SWITCHCAPVCC` vs `SSD1306_EXTERNALVCC` — both ACKed fine, screen
stayed dark either way) — before finally swapping in a same-model
replacement module, which fixed it immediately with zero code changes.
Firmware stays on the standard `SSD1306_SWITCHCAPVCC` mode.

Also note: some OLED module silkscreens print the **8-bit** I2C address
(including the R/W bit) instead of the 7-bit address Arduino/`Wire` expects
— e.g. a "0x7B" label is `(0x3D << 1) | 1`, i.e. 7-bit address `0x3D`. Right-shift
by 1 to convert. The firmware's existing 0x3C→0x3D fallback already covers
both common variants.

## Known hardware-level issues (schematic-level, not fixable in firmware alone)

- `FAN_PWM` = GPIO3, which is `U0RXD` (USB/serial RX) — conflicts with Serial.
  Fan output is currently unimplemented in firmware, flagged with a comment
  in `globals.h`.
- Device runs as an open WiFi AP with an unauthenticated HTTP server + OSC/UDP
  listener — anyone on the AP can hit these endpoints. Input validation
  (bounds checks, preset-name allow-list, CSV upload size limits) prevents
  crashes/file-overwrite, but there's still no auth layer — accepted risk,
  needs a product decision if that ever changes.

## History

Predecessors (`osclinga_3_1/`, `osclinga_3_1_oldpcb/`, `osclinga_3_2/`,
`osclinga_3_3/`) are kept alongside this folder at the repo root for
reference. `osclinga_3_3/` additionally had an `OLD_code/` archive (a
pre-`HAVE_*`-flag full-hardware copy) which is not carried forward here —
the single `src/` tree plus PlatformIO build environments now cover both
dev-board and real-PCB targets from one codebase.
