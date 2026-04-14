Add-Type -AssemblyName System.Drawing

$width = 60
$height = 80

function Process-Image {
    param (
        [string]$inFile,
        [string]$outFile
    )
    
    $img = [System.Drawing.Image]::FromFile($inFile)
    $bmp = New-Object Drawing.Bitmap($width, $height, [System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    
    # High-quality resize
    $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    
    # White bounding background to avoid black smudges near transparent edges
    $g.Clear([System.Drawing.Color]::White)
    $g.DrawImage($img, 0, 0, $width, $height)
    $g.Dispose()
    $img.Dispose()
    
    # Replace near-white background (>230) completely with pure black (0) for iShowBMP2(..., 0) transparency
    for ($y = 0; $y -lt $height; $y++) {
        for ($x = 0; $x -lt $width; $x++) {
            $c = $bmp.GetPixel($x, $y)
            if ($c.R -gt 230 -and $c.G -gt 230 -and $c.B -gt 230) {
                # Set fully back (0,0,0)
                $bmp.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(0, 0, 0))
            } elseif ($c.R -lt 5 -and $c.G -lt 5 -and $c.B -lt 5) {
                # Fix pure blacks turning transparent by mistake! Bump them to 6
                $bmp.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(6, 6, 6))
            }
        }
    }
    
    $bmp.Save($outFile, [System.Drawing.Imaging.ImageFormat]::Bmp)
    $bmp.Dispose()
    Write-Host "Processed $outFile successfully"
}

# 1. Process new realistic zombie
$pngZombie = "C:\Users\shibl\.gemini\antigravity\brain\2cb214dd-5fe2-4e84-b833-ce5ee4995ed9\realistic_topdown_zombie_1774685221950.png"
$bmpZombie = "c:\Users\shibl\Desktop\Nitro Nexus\Nitro Nexus\image\zombie.bmp"
Process-Image -inFile $pngZombie -outFile $bmpZombie

# 2. Process new realistic innocent pedestrian
$pngInnocent = "C:\Users\shibl\.gemini\antigravity\brain\2cb214dd-5fe2-4e84-b833-ce5ee4995ed9\realistic_topdown_innocent_1774687081238.png"
$bmpInnocent = "c:\Users\shibl\Desktop\Nitro Nexus\Nitro Nexus\image\innocent.bmp"
Process-Image -inFile $pngInnocent -outFile $bmpInnocent
