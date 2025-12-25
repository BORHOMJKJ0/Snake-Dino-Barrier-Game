# Snake · Dino · Barrier Game 🕹️

**A compact Arduino-based set of simple games for a 16×2 LCD using a joystick and two buttons.**

---

## Overview ✅

This repository contains an Arduino sketch (`Code/coachMalek/coachMalek.ino`) that implements three small games suitable for a 16×2 character LCD: **Snake**, **Dinosaur (Dino)**, and **Barrier**. The sketch uses a joystick for movement/selection and two push buttons for barrier controls. The UI is designed for low-resource hardware and is ideal for learning microcontroller games and simple input handling.

---

## Features ✨

- Menu to choose between three game modes
- Snake: eat apples, level up, and avoid collision
- Dino: jump over moving obstacles and level progression
- Barrier: two-player style left/right barrier race using buttons
- Adjustable speeds, thresholds and level goals are defined in the sketch

---

## Hardware Required 🔧

- Arduino Uno / Nano / compatible board
- 16×2 LCD (HD44780-compatible)
- Joystick (2-axis analog)
- 2 × momentary push-buttons
- Jumper wires and breadboard

---

## Wiring (as used in the sketch) 🔌

LCD pins (LiquidCrystal):

- RS -> D12
- E -> D11
- D4 -> D5
- D5 -> D4
- D6 -> D3
- D7 -> D2

Joystick:

- X-axis -> A0 (JOY_X)
- Y-axis -> A1 (JOY_Y)

Buttons:

- leftButton -> D8 (configured INPUT_PULLUP)
- rightButton -> D9 (configured INPUT_PULLUP)

> Note: The code expects buttons to be wired to GND and rely on the internal pull-up resistors (INPUT_PULLUP).

---

## Software / Libraries 🧩

- Arduino IDE (the original `Readme.txt` mentions Arduino IDE 2.3.4; any recent IDE should work)
- Built-in `LiquidCrystal` library (no additional libraries required by the sketch)

---

## Installation & Upload 🚀

1. Clone or download this repository:

   ```bash
   git clone <repo_url>
   ```

2. Open `Code/coachMalek/coachMalek.ino` in the Arduino IDE.
3. Select your board and port (Tools → Board / Port).
4. Install any missing libraries (the sketch uses `LiquidCrystal` which is bundled with the IDE).
5. Upload the sketch to your board.

---

## How to Play 🎮

Menu:

- Push joystick left → Start Snake
- Push joystick right → Start Dino
- Push joystick down → Start Barrier

Snake:

- Use joystick to move up/down/left/right
- Eat apples (`a`) to increase length and score
- Reach level targets to increase speed

Dino:

- Push joystick up to jump; push down to force landing
- Avoid `X` obstacles; score increases when obstacles pass

Barrier:

- Use the two push buttons (configured on D8/D9) to move the `|` barrier left or right
- First to move the barrier to either edge wins

---

## Configuration & Tuning 🛠️

Several constants at the top of `coachMalek.ino` control behavior:

- `JOY_LOW` / `JOY_HIGH` — adjust joystick thresholds for your hardware
- `baseMoveInterval` — initial speed for moving elements
- Level goals arrays (`snakeLevelGoals`, `dinoLevelGoals`) — change to adjust progression

---

## Troubleshooting & Tips ⚠️

- If movement is noisy or inaccurate, tune `JOY_LOW` and `JOY_HIGH` to match your joystick's center values (use `Serial.println()` for debugging analog readings).
- If buttons behave inverted, re-check wiring (they should connect to GND when pressed) or invert logic in code.
- The LCD is cleared frequently by the sketch; if you want to add colors/graphics, consider upgrading to a different display or using a graphical LCD.

---

## Contributing 🤝

Contributions, issues and feature requests are welcome. Please open an issue first for larger changes or create a pull request for bug fixes and incremental improvements.

---

## Credits & Acknowledgements 🙏

- Sketch shows `Coach Malek` on game over screens — likely the original author of the project.

---

## License

No license specified. If you want this project to be open source, I recommend adding an explicit license (MIT, Apache-2.0, etc.).

---

If you'd like, I can:

- Create wiring diagrams or a Fritzing sketch
- Add an image or GIF demonstrating the gameplay
- Propose a short contributor guide and add a license file

— GitHub Copilot (using Raptor mini (Preview))
