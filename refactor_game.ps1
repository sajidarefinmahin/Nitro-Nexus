$path = "c:\Users\shibl\Desktop\Nitro Nexus\Nitro Nexus\iMain.cpp"
$content = Get-Content $path -Raw

# 1. Globals
$content = $content -replace '(?s)// ======================= Innocent People ======================= //.*?int innocentFire\[MAX_INNOCENTS\] = \{ 0 \}; // For hit effect on innocents', ''
$content = $content -replace '(?s)// ======================= Zombies ======================= //.*?int zombieAlive = 1;', '// ======================= Pedestrians (Zombie/Innocent) ======================= //
float zx = 540, zy = 1201;
int zombieAlive = 1; // 1 = Zombie, 2 = Innocent'

# 2. Delete the functions (spawnInnocent, initInnocents, checkInnocentHit)
$content = $content -replace '(?s)// ======================= Innocent People Functions ======================= //.*?void checkInnocentHit.*?break;\s*\}\s*\}\s*\}', ''

# 3. collision() update
$oldCol = '(?s)if \(zombieAlive && fireZ == 0 &&\s*\(playerX \+ 44 >= zx && playerX <= zx \+ 50\) &&\s*\(playerY \+ 88 >= zy && playerY <= zy \+ 50\)\) \{(.*?)\s*zy = 1201; zx = getFreeLane\(\(int\)x, \(int\)a, \(int\)c\);\s*\}'
$newCol = 'if (zombieAlive > 0 && fireZ == 0 && (playerX + 44 >= zx && playerX <= zx + 60) && (playerY + 88 >= zy && playerY <= zy + 80)) {
		if (zombieAlive == 1) { life--; spawnCollisionEffect((float)playerX + 22.0f, (float)playerY + 88.0f); }
		else { score -= 20; if (score < 0) score = 0; spawnSadEmoji(zx, zy); }
		fireZ = FIRE_TIME; zombieAlive = 0; zy = 1201; zx = getFreeLane((int)x, (int)a, (int)c);
	}'
$content = $content -replace $oldCol, $newCol

# 4. Remove initInnocents from main
$content = $content -replace '\s*initInnocents\(\);', ''

# 5. Remove checkInnocentHit(i);
$content = $content -replace '(?s)// Check collision with innocent people\s*checkInnocentHit\(i\);', ''

# 6. Inside drawEasyPage/Medium/Hard: replace image rendering
$oldDrawZom = '(?s)if \(zombieAlive\) iShowBMP2\(zx, zy, "image\\\\zombie\.bmp", 0\);\s*else if \(fireZ > 0\) drawFireEffect\(zx, zy, 50, 50, fireZ\);'
$newDrawZom = 'if (zombieAlive == 1) iShowBMP2(zx, zy, "image\\\\zombie.bmp", 0);
	else if (zombieAlive == 2) iShowBMP2(zx, zy, "image\\\\innocent.bmp", 0);
	else if (fireZ > 0) drawFireEffect(zx, zy, 60, 80, fireZ);'
$content = $content -replace $oldDrawZom, $newDrawZom

# 7. Remove // Draw innocent people block (in medium/hard)
$content = $content -replace '(?s)// Draw innocent people\s*for \(int i = 0; i < MAX_INNOCENTS; i\+\+\) \{.*?(?=\s*// Draw player|\s*// Enhanced bullet)', ''

# 8. Update bullet hit code
$oldBulletZom = '(?s)if \(zombieAlive &&.*?bullets\[i\]\.x >= zx && bullets\[i\]\.x <= zx \+ 50 &&\s*bullets\[i\]\.y >= zy && bullets\[i\]\.y <= zy \+ 50\)\s*\{\s*fireZ = FIRE_TIME;\s*bullets\[i\]\.active = 0;\s*zombieAlive = 0;\s*(bloodX = \(int\)zx;\s*bloodY = \(int\)zy;\s*bloodTimer = 25;\s*)?score \+= 20;\s*\}'
$newBulletZom = 'if (zombieAlive > 0 && bullets[i].x >= zx && bullets[i].x <= zx + 60 && bullets[i].y >= zy && bullets[i].y <= zy + 80) {
				fireZ = FIRE_TIME; bullets[i].active = 0;
				if (zombieAlive == 1) { bloodX = (int)zx; bloodY = (int)zy; bloodTimer = 25; score += 20; }
				else { spawnSadEmoji(zx, zy); score -= 20; if (score < 0) score = 0; }
				zombieAlive = 0;
			}'
$content = $content -replace $oldBulletZom, $newBulletZom

# 9. Update movement
$oldMoveZom = '(?s)if \(zombieAlive\)\s*\{\s*zy -= sz;\s*if \(zy <= -100\)\s*\{\s*zy = 1201;\s*zx = getFreeLane\(\(int\)x, \(int\)a, \(int\)c\);\s*\}\s*\}'
$newMoveZom = 'if (zombieAlive > 0) { zy -= sz; if (zy <= -100) { zy = 1201; zx = getFreeLane((int)x, (int)a, (int)c); zombieAlive = (rand() % 2) + 1; } }'
$content = $content -replace $oldMoveZom, $newMoveZom

$oldMoveZomMed = '(?s)if \(zombieAlive\) \{ zy -= \(sz \+ nitroBoost\); if \(zy <= -100\) \{ zy = 1201; zx = getFreeLane\(\(int\)x, \(int\)a, \(int\)c\); \} \}'
$newMoveZomMed = 'if (zombieAlive > 0) { zy -= (sz + nitroBoost); if (zy <= -100) { zy = 1201; zx = getFreeLane((int)x, (int)a, (int)c); zombieAlive = (rand() % 2) + 1; } }'
$content = $content -replace $oldMoveZomMed, $newMoveZomMed

# 10. Remove // Move innocent people block
$content = $content -replace '(?s)// Move innocent people\s*for \(int i = 0; i < MAX_INNOCENTS; i\+\+\) \{.*?(?=\s*// Bullets|\s*// ---------------- bullets)', ''

# 11. Update fireZ fade respawn
$oldFireZMed = '(?s)if \(fireZ > 0\) \{ fireZ--; if \(fireZ == 0\) \{ zy = 1201; zx = getFreeLane\(\(int\)x, \(int\)a, \(int\)c\); zombieAlive = 1; \} \}'
$newFireZMed = 'if (fireZ > 0) { fireZ--; if (fireZ == 0) { zy = 1201; zx = getFreeLane((int)x, (int)a, (int)c); zombieAlive = (rand() % 2) + 1; } }'
$content = $content -replace $oldFireZMed, $newFireZMed

$oldFireZEasy = '(?s)if \(fireZ > 0\)\s*\{\s*fireZ--;\s*if \(fireZ == 0\)\s*\{\s*zy = 1201;\s*zx = getFreeLane\(\(int\)x, \(int\)a, \(int\)c\);\s*zombieAlive = 1;\s*\}\s*\}'
$newFireZEasy = 'if (fireZ > 0) { fireZ--; if (fireZ == 0) { zy = 1201; zx = getFreeLane((int)x, (int)a, (int)c); zombieAlive = (rand() % 2) + 1; } }'
$content = $content -replace $oldFireZEasy, $newFireZEasy

# 12. Update Pause Button visual
$oldPauseButton = '(?s)// Pause button \(Top Center\).*?iText\(470, 568, "PAUSE", GLUT_BITMAP_TIMES_ROMAN_24\);'
$newPauseButton = '// Pause button (Bottom Left)
	iSetColor(50, 50, 60);
	iFilledRectangle(10, 10, 100, 35);
	iSetColor(150, 200, 255);
	iRectangle(10, 10, 100, 35);
	iFilledRectangle(25, 17, 6, 20);
	iFilledRectangle(38, 17, 6, 20);
	iSetColor(255, 255, 255);
	iText(55, 20, "PAUSE", GLUT_BITMAP_HELVETICA_18);'
$content = $content -replace $oldPauseButton, $newPauseButton

# 13. Update Pause Hitbox
$content = $content -replace 'if \(mx >= 450 && mx <= 550 && my >= 560 && my <= 590\)', 'if (mx >= 10 && mx <= 110 && my >= 10 && my <= 45)'

[IO.File]::WriteAllText($path, $content)
Write-Host "Refactor completed."
