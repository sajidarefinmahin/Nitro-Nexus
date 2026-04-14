# Nitro Nexus

## Game Description

**Nitro Nexus** is an exciting action racing game created using the **iGraphics** library in C. The project demonstrates advanced graphics programming concepts like drawing shapes, handling user input, animations, particle effects, and dynamic difficulty systems.

## Features
- Three difficulty levels: Easy, Medium, and Hard
- Nitro boost system with visual effects (Medium & Hard modes)
- Innocent civilian system with penalty mechanics (Medium & Hard modes)
- Zombie enemy with special effects
- Dynamic road scrolling with lane markings
- Particle effects for collisions, explosions, and nitro flames
- Score tracking and leaderboard system
- Health system with collision detection
- Multiple enemy vehicle types
- Smooth animations and visual effects

## Project Details
- **IDE:** Visual Studio 2013
- **Language:** C, C++
- **Platform:** Windows PC
- **Genre:** 2D Action Racing/Arcade

## How to Run the Project

Make sure you have the following installed:
- **Visual Studio 2013**
- **MinGW Compiler** (if needed)
- **iGraphics Library** (included in this repository)

### Steps:
1. Open the project in Visual Studio 2013
2. Go to File → Open → Project/Solution
3. Locate and select the .sln file from the cloned repository
4. Click Build → Build Solution
5. Run the program by clicking Debug → Start Without Debugging

## How to Play

### **Controls**
| Action | Key |
|--------|-----|
| Move Left | `←` (Left Arrow) |
| Move Right | `→` (Right Arrow) |
| Move Up | `↑` (Up Arrow) |
| Move Down | `↓` (Down Arrow) |
| Shoot | `Enter` |
| Activate Nitro | `F` (Medium/Hard modes only) |
| Pause Game | Click PAUSE button |
| Toggle Music | `F1` |

### **Game Rules**

- Each player starts with 3 lives and 0 score
- Enemies reduce health on collision:
  - Regular cars: -1 life
  - Zombie: -1 life (disappears after collision)
- Points system:
  - Destroy regular enemy: +10 points
  - Destroy zombie: +20 points
  - Hitting innocent people: -20 points (Medium/Hard modes)
- Killing 2 innocent people costs 1 life (Medium/Hard modes)
- Nitro boost (Medium/Hard modes):
  - Press `F` to activate temporary speed boost
  - Limited duration with cooldown
  - Visual flame effects when active
- Game ends when health reaches 0
- Scores are saved with player name to leaderboard

## Project Contributors

1. Ahnaf
2. Snigdha
3. Minhal
4. Akib

## Game Modes

### Easy Mode
- Slower enemy speeds
- No innocent civilians
- No nitro system
- Ideal for beginners

### Medium Mode
- Faster enemy speeds
- Innocent civilians appear
- Nitro boost system available
- Balanced challenge

### Hard Mode
- Fastest enemy speeds
- Innocent civilians with stricter penalties
- Enhanced nitro system
- Maximum difficulty

## Visual Effects
- Explosion animations with particle systems
- Collision shockwave effects
- Nitro flame effects with dynamic particles
- Sad emoji animations for innocent casualties
- Glowing button effects
- Scrolling road with lane markings
- Dynamic background scenery

## YouTube Link
[CSE 1200 Project: Nitro Nexus](https://www.youtube.com/](https://youtube.com/shorts/xPmd6wZT2og?feature=share))

## Project Report
[Project Report: Nitro Nexus](https://drive.google.com/file/d/1GCkAtVIdb5IcFK6SUtD_Fk5k9Pj5bln1/view?usp=sharing)
