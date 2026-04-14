Add-Type -AssemblyName System.Drawing
$zPath = "c:\Users\shibl\Desktop\Nitro Nexus\Nitro Nexus\image\zombie.bmp"
$iPath = "c:\Users\shibl\Desktop\Nitro Nexus\Nitro Nexus\image\innocent.bmp"
if (Test-Path $zPath) {
    $imgZ = [System.Drawing.Image]::FromFile($zPath)
    Write-Host "Zombie is $($imgZ.Width) x $($imgZ.Height)"
    $imgZ.Dispose()
} else { Write-Host "Zombie not found" }
if (Test-Path $iPath) {
    $imgI = [System.Drawing.Image]::FromFile($iPath)
    Write-Host "Innocent is $($imgI.Width) x $($imgI.Height)"
    $imgI.Dispose()
} else { Write-Host "Innocent not found" }
