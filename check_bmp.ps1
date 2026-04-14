$bytes = [System.IO.File]::ReadAllBytes("c:\Users\shibl\Desktop\Nitro Nexus\Nitro Nexus\pic\01.bmp")
$w = [BitConverter]::ToInt32($bytes, 18)
$h = [BitConverter]::ToInt32($bytes, 22)
Write-Host "Width: $w, Height: $h"
