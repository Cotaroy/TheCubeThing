# THE CUBE THING

**THE CUBE THING** is an interactive, right-angle-edged, last-gen spinning cube and rectangular prism raytraced ASCII-art renderer. It features simulated lighting effects and a multi-process architecture to distribute rendering tasks to child processes.

- [Video Demo 1](https://youtu.be/UxQB3jfsUAE)
- [Video Demo 2](https://www.youtube.com/watch?v=-IppomLf0mg)

---

## Project Overview
This application belongs to **Category 1: Multi-Process Application Using Pipes**. The screen is drawn to the terminal using ASCII characters chosen to give the impression of brightness at each "pixel". To achieve this, raycast tasks are distributed across multiple worker processes.

### Key Features
* **3D Rendering:** Place cuboids (entities) and light sources in a digital 3D space to be rendered with simulated lighting.
* **Interactive Exploration:** Traverse the space using `wasd` to move and `hjkl` to rotate the camera.
* **Command Mode:** Enter a dedicated mode by pressing `:` to precisely place, move, or delete objects.
* **Automated Demos:** For demonstration, entity 0 spins automatically and light source 0 orbits the origin.

---

## Controls

### Movement & Camera
Use these keys while outside of "command mode" to navigate the environment.

| Key | Movement | Key | Facing Direction |
| :--- | :--- | :--- | :--- |
| **W** | Move Forward | **h** | Turn Left |
| **A** | Move Orthogonally Left | **j** | Turn Down |
| **S** | Move Backward | **k** | Turn Up |
| **D** | Move Orthogonally Right | **l** | Turn Right |

### Command Mode
Press `:` to enter command mode. Type the command and press **Enter** to execute. The simulation pauses during execution; press **Enter** a second time to resume.

| Command & Syntax | Effect |
| :--- | :--- |
| `exit` / alias: `q` | Exit the program |
| `list` | Displays all Entity and Light Source IDs and positions |
| `delete <e\|l> <id>` | Remove the entity (e) or light (l) with specified ID |
| `translate <e\|l> <id> <xoff> <yoff> <zoff>` | Add offsets to an object's position vector |
| `rotate <e\|l> <id> <axis> <angle> <cx> <cy> <cz>` | Rotate object by angle degrees about an axis shifted to a center |
| `brighten <id> <delta_intensity>` | Increases brightness of light with ID `id` |
| `create e <id> <x> <y> <z> <xl> <yl> <zl>` | Create a new cuboid with specified ID, position, and lengths |
| `create l <id> <x> <y> <z> <intensity>` | Create a new light with ID `id` at provided coordinates |

Entity 0 will always spin around its center.
Light 0 will always spin around the origin.

---

## Build & Run

### Instructions
1.  **Build:** Execute `make` in the terminal.
2.  **Run:** Launch the executable to enter an empty void:
    ```bash
    ./TheCubeThing
    ```
   
3.  **Demos:** You can pipe text files into the program to load prebuilt scenes at startup.
    ```bash
    ./TheCubeThing < scene.txt
    ```
