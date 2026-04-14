#define _CRT_SECURE_NO_WARNINGS

#include "iGraphics.h"
#include <math.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>


// ======================= Function Declarations ======================= //
void drawHomepage(void);
void drawStartPage(void);
void drawAboutPage(void);
void drawInstructionPage(void);
void drawScorePage(void);
void drawNameEntryPage(void);
void drawEasyPage(void);
void drawMediumPage(void);
void drawHardPage(void);

void startButtonClickHandler(void);
void aboutButtonClickHandler(void);
void instructionButtonClickHandler(void);
void scoreButtonClickHandler(void);
void backButtonClickHandler(void);
void easyButtonClickHandler(void);
void mediumButtonClickHandler(void);
void hardButtonClickHandler(void);
void playClickSound(void);

// ======================= UI States ======================= //
int homePage = 1, startPage = 0, aboutPage = 0, instructionPage = 0,
    scorePage = 0, nameEntryPage = 0;
int easyPage = 0, mediumPage = 0, hardPage = 0;

// ======================= Player ======================= //
int playerX = 470, playerY = 0;
int life = 3, score = 0, innocentKilled = 0;
int gameOver = 0;
int isPaused = 0;

// ======================= Name Entry System ======================= //
char playerName[32] = "";
int playerNameLen = 0;
int nameEntryCursorBlink = 0;
int lastGameScore = 0;
char lastGameMode[16] = "";

// ======================= Score Records ======================= //
#define MAX_SCORE_RECORDS 100
struct ScoreRecord {
  char name[32];
  int score;
  char mode[16];
  char date[32];
};
struct ScoreRecord scoreRecords[MAX_SCORE_RECORDS];
int totalRecords = 0;
int scoreScrollOffset = 0;

void loadScoreRecords(void) {
  totalRecords = 0;
  FILE *f = fopen("scores.txt", "r");
  if (!f) return;
  while (totalRecords < MAX_SCORE_RECORDS &&
         fscanf(f, "%31[^|]|%d|%15[^|]|%31[^\n]\n",
                scoreRecords[totalRecords].name,
                &scoreRecords[totalRecords].score,
                scoreRecords[totalRecords].mode,
                scoreRecords[totalRecords].date) == 4) {
    totalRecords++;
  }
  fclose(f);
}

void saveScoreRecord(const char *name, int sc, const char *mode) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  char dateStr[32];
  sprintf(dateStr, "%02d-%02d-%04d %02d:%02d",
          t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
          t->tm_hour, t->tm_min);

  FILE *f = fopen("scores.txt", "a");
  if (f) {
    fprintf(f, "%s|%d|%s|%s\n", name, sc, mode, dateStr);
    fclose(f);
  }
  loadScoreRecords();
}

void enterNameEntryPage(int finalScore, const char *mode) {
  lastGameScore = finalScore;
  strcpy(lastGameMode, mode);
  playerName[0] = '\0';
  playerNameLen = 0;
  nameEntryPage = 1;
  easyPage = mediumPage = hardPage = 0;
  scorePage = 0;
}
void drawPauseMenu(void);
void updatePlayerMovement(void);

// ======================= Lane System ======================= //
int laneX[4] = {300, 380, 460, 540};

// ======================= Enemy Cars ======================= //
float x = 300, y = 700;
float a = 380, b = 1000;
float c = 460, d = 1300;

// ======================= Zombies ======================= //
float zx = 540, zy = 1600;
int zombieAlive = 1;

// ======================= Innocent People ======================= //
#define MAX_INNOCENTS 3
struct Innocent {
  float x, y;
  int active;
  int lane;
};
struct Innocent innocents[MAX_INNOCENTS];
int innocentFire[MAX_INNOCENTS] = {0}; // For hit effect on innocents

// ======================= Bullet ======================= //
#define MAX_BULLETS 5
struct Bullet {
  int x, y, active;
};
struct Bullet bullets[MAX_BULLETS];
int bulletSpeed = 10;

// ======================= Fire Effects ======================= //
int fire1 = 0, fire2 = 0, fire3 = 0, fireZ = 0;
const int FIRE_TIME = 20;

// ======================= Collision Effect ======================= //
#define MAX_COLLISION_FX 5
struct CollisionFX {
  float x, y;
  int timer;
  int active;
};
struct CollisionFX collisionEffects[MAX_COLLISION_FX];
const int COLLISION_FX_TIME = 30;

void spawnCollisionEffect(float cx, float cy) {
  for (int i = 0; i < MAX_COLLISION_FX; i++) {
    if (!collisionEffects[i].active) {
      collisionEffects[i].x = cx;
      collisionEffects[i].y = cy;
      collisionEffects[i].timer = COLLISION_FX_TIME;
      collisionEffects[i].active = 1;
      return;
    }
  }
}

void drawCollisionEffects(void) {
  float PI = 3.14159265f;
  for (int i = 0; i < MAX_COLLISION_FX; i++) {
    if (collisionEffects[i].active) {
      float cx = collisionEffects[i].x;
      float cy = collisionEffects[i].y;
      int t = collisionEffects[i].timer;
      float intensity = (float)t / (float)COLLISION_FX_TIME;

      // Central impact flash (white -> yellow -> orange)
      if (t > COLLISION_FX_TIME * 0.7f) {
        iSetColor(255, 255, 255);
        iFilledCircle(cx, cy, 25 * intensity);
      }
      iSetColor(255, 200, 50);
      iFilledCircle(cx, cy, 18 * intensity);
      iSetColor(255, 100, 0);
      iFilledCircle(cx, cy, 10 * intensity);

      // Flying sparks/debris in all directions
      for (int s = 0; s < 8; s++) {
        float angle =
            (float)s * PI / 4.0f + (float)(COLLISION_FX_TIME - t) * 0.15f;
        float dist =
            (30.0f + (float)(COLLISION_FX_TIME - t) * 2.0f) * intensity;
        float sx = cx + dist * cos(angle);
        float sy = cy + dist * sin(angle);
        float sparkSize = (3.0f + (float)(s % 3)) * intensity;

        if (s % 3 == 0)
          iSetColor(255, 255, 100);
        else if (s % 3 == 1)
          iSetColor(255, 150, 0);
        else
          iSetColor(255, 80, 0);

        iFilledCircle(sx, sy, sparkSize);
      }

      // Smoke puffs rising
      for (int p = 0; p < 3; p++) {
        float offsetX = (float)((p * 23 + t * 5) % 40) - 20.0f;
        float riseY = (float)(COLLISION_FX_TIME - t) * 1.5f;
        float smokeR = (8.0f + (float)p * 3.0f) * intensity;
        int gray = 80 + (int)(80 * intensity);
        iSetColor(gray, gray, gray);
        iFilledCircle(cx + offsetX, cy + 20.0f + riseY, smokeR);
      }

      // Shockwave ring
      if (t > COLLISION_FX_TIME * 0.5f) {
        float ringR = (40.0f - 40.0f * intensity) + 20.0f;
        iSetColor(255, 200, 100);
        iCircle(cx, cy, ringR);
      }

      collisionEffects[i].timer--;
      if (collisionEffects[i].timer <= 0) {
        collisionEffects[i].active = 0;
      }
    }
  }
}

// ======================= Sad Emoji Effect ======================= //
#define MAX_SAD_FX 5
struct SadEmojiFX {
  float x, y;
  int timer;
  int active;
};
struct SadEmojiFX sadEffects[MAX_SAD_FX];
const int SAD_FX_TIME = 40;

void spawnSadEmoji(float sx, float sy) {
  for (int i = 0; i < MAX_SAD_FX; i++) {
    if (!sadEffects[i].active) {
      sadEffects[i].x = sx;
      sadEffects[i].y = sy;
      sadEffects[i].timer = SAD_FX_TIME;
      sadEffects[i].active = 1;
      return;
    }
  }
}

void drawSadEmojis(void) {
  for (int i = 0; i < MAX_SAD_FX; i++) {
    if (sadEffects[i].active) {
      float ex = sadEffects[i].x;
      float ey = sadEffects[i].y;
      int t = sadEffects[i].timer;
      float intensity = (float)t / (float)SAD_FX_TIME;
      float floatUp = (float)(SAD_FX_TIME - t) * 1.2f;
      float cy = ey + 40.0f + floatUp;
      float cx = ex + 20.0f;
      float r = 18.0f * intensity;

      if (r < 3.0f)
        r = 3.0f;

      // Face circle (yellow)
      iSetColor(255, 220, 50);
      iFilledCircle(cx, cy, r);

      // Face outline
      iSetColor(180, 150, 0);
      iCircle(cx, cy, r);

      if (r > 6.0f) {
        // Eyes (sad droopy eyes)
        float eyeOff = r * 0.3f;
        float eyeY = cy + r * 0.15f;
        float eyeR = r * 0.12f;
        if (eyeR < 1.5f)
          eyeR = 1.5f;

        // Left eye
        iSetColor(60, 40, 0);
        iFilledCircle(cx - eyeOff, eyeY, eyeR);
        // Right eye
        iFilledCircle(cx + eyeOff, eyeY, eyeR);

        // Tear drops
        iSetColor(100, 180, 255);
        float tearLen = r * 0.25f * (1.0f - intensity + 0.3f);
        iFilledCircle(cx - eyeOff, eyeY - eyeR - tearLen, eyeR * 0.7f);
        iFilledCircle(cx + eyeOff, eyeY - eyeR - tearLen * 1.3f, eyeR * 0.6f);

        // Sad mouth (frown arc using small circles)
        float mouthY = cy - r * 0.35f;
        float mouthW = r * 0.35f;
        iSetColor(60, 40, 0);
        for (int m = -3; m <= 3; m++) {
          float mx2 = cx + (float)m * (mouthW / 3.0f);
          float my2 = mouthY + (float)(m * m) * (r * 0.02f);
          iFilledCircle(mx2, my2, 1.2f);
        }
      }

      // "-20" text floating up
      iSetColor(255, 50, 50);
      char minusStr[10];
      sprintf(minusStr, "-20");
      iText(cx + r + 3, cy, minusStr, GLUT_BITMAP_HELVETICA_12);

      sadEffects[i].timer--;
      if (sadEffects[i].timer <= 0) {
        sadEffects[i].active = 0;
      }
    }
  }
}

// ======================= Glow Animation ======================= //
float glowPhase = 0.0f;

// ======================= Fire Effect Drawing ======================= //
void drawFireEffect(float fx, float fy, float w, float h, int timer) {
  float cx = fx + w / 2.0f;
  float cy = fy + h / 2.0f;
  float intensity = (float)timer / (float)FIRE_TIME;
  float PI = 3.14159265f;
  float elapsed = (float)(FIRE_TIME - timer);

  // ---- Layer 1: Large outer explosion glow (dark crimson) ----
  iSetColor(120, 10, 0);
  iFilledCircle(cx, cy, (w * 1.2f + 15) * intensity);

  // ---- Layer 2: Fiery orange shell ----
  iSetColor(220, 80, 0);
  iFilledCircle(cx, cy, (w * 0.9f + 10) * intensity);

  // ---- Layer 3: Bright orange-yellow middle ----
  iSetColor(255, 160, 20);
  iFilledCircle(cx, cy, (w * 0.65f + 5) * intensity);

  // ---- Layer 4: Hot white-yellow core ----
  iSetColor(255, 255, 150);
  iFilledCircle(cx, cy, (w * 0.3f) * intensity);

  // ---- Layer 5: White-hot center flash ----
  if (timer > FIRE_TIME * 0.6f) {
    iSetColor(255, 255, 255);
    iFilledCircle(cx, cy, (w * 0.15f) * intensity);
  }

  // ---- 10 rotating flame tongues ----
  for (int i = 0; i < 10; i++) {
    float angle = (float)i * PI / 5.0f + elapsed * 0.4f;
    float dist = (w * 0.5f + 12) * intensity;
    float flamex = cx + dist * cos(angle);
    float flamey = cy + dist * sin(angle);
    float flameR = (5.0f + (float)(i % 4) * 2.5f) * intensity;

    int colorIdx = i % 4;
    if (colorIdx == 0)
      iSetColor(255, 60, 0);
    else if (colorIdx == 1)
      iSetColor(255, 180, 0);
    else if (colorIdx == 2)
      iSetColor(255, 255, 80);
    else
      iSetColor(255, 100, 20);
    iFilledCircle(flamex, flamey, flameR);
  }

  // ---- Rising flame columns ----
  for (int i = 0; i < 6; i++) {
    float offsetX =
        (float)((i * 13 + (int)elapsed * 9) % (int)(w * 0.8f + 1)) - w * 0.4f;
    float riseH = (20.0f + (float)(i * 7)) * intensity;
    float flameR = (4.0f + (float)(i % 3) * 2.0f) * intensity;
    int ri = 255, gi = 120 + (i * 30), bi = 0;
    if (gi > 255)
      gi = 255;
    iSetColor(ri, gi, bi);
    iFilledCircle(cx + offsetX, cy + h * 0.4f + riseH * 0.6f, flameR);
  }

  // ---- Ember/spark particles flying outward ----
  for (int i = 0; i < 12; i++) {
    float angle = (float)i * PI / 6.0f + elapsed * 0.2f;
    float dist = (w * 0.3f + elapsed * 3.5f) * intensity;
    float ex = cx + dist * cos(angle);
    float ey = cy + dist * sin(angle);
    float sparkR = (2.0f + (float)(i % 3)) * intensity;

    if (i % 3 == 0)
      iSetColor(255, 255, 100);
    else if (i % 3 == 1)
      iSetColor(255, 200, 50);
    else
      iSetColor(255, 120, 0);
    iFilledCircle(ex, ey, sparkR);
  }

  // ---- Smoke puffs ----
  for (int i = 0; i < 5; i++) {
    float offsetX = (float)((i * 19 + (int)elapsed * 7) % 50) - 25.0f;
    float riseY = elapsed * 2.0f + (float)i * 5.0f;
    float smokeR = (6.0f + (float)i * 2.0f) * intensity;
    int gray = 50 + (int)(50 * intensity);
    iSetColor(gray, gray, gray);
    iFilledCircle(cx + offsetX, cy + h * 0.5f + riseY, smokeR);
  }

  // ---- Expanding shockwave ring ----
  if (timer > FIRE_TIME * 0.4f) {
    float ringR = (w * 0.6f + (1.0f - intensity) * 30.0f);
    iSetColor(255, 200, 80);
    iCircle(cx, cy, ringR);
    iSetColor(255, 150, 30);
    iCircle(cx, cy, ringR + 2);
  }
}

// ======================= Rocket/Nitro Visual Effect ======================= //
void drawRocketEffect(int px, int py) {
  float PI = 3.14159265f;
  float phase = glowPhase * 8.0f;
  float cx = (float)px + 22.0f;
  float baseY = (float)py - 5.0f;

  // ---- Outer exhaust glow (large reddish aura) ----
  iSetColor(200, 50, 0);
  iFilledCircle(cx, baseY - 15, 18 + 3.0f * sin(phase));

  // ---- Main flame body (orange-yellow cone) ----
  iSetColor(255, 140, 0);
  iFilledCircle(cx, baseY - 8, 12 + 2.0f * sin(phase * 1.3f));
  iFilledCircle(cx, baseY - 18, 10 + 2.0f * cos(phase));
  iFilledCircle(cx, baseY - 28, 7 + 1.5f * sin(phase * 0.9f));

  // ---- Bright yellow-white core ----
  iSetColor(255, 230, 50);
  iFilledCircle(cx, baseY - 6, 8 + 1.5f * sin(phase * 1.5f));
  iFilledCircle(cx, baseY - 16, 6 + 1.0f * cos(phase * 1.2f));

  // ---- White-hot center ----
  iSetColor(255, 255, 200);
  iFilledCircle(cx, baseY - 4, 5 + 1.0f * sin(phase * 2.0f));

  // ---- Blue afterburner tip ----
  iSetColor(80, 150, 255);
  iFilledCircle(cx, baseY - 2, 4 + 0.8f * sin(phase * 2.5f));

  // ---- Side exhaust plumes (left and right) ----
  for (int side = -1; side <= 1; side += 2) {
    float sx = cx + (float)side * 12.0f;
    iSetColor(255, 100, 0);
    iFilledCircle(sx, baseY - 4, 5 + 1.0f * sin(phase + side));
    iSetColor(255, 200, 50);
    iFilledCircle(sx, baseY - 10, 3.5f + 0.8f * cos(phase * 1.4f + side));
  }

  // ---- Trailing fire particles ----
  for (int i = 0; i < 8; i++) {
    float angle = PI + (float)i * PI / 8.0f - PI / 2.0f + sin(phase + i) * 0.3f;
    float dist = 20.0f + (float)i * 5.0f + 3.0f * sin(phase * 0.7f + i);
    float px2 = cx + dist * cos(angle) * 0.5f;
    float py2 = baseY + dist * sin(angle) * (-1.0f);
    float pR = 2.5f - (float)i * 0.25f;
    if (pR < 0.8f)
      pR = 0.8f;

    if (i % 3 == 0)
      iSetColor(255, 255, 100);
    else if (i % 3 == 1)
      iSetColor(255, 150, 0);
    else
      iSetColor(200, 50, 0);
    iFilledCircle(px2, py2 - 15, pR);
  }

  // ---- Heat shimmer (flickering small dots) ----
  for (int i = 0; i < 4; i++) {
    float sx = cx + (float)((int)(phase * 7 + i * 13) % 30 - 15);
    float sy = baseY - 35.0f - (float)i * 6.0f - 5.0f * sin(phase + i * 2.0f);
    int gray = 180 + (int)(40.0f * sin(phase * 3.0f + i));
    if (gray > 255)
      gray = 255;
    iSetColor(gray, gray / 2, 0);
    iFilledCircle(sx, sy, 1.5f);
  }
}

// ======================= Black Car (Nitro Mode) ======================= //
void drawBlackCar(int px, int py) {
  // Dark car body
  iSetColor(25, 25, 30);
  iFilledRectangle(px + 5, py + 10, 34, 68);

  // Car roof
  iSetColor(15, 15, 20);
  iFilledRectangle(px + 10, py + 30, 24, 30);

  // Windshield (dark blue tint)
  iSetColor(30, 50, 80);
  iFilledRectangle(px + 12, py + 55, 20, 12);

  // Rear window
  iSetColor(25, 40, 65);
  iFilledRectangle(px + 12, py + 32, 20, 8);

  // Front bumper
  iSetColor(40, 40, 45);
  iFilledRectangle(px + 3, py + 78, 38, 6);

  // Rear bumper
  iSetColor(40, 40, 45);
  iFilledRectangle(px + 3, py + 5, 38, 5);

  // Wheels (left)
  iSetColor(50, 50, 55);
  iFilledRectangle(px, py + 12, 6, 18);
  iFilledRectangle(px, py + 58, 6, 18);
  // Wheels (right)
  iFilledRectangle(px + 38, py + 12, 6, 18);
  iFilledRectangle(px + 38, py + 58, 6, 18);

  // Headlights (bright cyan-white glow)
  iSetColor(150, 230, 255);
  iFilledCircle(px + 10, py + 82, 3);
  iFilledCircle(px + 34, py + 82, 3);
  // Headlight glow
  iSetColor(100, 180, 255);
  iFilledCircle(px + 10, py + 82, 5);
  iFilledCircle(px + 34, py + 82, 5);

  // Taillights (red glow for nitro)
  iSetColor(255, 0, 0);
  iFilledCircle(px + 8, py + 7, 3);
  iFilledCircle(px + 36, py + 7, 3);
  iSetColor(255, 50, 50);
  iFilledCircle(px + 8, py + 7, 5);
  iFilledCircle(px + 36, py + 7, 5);

  // Racing stripe (neon blue)
  iSetColor(0, 120, 255);
  iFilledRectangle(px + 20, py + 10, 4, 68);

  // Side neon underglow
  float pulse = (float)(sin(glowPhase * 6.0f) * 0.5 + 0.5);
  int glowR = 0, glowG = (int)(100 + 100 * pulse), glowB = 255;
  iSetColor(glowR, glowG, glowB);
  iFilledRectangle(px + 1, py + 8, 2, 72);
  iFilledRectangle(px + 41, py + 8, 2, 72);
}

// ======================= Nitro State ======================= //
int nitroOn = 0;
int nitroTimer = 0;

// ======================= Nitro HUD ======================= //
void drawNitroHUD(void) {
  // Nitro gauge background
  iSetColor(20, 20, 30);
  iFilledRectangle(800, 555, 180, 40);
  iSetColor(60, 60, 80);
  iRectangle(800, 555, 180, 40);

  // Label
  iSetColor(200, 200, 200);
  iText(808, 578, "NITRO [F]", GLUT_BITMAP_HELVETICA_12);

  // Gauge bar background
  iSetColor(40, 40, 50);
  iFilledRectangle(808, 560, 164, 14);

  if (nitroOn && nitroTimer > 0) {
    // Active nitro bar
    float ratio = (float)nitroTimer / 200.0f;
    int barW = (int)(164.0f * ratio);

    // Gradient from cyan to blue
    int r = 0;
    int g = (int)(180 * ratio + 50);
    int b2 = 255;
    iSetColor(r, g, b2);
    iFilledRectangle(808, 560, barW, 14);

    // Bright edge glow
    iSetColor(100, 220, 255);
    iFilledRectangle(808 + barW - 3, 560, 3, 14);

    // "ACTIVE" text
    iSetColor(0, 255, 255);
    iText(875, 578, "ACTIVE", GLUT_BITMAP_HELVETICA_12);
  } else {
    // Ready text
    iSetColor(100, 255, 100);
    iText(875, 578, "READY", GLUT_BITMAP_HELVETICA_12);

    // Full green bar
    iSetColor(0, 180, 0);
    iFilledRectangle(808, 560, 164, 14);
  }
}

// ======================= Enhanced Bullet Drawing ======================= //
void drawBullet(int bx, int by) {
  // Bullet glow trail
  iSetColor(255, 255, 100);
  iFilledRectangle(bx - 1, by - 8, 6, 8);

  // Bullet core (bright)
  iSetColor(255, 255, 0);
  iFilledRectangle(bx, by, 4, 12);

  // Bullet tip (white hot)
  iSetColor(255, 255, 200);
  iFilledCircle(bx + 2, by + 12, 2.5f);

  // Trailing glow
  iSetColor(255, 200, 0);
  iFilledCircle(bx + 2, by - 4, 2.0f);
  iSetColor(255, 130, 0);
  iFilledCircle(bx + 2, by - 10, 1.5f);
}

// ======================= Button Glow Drawing ======================= //
void drawButtonGlow(float bx, float by, float bw, float bh) {
  // Pulsing glow effect around button
  float pulse = (float)(sin(glowPhase) * 0.5 + 0.5); // 0.0 to 1.0
  int layers = 4;

  for (int i = layers; i >= 1; i--) {
    float expand = (float)i * 3.0f;
    float alpha = pulse * (0.15f + 0.1f * (float)(layers - i));

    // Cyan-ish glow color with fading
    int r = (int)(0 + 80 * alpha);
    int g = (int)(180 * alpha + 100 * pulse);
    int b2 = (int)(255 * alpha + 50 * pulse);
    if (r > 255)
      r = 255;
    if (g > 255)
      g = 255;
    if (b2 > 255)
      b2 = 255;

    iSetColor(r, g, b2);
    iRectangle(bx - expand, by - expand, bw + expand * 2, bh + expand * 2);
  }
}

// -------- extra effects (medium) ----------
int bloodTimer = 0;
int bloodX = 0, bloodY = 0;

// ======================= Music ======================= //
int musicOn = 1;

// ======================= helper ======================= //
int getFreeLane(int l1, int l2, int l3) {
  int ln;
  int attempts = 0;
  do {
    ln = laneX[rand() % 4];
    attempts++;
    if (attempts > 20) break; // safety: avoid infinite loop
  } while (ln == l1 || ln == l2 || ln == l3);

  return ln;
}

// Check if a lane is too close vertically to cars/zombies at given Y
int isLaneOccupiedNearY(int laneXVal, float spawnY) {
  float minGap = 150.0f; // Minimum vertical gap between entities
  // Check car 1
  if ((int)x == laneXVal && fabs(y - spawnY) < minGap) return 1;
  // Check car 2
  if ((int)a == laneXVal && fabs(b - spawnY) < minGap) return 1;
  // Check car 3
  if ((int)c == laneXVal && fabs(d - spawnY) < minGap) return 1;
  // Check zombie
  if (zombieAlive && (int)zx == laneXVal && fabs(zy - spawnY) < minGap) return 1;
  return 0;
}

// ======================= Innocent People Functions ======================= //
void spawnInnocent(int index) {
  if (index >= 0 && index < MAX_INNOCENTS) {
    innocents[index].active = 1;
    innocentFire[index] = 0;

    // Pick a Y position well off-screen with spacing
    float spawnY = 700.0f + (float)(rand() % 400) + (float)(index * 250);

    // Try to find a lane not occupied by cars/zombies/other innocents
    int bestLane = rand() % 4;
    int found = 0;
    for (int attempt = 0; attempt < 8; attempt++) {
      int tryLane = (bestLane + attempt) % 4;
      int tryX = laneX[tryLane];
      int occupied = isLaneOccupiedNearY(tryX, spawnY);

      // Also check other active innocents
      if (!occupied) {
        for (int j = 0; j < MAX_INNOCENTS; j++) {
          if (j != index && innocents[j].active &&
              (int)innocents[j].x == tryX &&
              fabs(innocents[j].y - spawnY) < 120.0f) {
            occupied = 1;
            break;
          }
        }
      }

      if (!occupied) {
        bestLane = tryLane;
        found = 1;
        break;
      }
    }

    innocents[index].lane = bestLane;
    innocents[index].x = laneX[bestLane];
    innocents[index].y = spawnY;
  }
}

void initInnocents(void) {
  for (int i = 0; i < MAX_INNOCENTS; i++) {
    innocents[i].active = 1;
    innocentFire[i] = 0;
    // Stagger Y positions so they don't all appear at once
    innocents[i].y = 800.0f + (float)(i * 350) + (float)(rand() % 200);
    // Pick lanes avoiding cars/zombies
    int lane = (i + 1) % 4; // spread across lanes
    innocents[i].lane = lane;
    innocents[i].x = laneX[lane];
  }
}

void checkInnocentHit(int bulletIndex) {
  for (int i = 0; i < MAX_INNOCENTS; i++) {
    if (innocents[i].active && innocentFire[i] == 0 &&
        bullets[bulletIndex].x >= innocents[i].x &&
        bullets[bulletIndex].x <= innocents[i].x + 50 &&
        bullets[bulletIndex].y >= innocents[i].y &&
        bullets[bulletIndex].y <= innocents[i].y + 90) {
      // Penalty for killing innocent
      score -= 20;
      innocentKilled++;
      if (innocentKilled >= 2) {
        life--;
        innocentKilled = 0;
        if (life <= 0) {
          gameOver = 1;
        }
      }
      innocentFire[i] = FIRE_TIME; // Show hit effect
      bullets[bulletIndex].active = 0;
      // Spawn sad emoji effect
      spawnSadEmoji(innocents[i].x, innocents[i].y);
      // Respawn innocent after short delay
      innocents[i].active = 0;
      // Ensure score doesn't go negative (optional)
      if (score < 0)
        score = 0;
      break;
    }
  }
}

// ======================= Collision ======================= //
void collision(void) {
  if (fire1 == 0 && (playerX + 44 >= x && playerX <= x + 44) &&
      (playerY + 88 >= y && playerY <= y + 88)) {
    life--;
    fire1 = FIRE_TIME;
    spawnCollisionEffect((float)playerX + 22.0f, (float)playerY + 88.0f);
    y = 601;
    x = getFreeLane((int)a, (int)c, zombieAlive ? (int)zx : -1);
  }

  if (fire2 == 0 && (playerX + 44 >= a && playerX <= a + 44) &&
      (playerY + 88 >= b && playerY <= b + 88)) {
    life--;
    fire2 = FIRE_TIME;
    spawnCollisionEffect((float)playerX + 22.0f, (float)playerY + 88.0f);
    b = 801;
    a = getFreeLane((int)x, (int)c, zombieAlive ? (int)zx : -1);
  }

  if (fire3 == 0 && (playerX + 44 >= c && playerX <= c + 44) &&
      (playerY + 88 >= d && playerY <= d + 88)) {
    life--;
    fire3 = FIRE_TIME;
    spawnCollisionEffect((float)playerX + 22.0f, (float)playerY + 88.0f);
    d = 1001;
    c = getFreeLane((int)x, (int)a, zombieAlive ? (int)zx : -1);
  }

  if (zombieAlive && fireZ == 0 && (playerX + 44 >= zx && playerX <= zx + 50) &&
      (playerY + 88 >= zy && playerY <= zy + 90)) {

    life--;
    fireZ = FIRE_TIME;
    zombieAlive = 0;
    spawnCollisionEffect((float)playerX + 22.0f, (float)playerY + 88.0f);
    zy = 1201;
    zx = getFreeLane((int)x, (int)a, (int)c);
  }

  if (life <= 0) {
    gameOver = 1;
  }
}

// ======================= Background Rendering ======================= //
// Programmatic road rendering - no tile gaps possible
float roadScrollOffset = 0.0f;
const int ROAD_LEFT = 270;
const int ROAD_RIGHT = 630;
const int DASH_HEIGHT = 30;
const int DASH_GAP = 25;
const int DASH_CYCLE = DASH_HEIGHT + DASH_GAP; // 55px per dash cycle

void initializeImagePosition() { roadScrollOffset = 0.0f; }

void updateGlow() {
  glowPhase += 0.05f;
  if (glowPhase > 6.28318f)
    glowPhase -= 6.28318f;
}

void moveBackground() {
  if (isPaused || gameOver)
    return;

  float currentRoadSpeed = 8.0f; // Easy Mode
  if (mediumPage)
    currentRoadSpeed = 12.0f;
  if (hardPage)
    currentRoadSpeed = 16.0f;
  if (nitroOn && nitroTimer > 0)
    currentRoadSpeed += 8.0f;

  roadScrollOffset += currentRoadSpeed;
  while (roadScrollOffset >= 3300.0f) {
    roadScrollOffset -= 3300.0f;
  }
}

void renderring() {
  // ---- Road shoulder (dark green on both sides) ----
  iSetColor(30, 60, 20);
  iFilledRectangle(0, 0, ROAD_LEFT, 600);
  iFilledRectangle(ROAD_RIGHT, 0, 1000 - ROAD_RIGHT, 600);

  // ---- Main road surface (dark gray asphalt) ----
  iSetColor(55, 55, 60);
  iFilledRectangle(ROAD_LEFT, 0, ROAD_RIGHT - ROAD_LEFT, 600);

  // ---- Slight road texture (subtle darker strips) ----
  iSetColor(48, 48, 53);
  iFilledRectangle(ROAD_LEFT + 30, 0, 4, 600);
  iFilledRectangle(ROAD_LEFT + 70, 0, 3, 600);
  iFilledRectangle(ROAD_RIGHT - 35, 0, 4, 600);
  iFilledRectangle(ROAD_RIGHT - 75, 0, 3, 600);

  // ---- Road edge lines (solid white) ----
  iSetColor(220, 220, 220);
  iFilledRectangle(ROAD_LEFT, 0, 4, 600);
  iFilledRectangle(ROAD_RIGHT - 4, 0, 4, 600);

  // ---- Scrolling lane dash markings ----
  // 4 lanes need 3 lane dividers
  int laneWidth = (ROAD_RIGHT - ROAD_LEFT) / 4;
  for (int lane = 1; lane <= 3; lane++) {
    int laneLineX = ROAD_LEFT + lane * laneWidth - 2;

    // Yellow center line for lane 2 (center), white for others
    if (lane == 2) {
      iSetColor(255, 200, 0);
    } else {
      iSetColor(200, 200, 200);
    }

    // Draw scrolling dashes
    float startY = -fmodf(roadScrollOffset, (float)DASH_CYCLE);
    while (startY < 620) {
      int dashY = (int)startY;
      int dashEndY = dashY + DASH_HEIGHT;

      // Clamp to screen
      if (dashEndY > 0 && dashY < 600) {
        int drawY = dashY < 0 ? 0 : dashY;
        int drawH = (dashEndY > 600 ? 600 : dashEndY) - drawY;
        if (drawH > 0) {
          if (lane == 2) {
            // Double yellow center line
            iFilledRectangle(laneLineX - 2, drawY, 2, drawH);
            iFilledRectangle(laneLineX + 3, drawY, 2, drawH);
          } else {
            iFilledRectangle(laneLineX, drawY, 4, drawH);
          }
        }
      }
      startY += (float)DASH_CYCLE;
    }
  }

  // ---- Shoulder rumble strips ----
  iSetColor(180, 50, 50);
  float stripStart = -fmodf(roadScrollOffset * 0.8f, 30.0f);
  while (stripStart < 600) {
    if (stripStart > -15) {
      int sy = (int)stripStart;
      if (sy >= 0 && sy < 600) {
        iFilledRectangle(ROAD_LEFT - 8, sy, 8, 10);
        iFilledRectangle(ROAD_RIGHT, sy, 8, 10);
      }
    }
    stripStart += 30.0f;
  }

  // ---- Moving Trees Scenery ----
  float treeOffset = fmodf(roadScrollOffset * 0.7f, 300.0f);

  for (int i = -1; i < 4; i++) {
    float cy = 600.0f - (treeOffset + i * 300.0f);

    // Tree 1 (Left side)
    iSetColor(101, 67, 33); // Trunk brown
    iFilledRectangle(80, cy, 15, 30);
    iSetColor(15, 90, 25); // Dark Green leaves
    iFilledCircle(87, cy + 30, 25);
    iSetColor(34, 139, 34); // Lighter Green
    iFilledCircle(75, cy + 20, 20);
    iFilledCircle(100, cy + 20, 20);

    // Tree 2 (far left)
    iSetColor(101, 67, 33);
    iFilledRectangle(30, cy + 150, 10, 25);
    iSetColor(15, 80, 20);
    iFilledCircle(35, cy + 175, 20);
    iSetColor(34, 120, 34);
    iFilledCircle(25, cy + 165, 15);
    iFilledCircle(45, cy + 165, 15);

    // Tree 3 (Right side)
    iSetColor(101, 67, 33);
    iFilledRectangle(ROAD_RIGHT + 60, cy - 80, 15, 30);
    iSetColor(15, 90, 25);
    iFilledCircle(ROAD_RIGHT + 67, cy - 50, 25);
    iSetColor(34, 139, 34);
    iFilledCircle(ROAD_RIGHT + 55, cy - 60, 20);
    iFilledCircle(ROAD_RIGHT + 80, cy - 60, 20);

    // Tree 4 (far right)
    iSetColor(101, 67, 33);
    iFilledRectangle(ROAD_RIGHT + 130, cy + 70, 12, 28);
    iSetColor(15, 85, 22);
    iFilledCircle(ROAD_RIGHT + 136, cy + 98, 22);
    iSetColor(34, 125, 34);
    iFilledCircle(ROAD_RIGHT + 124, cy + 88, 18);
    iFilledCircle(ROAD_RIGHT + 148, cy + 88, 18);
  }
}

// ======================= HUD Drawing ======================= //
void drawHUD(int currentLife, int currentScore) {
  // Dark background panel for readability
  iSetColor(0, 0, 0);
  iFilledRectangle(5, 555, 280, 40);
  // Slightly lighter border
  iSetColor(60, 60, 80);
  iRectangle(5, 555, 280, 40);

  // Life text in green
  char lifeStr[40];
  sprintf(lifeStr, "Life: %d", currentLife);
  iSetColor(50, 255, 50);
  iText(15, 568, lifeStr, GLUT_BITMAP_TIMES_ROMAN_24);

  // Score text in yellow
  char scoreStr[40];
  sprintf(scoreStr, "Score: %d", currentScore);
  iSetColor(255, 255, 0);
  iText(150, 568, scoreStr, GLUT_BITMAP_TIMES_ROMAN_24);
  // Pause button (Bottom Left)
  iSetColor(200, 200, 200);
  iFilledRectangle(10, 10, 100, 30);
  iSetColor(0, 0, 0);
  iText(30, 18, "PAUSE", GLUT_BITMAP_TIMES_ROMAN_24);
}

void drawGameOverScreen(int finalScore) {
  iShowBMP2(0, 0, "image\\score.bmp", 0);

  // Large centered dark panel
  iSetColor(0, 0, 0);
  iFilledRectangle(150, 180, 700, 280);
  // Border - double gold border
  iSetColor(255, 200, 0);
  iRectangle(150, 180, 700, 280);
  iRectangle(152, 182, 696, 276);

  // GAME OVER title
  iSetColor(255, 50, 50);
  iText(370, 420, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);

  // Decorative line
  iSetColor(255, 200, 0);
  iLine(250, 405, 750, 405);

  // Final score - large and clear
  char str[80];
  sprintf(str, "Final Score : %d", finalScore);
  iSetColor(255, 255, 255);
  iText(350, 360, str, GLUT_BITMAP_TIMES_ROMAN_24);

  // Instruction text
  iSetColor(100, 255, 255);
  iText(310, 310, "Press ENTER to save your score", GLUT_BITMAP_HELVETICA_18);

  // Or click back
  iSetColor(180, 180, 180);
  iText(340, 270, "Click BACK to return to menu", GLUT_BITMAP_HELVETICA_18);

  // Mode display
  char modeStr[40];
  if (easyPage) sprintf(modeStr, "Mode: EASY");
  else if (mediumPage) sprintf(modeStr, "Mode: MEDIUM");
  else if (hardPage) sprintf(modeStr, "Mode: HARD");
  else sprintf(modeStr, "Mode: ---");
  iSetColor(200, 200, 0);
  iText(420, 220, modeStr, GLUT_BITMAP_HELVETICA_18);

  // Back button
  iShowBMP2(450, 20, "image\\back.bmp", 0);
}

// ======================= Name Entry Page ======================= //
void drawNameEntryPage(void) {
  updateGlow();
  iShowBMP2(0, 0, "image\\score.bmp", 0);

  // --- Main panel ---
  iSetColor(0, 0, 0);
  iFilledRectangle(100, 120, 800, 380);
  // Double gold border
  iSetColor(255, 200, 0);
  iRectangle(100, 120, 800, 380);
  iRectangle(102, 122, 796, 376);

  // --- Title: "ENTER YOUR NAME" with glow ---
  float pulse = (float)(sin(glowPhase * 2.0) * 0.5 + 0.5);
  int tr = (int)(255 * pulse + 100);
  int tg = (int)(200 * pulse + 55);
  if (tr > 255) tr = 255;
  if (tg > 255) tg = 255;
  iSetColor(tr, tg, 0);
  iText(340, 450, "ENTER YOUR NAME", GLUT_BITMAP_TIMES_ROMAN_24);

  // Decorative line under title
  iSetColor(255, 200, 0);
  iLine(200, 435, 800, 435);

  // --- Score display ---
  char scoreStr[80];
  sprintf(scoreStr, "Your Score: %d", lastGameScore);
  iSetColor(255, 255, 255);
  iText(390, 400, scoreStr, GLUT_BITMAP_TIMES_ROMAN_24);

  // Mode display
  char modeStr[40];
  sprintf(modeStr, "Mode: %s", lastGameMode);
  iSetColor(200, 200, 0);
  iText(420, 370, modeStr, GLUT_BITMAP_HELVETICA_18);

  // --- Name input box ---
  // Background
  iSetColor(30, 30, 50);
  iFilledRectangle(250, 270, 500, 60);
  // Glowing border
  int borderG = (int)(150 + 105 * pulse);
  iSetColor(0, borderG, 255);
  iRectangle(250, 270, 500, 60);
  iRectangle(249, 269, 502, 62);

  // Label
  iSetColor(180, 180, 200);
  iText(260, 340, "Player Name:", GLUT_BITMAP_HELVETICA_12);

  // Name text
  iSetColor(255, 255, 255);
  iText(270, 290, playerName, GLUT_BITMAP_TIMES_ROMAN_24);

  // Blinking cursor
  nameEntryCursorBlink++;
  if (nameEntryCursorBlink > 60) nameEntryCursorBlink = 0;
  if (nameEntryCursorBlink < 35) {
    // Calculate approximate cursor x position
    int cursorX = 270 + playerNameLen * 12;
    iSetColor(0, 200, 255);
    iFilledRectangle(cursorX, 280, 2, 30);
  }

  // --- Submit button ---
  drawButtonGlow(380, 190, 240, 50);
  iSetColor(40, 80, 40);
  iFilledRectangle(380, 190, 240, 50);
  iSetColor(0, 255, 100);
  iRectangle(380, 190, 240, 50);
  iSetColor(255, 255, 255);
  iText(430, 207, "SUBMIT  [Enter]", GLUT_BITMAP_TIMES_ROMAN_24);

  // --- Skip button ---
  iSetColor(60, 60, 60);
  iFilledRectangle(420, 140, 160, 35);
  iSetColor(150, 150, 150);
  iRectangle(420, 140, 160, 35);
  iSetColor(180, 180, 180);
  iText(455, 150, "Skip  [Esc]", GLUT_BITMAP_HELVETICA_18);

  // Back button
  iShowBMP2(450, 20, "image\\back.bmp", 0);
}

// ======================= iDraw ======================= //
void iDraw(void) {
  iClear();
  if (homePage)
    drawHomepage();
  else if (startPage)
    drawStartPage();
  else if (aboutPage)
    drawAboutPage();
  else if (instructionPage)
    drawInstructionPage();
  else if (nameEntryPage)
    drawNameEntryPage();
  else if (scorePage)
    drawScorePage();
  else if (easyPage)
    drawEasyPage();
  else if (mediumPage)
    drawMediumPage();
  else if (hardPage)
    drawHardPage();
}

// ======================= Mouse ======================= //
void iMouseMove(int mx, int my) {}
void iPassiveMouseMove(int mx, int my) {}

void iMouse(int button, int state, int mx, int my) {
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    if (homePage && mx >= 370 && mx <= 570 && my >= 380 && my <= 450)
      startButtonClickHandler();
    else if (homePage && mx >= 370 && mx <= 570 && my >= 280 && my <= 350)
      aboutButtonClickHandler();
    else if (homePage && mx >= 370 && mx <= 570 && my >= 180 && my <= 250)
      instructionButtonClickHandler();
    else if (homePage && mx >= 370 && mx <= 570 && my >= 80 && my <= 150)
      scoreButtonClickHandler();
    else if ((startPage || aboutPage || instructionPage || scorePage ||
              nameEntryPage || easyPage || mediumPage || hardPage) &&
             mx >= 450 && mx <= 570 && my >= 20 && my <= 60)
      backButtonClickHandler();
    else if (startPage && mx >= 0 && mx <= 200 && my >= 500 && my <= 550)
      easyButtonClickHandler();
    else if (startPage && mx >= 200 && mx <= 400 && my >= 500 && my <= 550)
      mediumButtonClickHandler();
    else if (startPage && mx >= 400 && mx <= 600 && my >= 500 && my <= 550)
      hardButtonClickHandler();
    else if (nameEntryPage) {
      // Submit button click
      if (mx >= 380 && mx <= 620 && my >= 190 && my <= 240) {
        if (playerNameLen > 0) {
          saveScoreRecord(playerName, lastGameScore, lastGameMode);
        }
        nameEntryPage = 0;
        scorePage = 1;
        loadScoreRecords();
        scoreScrollOffset = 0;
        playClickSound();
      }
      // Skip button click
      else if (mx >= 420 && mx <= 580 && my >= 140 && my <= 175) {
        nameEntryPage = 0;
        scorePage = 1;
        loadScoreRecords();
        scoreScrollOffset = 0;
        playClickSound();
      }
    }
    else if (scorePage) {
      // Scroll up button
      if (mx >= 900 && mx <= 960 && my >= 530 && my <= 560) {
        if (scoreScrollOffset > 0) scoreScrollOffset--;
        playClickSound();
      }
      // Scroll down button
      else if (mx >= 900 && mx <= 960 && my >= 100 && my <= 130) {
        if (scoreScrollOffset < totalRecords - 8 && totalRecords > 8) scoreScrollOffset++;
        playClickSound();
      }
    }
    else if (easyPage || mediumPage || hardPage) {
      if (gameOver) {
        // On game over screen, pressing Enter goes to name entry
        // Back button still works
      }
      else if (!gameOver && !isPaused) {
        if (mx >= 10 && mx <= 110 && my >= 10 && my <= 40) {
          isPaused = 1;
          playClickSound();
        }
      } else if (isPaused) {
        if (mx >= 350 && mx <= 650 && my >= 320 && my <= 370) {
          isPaused = 0;
          playClickSound();
        } else if (mx >= 350 && mx <= 650 && my >= 250 && my <= 300) {
          isPaused = 0;
          backButtonClickHandler();
        } else if (mx >= 350 && mx <= 650 && my >= 180 && my <= 230) {
          exit(0);
        }
      }
    }
  }
}

// ======================= Pause / Player Movement ======================= //
void updatePlayerMovement(void) {
  if (isPaused || gameOver || homePage || startPage || aboutPage ||
      instructionPage || scorePage)
    return;

  float speed = (nitroOn && nitroTimer > 0) ? 8.0f : 5.0f;

  if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
    playerX += (int)speed;
    if (playerX > 600 - 44)
      playerX = 600 - 44;
  }
  if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
    playerX -= (int)speed;
    if (playerX < 280)
      playerX = 280;
  }
  if (GetAsyncKeyState(VK_UP) & 0x8000) {
    playerY += (int)speed;
    if (playerY > 500)
      playerY = 500;
  }
  if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
    playerY -= (int)speed;
    if (playerY < 0)
      playerY = 0;
  }
}

void drawPauseMenu(void) {
  iSetColor(0, 0, 0);
  iFilledRectangle(300, 150, 400, 300);
  iSetColor(255, 200, 0);
  iRectangle(300, 150, 400, 300);
  iRectangle(302, 152, 396, 296);

  iSetColor(255, 255, 255);
  iText(450, 400, "PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);

  iSetColor(50, 50, 50);
  iFilledRectangle(350, 320, 300, 50);
  iSetColor(200, 200, 200);
  iRectangle(350, 320, 300, 50);
  iSetColor(255, 255, 255);
  iText(460, 335, "Resume", GLUT_BITMAP_TIMES_ROMAN_24);

  iSetColor(50, 50, 50);
  iFilledRectangle(350, 250, 300, 50);
  iSetColor(200, 200, 200);
  iRectangle(350, 250, 300, 50);
  iSetColor(255, 255, 255);
  iText(440, 265, "Main Menu", GLUT_BITMAP_TIMES_ROMAN_24);

  iSetColor(50, 50, 50);
  iFilledRectangle(350, 180, 300, 50);
  iSetColor(200, 200, 200);
  iRectangle(350, 180, 300, 50);
  iSetColor(255, 255, 255);
  iText(465, 195, "Exit", GLUT_BITMAP_TIMES_ROMAN_24);
}

// ======================= Keyboard ======================= //
void iKeyboard(unsigned char key) {
  // --- Name entry page input ---
  if (nameEntryPage) {
    if (key == '\r' || key == '\n') {
      // Submit
      if (playerNameLen > 0) {
        saveScoreRecord(playerName, lastGameScore, lastGameMode);
      }
      nameEntryPage = 0;
      scorePage = 1;
      loadScoreRecords();
      scoreScrollOffset = 0;
      playClickSound();
    } else if (key == 27) {
      // Escape = skip
      nameEntryPage = 0;
      scorePage = 1;
      loadScoreRecords();
      scoreScrollOffset = 0;
      playClickSound();
    } else if (key == 8) {
      // Backspace
      if (playerNameLen > 0) {
        playerNameLen--;
        playerName[playerNameLen] = '\0';
      }
    } else if (playerNameLen < 20 && key >= 32 && key <= 126 && key != '|') {
      // Printable characters (exclude pipe for file format safety)
      playerName[playerNameLen] = key;
      playerNameLen++;
      playerName[playerNameLen] = '\0';
    }
    return;
  }

  // --- Game over: Enter goes to name entry ---
  if (gameOver && (easyPage || mediumPage || hardPage)) {
    if (key == '\r' || key == '\n') {
      const char *mode = "EASY";
      if (mediumPage) mode = "MEDIUM";
      if (hardPage) mode = "HARD";
      enterNameEntryPage(score, mode);
      gameOver = 0;
      playClickSound();
    }
    return;
  }

  if (key == '\r') {
    for (int i = 0; i < MAX_BULLETS; i++) {
      if (!bullets[i].active) {
        bullets[i].active = 1;
        bullets[i].x = playerX + 22;
        bullets[i].y = playerY + 88;
        break;
      }
    }
  }

  if (key == 'f' || key == 'F') {
    // Nitro only in medium and hard modes
    if (mediumPage || hardPage) {
      nitroOn = 1;
      nitroTimer = 200;
    }
  }
}

// ======================= Special Keyboard ======================= //
void iSpecialKeyboard(unsigned char key) {
  if (key == GLUT_KEY_F1) {
    if (musicOn) {
      musicOn = 0;
      PlaySound(0, 0, 0);
    } else {
      musicOn = 1;
      PlaySound("gamemusic.wav", NULL, SND_LOOP | SND_ASYNC);
    }
  }
}

// ======================= Sound ======================= //
void playClickSound(void) { mciSendString("play click.wav", NULL, 0, NULL); }

// ======================= Pages ======================= //
void drawHomepage(void) {
  updateGlow();
  iShowBMP2(0, 0, "image\\nitro nexus.bmp", 0);

  // Draw glow on buttons
  drawButtonGlow(370, 380, 200, 70);
  drawButtonGlow(370, 280, 200, 70);
  drawButtonGlow(370, 180, 200, 70);
  drawButtonGlow(370, 80, 200, 70);

  iShowBMP2(370, 380, "image\\button1.bmp", 0);
  iShowBMP2(370, 280, "image\\button2.bmp", 0);
  iShowBMP2(370, 180, "image\\button3.bmp", 0);
  iShowBMP2(370, 80, "image\\button4.bmp", 0);
  iSetColor(255, 255, 255);
  iText(450, 407, "Start", GLUT_BITMAP_TIMES_ROMAN_24);
  iText(428, 307, "About us", GLUT_BITMAP_TIMES_ROMAN_24);
  iText(420, 207, "Instruction", GLUT_BITMAP_TIMES_ROMAN_24);
  iText(440, 107, "Score", GLUT_BITMAP_TIMES_ROMAN_24);
}

void drawStartPage(void) {
  updateGlow();
  iShowBMP2(0, 0, "image\\car1.bmp", 0);

  // Draw glow on difficulty buttons
  drawButtonGlow(0, 500, 200, 50);
  drawButtonGlow(200, 500, 200, 50);
  drawButtonGlow(400, 500, 200, 50);
  drawButtonGlow(450, 20, 120, 40);

  iShowBMP2(0, 500, "image\\button1.bmp", 0);
  iSetColor(255, 255, 255);
  iText(70, 530, "EASY", GLUT_BITMAP_TIMES_ROMAN_24);
  iShowBMP2(200, 500, "image\\button4.bmp", 0);
  iSetColor(255, 255, 255);
  iText(250, 530, "MEDIUM", GLUT_BITMAP_TIMES_ROMAN_24);
  iShowBMP2(400, 500, "image\\button2.bmp", 0);
  iSetColor(255, 255, 255);
  iText(465, 530, "HARD", GLUT_BITMAP_TIMES_ROMAN_24);
  iShowBMP2(450, 20, "image\\back.bmp", 0);
}

void drawAboutPage(void) {
  iShowBMP2(0, 0, "image\\about.bmp", 0);
  iShowBMP2(450, 20, "image\\back.bmp", 0);
}

void drawInstructionPage(void) {
  iShowBMP2(0, 0, "image\\instruction.bmp", 0);
  iShowBMP2(450, 20, "image\\back.bmp", 0);
}

void drawScorePage(void) {
  updateGlow();
  iShowBMP2(0, 0, "image\\score.bmp", 0);

  // --- Title Panel (below the "play, repeat, win" line) ---
  iSetColor(0, 0, 0);
  iFilledRectangle(50, 420, 900, 45);
  iSetColor(255, 200, 0);
  iRectangle(50, 420, 900, 45);
  float pulse = (float)(sin(glowPhase * 2.0) * 0.5 + 0.5);
  int titleR = (int)(255 * pulse + 150);
  if (titleR > 255) titleR = 255;
  iSetColor(titleR, 200, 0);
  iText(350, 435, "SCORE RECORDS", GLUT_BITMAP_TIMES_ROMAN_24);

  // --- Table header ---
  iSetColor(20, 20, 40);
  iFilledRectangle(50, 385, 900, 32);
  iSetColor(255, 200, 0);
  iRectangle(50, 385, 900, 32);

  iSetColor(255, 255, 200);
  iText(75, 395, "#", GLUT_BITMAP_HELVETICA_18);
  iText(130, 395, "PLAYER NAME", GLUT_BITMAP_HELVETICA_18);
  iText(400, 395, "SCORE", GLUT_BITMAP_HELVETICA_18);
  iText(560, 395, "MODE", GLUT_BITMAP_HELVETICA_18);
  iText(720, 395, "DATE / TIME", GLUT_BITMAP_HELVETICA_18);

  // --- Table rows ---
  int rowHeight = 38;
  int startY = 348;
  int maxVisible = 7;

  if (totalRecords == 0) {
    // No records message
    iSetColor(10, 10, 30);
    iFilledRectangle(50, 150, 900, 230);
    iSetColor(60, 60, 80);
    iRectangle(50, 150, 900, 230);
    iSetColor(180, 180, 180);
    iText(350, 280, "No game records yet.", GLUT_BITMAP_TIMES_ROMAN_24);
    iSetColor(120, 120, 150);
    iText(310, 240, "Play a game and enter your name!", GLUT_BITMAP_HELVETICA_18);
  } else {
    // Display records (most recent first)
    for (int i = 0; i < maxVisible && (i + scoreScrollOffset) < totalRecords; i++) {
      int recIdx = totalRecords - 1 - (i + scoreScrollOffset); // Most recent first
      int rowY = startY - i * rowHeight;

      // Alternate row colors
      if (i % 2 == 0) {
        iSetColor(15, 15, 35);
      } else {
        iSetColor(25, 25, 50);
      }
      iFilledRectangle(50, rowY - 5, 900, rowHeight - 5);

      // Row border
      iSetColor(50, 50, 80);
      iRectangle(50, rowY - 5, 900, rowHeight - 5);

      // Rank number
      char rankStr[8];
      sprintf(rankStr, "%d", i + scoreScrollOffset + 1);
      iSetColor(150, 150, 200);
      iText(75, rowY + 5, rankStr, GLUT_BITMAP_HELVETICA_18);

      // Player name (cyan)
      iSetColor(0, 220, 255);
      iText(130, rowY + 5, scoreRecords[recIdx].name, GLUT_BITMAP_HELVETICA_18);

      // Score (gold/yellow)
      char scStr[20];
      sprintf(scStr, "%d", scoreRecords[recIdx].score);
      iSetColor(255, 255, 0);
      iText(410, rowY + 5, scStr, GLUT_BITMAP_HELVETICA_18);

      // Mode with color coding
      if (strcmp(scoreRecords[recIdx].mode, "EASY") == 0)
        iSetColor(100, 255, 100);
      else if (strcmp(scoreRecords[recIdx].mode, "MEDIUM") == 0)
        iSetColor(255, 200, 50);
      else
        iSetColor(255, 80, 80);
      iText(560, rowY + 5, scoreRecords[recIdx].mode, GLUT_BITMAP_HELVETICA_18);

      // Date/time
      iSetColor(180, 180, 200);
      iText(720, rowY + 5, scoreRecords[recIdx].date, GLUT_BITMAP_HELVETICA_18);
    }

    // --- Scroll indicators ---
    if (totalRecords > maxVisible) {
      // Scroll up button
      iSetColor(40, 40, 60);
      iFilledRectangle(960, 400, 30, 30);
      iSetColor(200, 200, 200);
      iRectangle(960, 400, 30, 30);
      iSetColor(255, 255, 255);
      iText(967, 408, "^", GLUT_BITMAP_HELVETICA_18);

      // Scroll down button
      iSetColor(40, 40, 60);
      iFilledRectangle(960, 90, 30, 30);
      iSetColor(200, 200, 200);
      iRectangle(960, 90, 30, 30);
      iSetColor(255, 255, 255);
      iText(967, 98, "v", GLUT_BITMAP_HELVETICA_18);

      // Scroll info
      char scrollInfo[40];
      sprintf(scrollInfo, "%d-%d of %d",
              scoreScrollOffset + 1,
              (scoreScrollOffset + maxVisible > totalRecords) ? totalRecords : scoreScrollOffset + maxVisible,
              totalRecords);
      iSetColor(150, 150, 180);
      iText(940, 75, scrollInfo, GLUT_BITMAP_HELVETICA_12);
    }
  }

  iShowBMP2(450, 20, "image\\back.bmp", 0);
}

// ======================= EASY GAME ======================= //
void drawEasyPage(void) {
  if (gameOver) {
    drawGameOverScreen(score);
    return;
  }

  // ---------- EASY SPEED (medium Ã Â¦Â¥Ã Â§â€¡Ã Â¦â€¢Ã Â§â€¡ Ã Â¦â€¢Ã Â¦Â®)
  // ----------
  static float s1 = 1.5f, s2 = 1.7f, s3 = 1.9f, sz = 1.6f;

  iShowBMP2(0, 0, "image\\road.bmp", 0);
  renderring();

  // draw enemies
  if (fire1 == 0)
    iShowBMP2(x, y, "image\\v1.bmp", 0);
  else
    drawFireEffect(x, y, 44, 88, fire1);
  if (fire2 == 0)
    iShowBMP2(a, b, "image\\v2.bmp", 0);
  else
    drawFireEffect(a, b, 44, 88, fire2);
  if (fire3 == 0)
    iShowBMP2(c, d, "image\\v3.bmp", 0);
  else
    drawFireEffect(c, d, 44, 88, fire3);
  if (zombieAlive)
    iShowBMP2(zx, zy, "image\\zombie.bmp", 0);
  else if (fireZ > 0)
    drawFireEffect(zx, zy, 50, 90, fireZ);

  // player
  iShowBMP2(playerX, playerY, "image\\carhero.bmp", 0);

  // Enhanced bullet drawing
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      drawBullet(bullets[i].x, bullets[i].y);
    }
  }

  // ---------------- move enemies (slow) ----------------
  if (!isPaused) {
    if (fire1 == 0) {
      y -= s1;
      if (y <= -100) {
        y = 601;
        x = getFreeLane((int)a, (int)c, zombieAlive ? (int)zx : -1);
      }
    }

    if (fire2 == 0) {
      b -= s2;
      if (b <= -100) {
        b = 801;
        a = getFreeLane((int)x, (int)c, zombieAlive ? (int)zx : -1);
      }
    }

    if (fire3 == 0) {
      d -= s3;
      if (d <= -100) {
        d = 1001;
        c = getFreeLane((int)x, (int)a, zombieAlive ? (int)zx : -1);
      }
    }

    if (zombieAlive) {
      zy -= sz;
      if (zy <= -100) {
        zy = 1201;
        zx = getFreeLane((int)x, (int)a, (int)c);
      }
    }

    // ---------------- bullets ----------------
    for (int i = 0; i < MAX_BULLETS; i++) {
      if (bullets[i].active) {
        bullets[i].y += bulletSpeed;

        // Bullet hit detection only (drawing done above)
        if (fire1 == 0 && bullets[i].x >= x && bullets[i].x <= x + 44 &&
            bullets[i].y >= y && bullets[i].y <= y + 88) {
          fire1 = FIRE_TIME;
          bullets[i].active = 0;
          score += 10;
        }

        if (fire2 == 0 && bullets[i].x >= a && bullets[i].x <= a + 44 &&
            bullets[i].y >= b && bullets[i].y <= b + 88) {
          fire2 = FIRE_TIME;
          bullets[i].active = 0;
          score += 10;
        }

        if (fire3 == 0 && bullets[i].x >= c && bullets[i].x <= c + 44 &&
            bullets[i].y >= d && bullets[i].y <= d + 88) {
          fire3 = FIRE_TIME;
          bullets[i].active = 0;
          score += 10;
        }

        if (zombieAlive && bullets[i].x >= zx && bullets[i].x <= zx + 50 &&
            bullets[i].y >= zy && bullets[i].y <= zy + 90) {
          fireZ = FIRE_TIME;
          bullets[i].active = 0;
          zombieAlive = 0;

          bloodX = (int)zx;
          bloodY = (int)zy;
          bloodTimer = 25;

          score += 20;
        }

        if (bullets[i].y > 650)
          bullets[i].active = 0;
      }
    }

    // blood effect
    if (bloodTimer > 0) {
      iSetColor(200, 0, 0);
      iFilledCircle(bloodX + 20, bloodY + 20, 12);
      bloodTimer--;
    }

    // fire countdown
    if (fire1 > 0) {
      fire1--;
      if (fire1 == 0) {
        y = 601;
        x = getFreeLane((int)a, (int)c, zombieAlive ? (int)zx : -1);
      }
    }

    if (fire2 > 0) {
      fire2--;
      if (fire2 == 0) {
        b = 801;
        a = getFreeLane((int)x, (int)c, zombieAlive ? (int)zx : -1);
      }
    }

    if (fire3 > 0) {
      fire3--;
      if (fire3 == 0) {
        d = 1001;
        c = getFreeLane((int)x, (int)a, zombieAlive ? (int)zx : -1);
      }
    }

    if (fireZ > 0) {
      fireZ--;
      if (fireZ == 0) {
        zy = 1201;
        zx = getFreeLane((int)x, (int)a, (int)c);
        zombieAlive = 1;
      }
    }

    collision();
  }

  // Draw collision effects
  drawCollisionEffects();

  // Draw sad emoji effects
  drawSadEmojis();

  drawHUD(life, score);
  if (isPaused)
    drawPauseMenu();
}

// ======================= MEDIUM GAME ======================= //
void drawMediumPage(void) {
  if (gameOver) {
    drawGameOverScreen(score);
    return;
  }

  static float s1 = 3.2f, s2 = 3.8f, s3 = 4.2f, sz = 3.6f;
  static float innocentSpeed = 3.5f; // Speed for innocents
  float nitroBoost = 0;

  if (nitroOn) {
    nitroBoost = 3.0f;
    nitroTimer--;
    if (nitroTimer <= 0)
      nitroOn = 0;
  }

  iShowBMP2(0, 0, "image\\road.bmp", 0);
  renderring();

  // Draw enemies
  if (fire1 == 0)
    iShowBMP2(x, y, "image\\v1.bmp", 0);
  else
    drawFireEffect(x, y, 44, 88, fire1);
  if (fire2 == 0)
    iShowBMP2(a, b, "image\\v2.bmp", 0);
  else
    drawFireEffect(a, b, 44, 88, fire2);
  if (fire3 == 0)
    iShowBMP2(c, d, "image\\v3.bmp", 0);
  else
    drawFireEffect(c, d, 44, 88, fire3);
  if (zombieAlive)
    iShowBMP2(zx, zy, "image\\zombie.bmp", 0);
  else if (fireZ > 0)
    drawFireEffect(zx, zy, 50, 90, fireZ);

  // Draw innocent people
  for (int i = 0; i < MAX_INNOCENTS; i++) {
    if (innocents[i].active) {
      if (innocentFire[i] == 0) {
        iShowBMP2(innocents[i].x, innocents[i].y, "image\\innocent.bmp", 0);
      } else {
        // Fade out effect on innocent
        iSetColor(255, 0, 0);
        iFilledCircle(innocents[i].x + 20, innocents[i].y + 30, 15);
        if (!isPaused)
          innocentFire[i]--;
        if (innocentFire[i] == 0) {
          spawnInnocent(i);
        }
      }
    }
  }

  // Draw player (black car during nitro, normal otherwise)
  if (nitroOn) {
    drawBlackCar(playerX, playerY);
    drawRocketEffect(playerX, playerY);
  } else {
    iShowBMP2(playerX, playerY, "image\\carhero.bmp", 0);
  }

  // Enhanced bullet drawing for medium
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      drawBullet(bullets[i].x, bullets[i].y);
    }
  }

  // Move enemies
  if (!isPaused) {
    if (fire1 == 0) {
      y -= (s1 + nitroBoost);
      if (y <= -100) {
        y = 601;
        x = getFreeLane((int)a, (int)c, zombieAlive ? (int)zx : -1);
      }
    }
    if (fire2 == 0) {
      b -= (s2 + nitroBoost);
      if (b <= -100) {
        b = 801;
        a = getFreeLane((int)x, (int)c, zombieAlive ? (int)zx : -1);
      }
    }
    if (fire3 == 0) {
      d -= (s3 + nitroBoost);
      if (d <= -100) {
        d = 1001;
        c = getFreeLane((int)x, (int)a, zombieAlive ? (int)zx : -1);
      }
    }

    if (zombieAlive) {
      zy -= (sz + nitroBoost);
      if (zy <= -100) {
        zy = 1201;
        zx = getFreeLane((int)x, (int)a, (int)c);
      }
    }

    // Move innocent people
    for (int i = 0; i < MAX_INNOCENTS; i++) {
      if (innocents[i].active) {
        innocents[i].y -=
            (innocentSpeed +
             nitroBoost * 0.5f); // Innocents move slower with nitro

        // Respawn if off screen
        if (innocents[i].y <= -100) {
          spawnInnocent(i);
        }
      }
    }

    // Bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
      if (bullets[i].active) {
        bullets[i].y += bulletSpeed;

        iSetColor(255, 255, 0);
        iFilledRectangle(bullets[i].x, bullets[i].y, 4, 10);

        // Check collision with enemies
        if (fire1 == 0 && bullets[i].x >= x && bullets[i].x <= x + 44 &&
            bullets[i].y >= y && bullets[i].y <= y + 88) {
          fire1 = FIRE_TIME;
          bullets[i].active = 0;
          score += 10;
        }

        if (fire2 == 0 && bullets[i].x >= a && bullets[i].x <= a + 44 &&
            bullets[i].y >= b && bullets[i].y <= b + 88) {
          fire2 = FIRE_TIME;
          bullets[i].active = 0;
          score += 10;
        }

        if (fire3 == 0 && bullets[i].x >= c && bullets[i].x <= c + 44 &&
            bullets[i].y >= d && bullets[i].y <= d + 88) {
          fire3 = FIRE_TIME;
          bullets[i].active = 0;
          score += 10;
        }

        if (zombieAlive && bullets[i].x >= zx && bullets[i].x <= zx + 50 &&
            bullets[i].y >= zy && bullets[i].y <= zy + 90) {
          fireZ = FIRE_TIME;
          bullets[i].active = 0;
          zombieAlive = 0;
          bloodX = (int)zx;
          bloodY = (int)zy;
          bloodTimer = 25;
          score += 20;
        }

        // Check collision with innocent people
        checkInnocentHit(i);

        if (bullets[i].y > 650)
          bullets[i].active = 0;
      }
    }

    // Blood effect
    if (bloodTimer > 0) {
      iSetColor(200, 0, 0);
      iFilledCircle(bloodX + 20, bloodY + 20, 12);
      bloodTimer--;
    }

    // Fire countdown
    if (fire1 > 0) {
      fire1--;
      if (fire1 == 0) {
        y = 601;
        x = getFreeLane((int)a, (int)c, zombieAlive ? (int)zx : -1);
      }
    }
    if (fire2 > 0) {
      fire2--;
      if (fire2 == 0) {
        b = 801;
        a = getFreeLane((int)x, (int)c, zombieAlive ? (int)zx : -1);
      }
    }
    if (fire3 > 0) {
      fire3--;
      if (fire3 == 0) {
        d = 1001;
        c = getFreeLane((int)x, (int)a, zombieAlive ? (int)zx : -1);
      }
    }

    if (fireZ > 0) {
      fireZ--;
      if (fireZ == 0) {
        zy = 1201;
        zx = getFreeLane((int)x, (int)a, (int)c);
        zombieAlive = 1;
      }
    }

    collision();
  }

  // Draw collision effects
  drawCollisionEffects();

  // Draw sad emoji effects
  drawSadEmojis();

  drawHUD(life, score);
  drawNitroHUD();

  // Add warning text on dark background
  iSetColor(0, 0, 0);
  iFilledRectangle(5, 535, 300, 18);
  iSetColor(255, 80, 80);
  iText(10, 538, "Don't kill innocent people! (-20 points)",
        GLUT_BITMAP_HELVETICA_12);
  if (isPaused)
    drawPauseMenu();
}

// ======================= HARD GAME ======================= //
void drawHardPage(void) {
  if (gameOver) {
    drawGameOverScreen(score);
    return;
  }

  // ---------- HARD SPEED (much faster than medium) ----------
  static float s1 = 5.5f, s2 = 6.2f, s3 = 6.8f, sz = 5.8f;
  static float innocentSpeed = 5.5f;
  float nitroBoost = 0;

  if (nitroOn) {
    nitroBoost = 4.0f;
    nitroTimer--;
    if (nitroTimer <= 0)
      nitroOn = 0;
  }

  iShowBMP2(0, 0, "image\\road.bmp", 0);
  renderring();

  // Draw enemies with fire effects
  if (fire1 == 0)
    iShowBMP2(x, y, "image\\v1.bmp", 0);
  else
    drawFireEffect(x, y, 44, 88, fire1);
  if (fire2 == 0)
    iShowBMP2(a, b, "image\\v2.bmp", 0);
  else
    drawFireEffect(a, b, 44, 88, fire2);
  if (fire3 == 0)
    iShowBMP2(c, d, "image\\v3.bmp", 0);
  else
    drawFireEffect(c, d, 44, 88, fire3);
  if (zombieAlive)
    iShowBMP2(zx, zy, "image\\zombie.bmp", 0);
  else if (fireZ > 0)
    drawFireEffect(zx, zy, 50, 90, fireZ);

  // Draw innocent people
  for (int i = 0; i < MAX_INNOCENTS; i++) {
    if (innocents[i].active) {
      if (innocentFire[i] == 0) {
        iShowBMP2(innocents[i].x, innocents[i].y, "image\\innocent.bmp", 0);
      } else {
        iSetColor(255, 0, 0);
        iFilledCircle(innocents[i].x + 20, innocents[i].y + 30, 15);
        if (!isPaused)
          innocentFire[i]--;
        if (innocentFire[i] == 0) {
          spawnInnocent(i);
        }
      }
    }
  }

  // Draw player (black car during nitro, normal otherwise)
  if (nitroOn) {
    drawBlackCar(playerX, playerY);
    drawRocketEffect(playerX, playerY);
  } else {
    iShowBMP2(playerX, playerY, "image\\carhero.bmp", 0);
  }

  // Enhanced bullet drawing for hard
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      drawBullet(bullets[i].x, bullets[i].y);
    }
  }

  // Move enemies (fast)
  if (!isPaused) {
    if (fire1 == 0) {
      y -= (s1 + nitroBoost);
      if (y <= -100) {
        y = 601;
        x = getFreeLane((int)a, (int)c, zombieAlive ? (int)zx : -1);
      }
    }
    if (fire2 == 0) {
      b -= (s2 + nitroBoost);
      if (b <= -100) {
        b = 801;
        a = getFreeLane((int)x, (int)c, zombieAlive ? (int)zx : -1);
      }
    }
    if (fire3 == 0) {
      d -= (s3 + nitroBoost);
      if (d <= -100) {
        d = 1001;
        c = getFreeLane((int)x, (int)a, zombieAlive ? (int)zx : -1);
      }
    }

    if (zombieAlive) {
      zy -= (sz + nitroBoost);
      if (zy <= -100) {
        zy = 1201;
        zx = getFreeLane((int)x, (int)a, (int)c);
      }
    }

    // Move innocent people
    for (int i = 0; i < MAX_INNOCENTS; i++) {
      if (innocents[i].active) {
        innocents[i].y -= (innocentSpeed + nitroBoost * 0.5f);
        if (innocents[i].y <= -100) {
          spawnInnocent(i);
        }
      }
    }

    // Bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
      if (bullets[i].active) {
        bullets[i].y += bulletSpeed;

        iSetColor(255, 255, 0);
        iFilledRectangle(bullets[i].x, bullets[i].y, 4, 10);

        // Check collision with enemies
        if (fire1 == 0 && bullets[i].x >= x && bullets[i].x <= x + 44 &&
            bullets[i].y >= y && bullets[i].y <= y + 88) {
          fire1 = FIRE_TIME;
          bullets[i].active = 0;
          score += 10;
        }

        if (fire2 == 0 && bullets[i].x >= a && bullets[i].x <= a + 44 &&
            bullets[i].y >= b && bullets[i].y <= b + 88) {
          fire2 = FIRE_TIME;
          bullets[i].active = 0;
          score += 10;
        }

        if (fire3 == 0 && bullets[i].x >= c && bullets[i].x <= c + 44 &&
            bullets[i].y >= d && bullets[i].y <= d + 88) {
          fire3 = FIRE_TIME;
          bullets[i].active = 0;
          score += 10;
        }

        if (zombieAlive && bullets[i].x >= zx && bullets[i].x <= zx + 50 &&
            bullets[i].y >= zy && bullets[i].y <= zy + 90) {
          fireZ = FIRE_TIME;
          bullets[i].active = 0;
          zombieAlive = 0;
          bloodX = (int)zx;
          bloodY = (int)zy;
          bloodTimer = 25;
          score += 20;
        }

        // Check collision with innocent people
        checkInnocentHit(i);

        if (bullets[i].y > 650)
          bullets[i].active = 0;
      }
    }

    // Blood effect
    if (bloodTimer > 0) {
      iSetColor(200, 0, 0);
      iFilledCircle(bloodX + 20, bloodY + 20, 12);
      bloodTimer--;
    }

    // Fire countdown
    if (fire1 > 0) {
      fire1--;
      if (fire1 == 0) {
        y = 601;
        x = getFreeLane((int)a, (int)c, zombieAlive ? (int)zx : -1);
      }
    }
    if (fire2 > 0) {
      fire2--;
      if (fire2 == 0) {
        b = 801;
        a = getFreeLane((int)x, (int)c, zombieAlive ? (int)zx : -1);
      }
    }
    if (fire3 > 0) {
      fire3--;
      if (fire3 == 0) {
        d = 1001;
        c = getFreeLane((int)x, (int)a, zombieAlive ? (int)zx : -1);
      }
    }

    if (fireZ > 0) {
      fireZ--;
      if (fireZ == 0) {
        zy = 1201;
        zx = getFreeLane((int)x, (int)a, (int)c);
        zombieAlive = 1;
      }
    }

    collision();
  }

  // Draw collision effects
  drawCollisionEffects();

  // Draw sad emoji effects
  drawSadEmojis();

  drawHUD(life, score);
  drawNitroHUD();

  // Add warning text on dark background
  iSetColor(0, 0, 0);
  iFilledRectangle(5, 535, 320, 18);
  iSetColor(255, 80, 80);
  iText(10, 538, "HARD MODE! Don't kill innocent people! (-20 pts)",
        GLUT_BITMAP_HELVETICA_12);
  if (isPaused)
    drawPauseMenu();
}

// ======================= Button Handlers ======================= //
void startButtonClickHandler(void) {
  playClickSound();
  homePage = 0;
  startPage = 1;
}
void aboutButtonClickHandler(void) {
  playClickSound();
  homePage = 0;
  aboutPage = 1;
}
void instructionButtonClickHandler(void) {
  playClickSound();
  homePage = 0;
  instructionPage = 1;
}
void scoreButtonClickHandler(void) {
  playClickSound();
  homePage = 0;
  scorePage = 1;
  loadScoreRecords();
  scoreScrollOffset = 0;
}

void backButtonClickHandler(void) {
  playClickSound();
  gameOver = 0;
  life = 3;
  score = 0;
  homePage = 1;
  startPage = aboutPage = instructionPage = scorePage = nameEntryPage = 0;
  easyPage = mediumPage = hardPage = 0;

  // Reset innocents
  for (int i = 0; i < MAX_INNOCENTS; i++) {
    innocents[i].active = 0;
    innocentFire[i] = 0;
  }

  // Reset collision effects
  for (int i = 0; i < MAX_COLLISION_FX; i++) {
    collisionEffects[i].active = 0;
  }

  // Reset sad emoji effects
  for (int i = 0; i < MAX_SAD_FX; i++) {
    sadEffects[i].active = 0;
  }
}

void easyButtonClickHandler(void) {
  homePage = startPage = aboutPage = instructionPage = scorePage = 0;
  easyPage = 1;
}
void mediumButtonClickHandler(void) {
  homePage = startPage = aboutPage = instructionPage = scorePage = easyPage = 0;
  mediumPage = 1;
  initInnocents(); // Initialize innocents when medium page starts
}
void hardButtonClickHandler(void) {
  homePage = startPage = aboutPage = instructionPage = scorePage = easyPage =
      mediumPage = 0;
  hardPage = 1;
  initInnocents(); // Initialize innocents when hard page starts
}

// ======================= Main ======================= //
int main(void) {
  srand((unsigned)time(NULL));
  loadScoreRecords();

  if (musicOn)
    PlaySound("gamemusic.wav", NULL, SND_LOOP | SND_ASYNC);

  initializeImagePosition();
  iSetTimer(10, moveBackground);
  iSetTimer(10, updatePlayerMovement); // Smooth movement timer

  iInitialize(1000, 600, "Nitro Nexus");
  iStart();

  return 0;
}