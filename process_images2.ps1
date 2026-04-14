Add-Type -AssemblyName System.Drawing

$zFile = "c:\Users\shibl\Desktop\Nitro Nexus\Nitro Nexus\image\zombie.bmp"
$iFile = "c:\Users\shibl\Desktop\Nitro Nexus\Nitro Nexus\image\innocent.bmp"

# 1. Process Zombie (Remove background)
$imgZ = [System.Drawing.Image]::FromFile($zFile)
$w = $imgZ.Width
$h = $imgZ.Height
$bmpZ = New-Object Drawing.Bitmap($w, $h, [System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
$gZ = [System.Drawing.Graphics]::FromImage($bmpZ)
$gZ.DrawImage($imgZ, 0, 0, $w, $h)
$gZ.Dispose()
$imgZ.Dispose()

for ($y = 0; $y -lt $h; $y++) {
    for ($x = 0; $x -lt $w; $x++) {
        $c = $bmpZ.GetPixel($x, $y)
        if ($c.R -gt 230 -and $c.G -gt 230 -and $c.B -gt 230) {
            $bmpZ.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(0, 0, 0))
        }
    }
}
$bmpZ.Save($zFile, [System.Drawing.Imaging.ImageFormat]::Bmp)
$bmpZ.Dispose()
Write-Host "Zombie background removed."

# 2. Process Innocent (Resize to 50x90 and Remove background)
$imgI = [System.Drawing.Image]::FromFile($iFile)
$bmpI = New-Object Drawing.Bitmap($w, $h, [System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
$gI = [System.Drawing.Graphics]::FromImage($bmpI)
$gI.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
$gI.Clear([System.Drawing.Color]::White)
$gI.DrawImage($imgI, 0, 0, $w, $h)
$gI.Dispose()
$imgI.Dispose()

for ($y = 0; $y -lt $h; $y++) {
    for ($x = 0; $x -lt $w; $x++) {
        $c = $bmpI.GetPixel($x, $y)
        if ($c.R -gt 230 -and $c.G -gt 230 -and $c.B -gt 230) {
            $bmpI.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(0, 0, 0))
        }
    }
}
$bmpI.Save($iFile, [System.Drawing.Imaging.ImageFormat]::Bmp)
$bmpI.Dispose()
Write-Host "Innocent resized to $w x $h and background removed."
