Add-Type -AssemblyName System.Drawing

$width = 50
$height = 60

# 1. Resize innocent.bmp
$innoPath = "c:\Users\shibl\Desktop\Nitro Nexus\Nitro Nexus\image\innocent.bmp"
$imgInno = [System.Drawing.Image]::FromFile($innoPath)
$bmpInno = New-Object Drawing.Bitmap($width, $height, [System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
$gInno = [System.Drawing.Graphics]::FromImage($bmpInno)
$gInno.DrawImage($imgInno, 0, 0, $width, $height)
$gInno.Dispose()
$imgInno.Dispose()
$bmpInno.Save($innoPath, [System.Drawing.Imaging.ImageFormat]::Bmp)
$bmpInno.Dispose()

# 2. Convert and resize realistic zombie png
$pngPath = "C:\Users\shibl\.gemini\antigravity\brain\2cb214dd-5fe2-4e84-b833-ce5ee4995ed9\realistic_topdown_zombie_1774685221950.png"
$zombiePath = "c:\Users\shibl\Desktop\Nitro Nexus\Nitro Nexus\image\zombie.bmp"
$imgZom = [System.Drawing.Image]::FromFile($pngPath)
$bmpZom = New-Object Drawing.Bitmap($width, $height, [System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
$gZom = [System.Drawing.Graphics]::FromImage($bmpZom)
$gZom.DrawImage($imgZom, 0, 0, $width, $height)
$gZom.Dispose()
$imgZom.Dispose()
$bmpZom.Save($zombiePath, [System.Drawing.Imaging.ImageFormat]::Bmp)
$bmpZom.Dispose()

Write-Host "Success: Resized both images to 50x60 24-bit BMP."
