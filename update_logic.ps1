$path = "c:\Users\shibl\Desktop\Nitro Nexus\Nitro Nexus\iMain.cpp"
$content = Get-Content $path -Raw

# 1. Add innocentKilled global variable right after life and score
$content = $content -replace "int life = 3, score = 0;(\r?\n)", "int life = 3, score = 0, innocentKilled = 0;`$1"

# 2. Update checkInnocentHit
$oldHit = '\{\s*// Penalty for killing innocent\s*score -= 20;\s*innocentFire\[i\] = FIRE_TIME; // Show hit effect\s*bullets\[bulletIndex\]\.active = 0;\s*// Spawn sad emoji effect\s*spawnSadEmoji\(innocents\[i\]\.x, innocents\[i\]\.y\);\s*// Respawn innocent after short delay\s*innocents\[i\]\.active = 0;\s*// Ensure score doesn''t go negative \(optional\)\s*if \(score < 0\) score = 0;\s*break;\s*\}'

$newHit = '{
			// Penalty for killing innocent
			score -= 20;
			innocentKilled++;
			if (innocentKilled >= 2) {
				life--;
				innocentKilled = 0;
				if (life <= 0) {
					gameOver = 1; easyPage = mediumPage = hardPage = 0; scorePage = 1;
				}
			}
			innocentFire[i] = FIRE_TIME; // Show hit effect
			bullets[bulletIndex].active = 0;
			// Spawn sad emoji effect
			spawnSadEmoji(innocents[i].x, innocents[i].y);
			// Respawn innocent after short delay
			innocents[i].active = 0;
			// Ensure score doesn''t go negative (optional)
			if (score < 0) score = 0;
			break;
		}'
$content = $content -replace $oldHit, $newHit

# Update innocent hit coordinates
$content = $content -replace ": \+ 40", " + 50" # It was + 40
$content = $content -replace "<= innocents\[i\]\.x \+ 40 &&", "<= innocents[i].x + 50 &&"
$content = $content -replace "<= innocents\[i\]\.y \+ 60\) \{", "<= innocents[i].y + 90) {"

# 3. Update zombie collision with player y hitting bounds (around line 670)
$oldZombiePlayerHit = '(\&\&\s*\(playerY \+ 88 >= zy \&\& playerY <= zy \+ )50(\)\)\s*\{)'
$content = $content -replace $oldZombiePlayerHit, '`${1}90`${2}'

# 4. Update zombie collision with bullet y hitting bounds
$oldZombieBulletHit = '(\&\&\s*bullets\[i\]\.y >= zy \&\& bullets\[i\]\.y <= zy \+ )50(\))'
$content = $content -replace $oldZombieBulletHit, '`${1}90`${2}'

# 5. Update drawFireEffect size for zombie
$content = $content -replace 'drawFireEffect\(zx, zy, 50, 50, fireZ\);', 'drawFireEffect(zx, zy, 50, 90, fireZ);'

[IO.File]::WriteAllText($path, $content)
Write-Host "Updated gameplay logic and hitboxes."
