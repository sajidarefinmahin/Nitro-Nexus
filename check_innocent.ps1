Add-Type -AssemblyName System.Drawing
$img = [System.Drawing.Image]::FromFile("c:\Users\shibl\Desktop\Nitro Nexus\Nitro Nexus\image\innocent.bmp")
Write-Host "Innocent: $($img.Width)x$($img.Height)"
$img.Dispose()
