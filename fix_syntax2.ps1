$path = "c:\Users\shibl\Desktop\Nitro Nexus\Nitro Nexus\iMain.cpp"
$content = [IO.File]::ReadAllText($path)

$content = $content.Replace(' `&&', ' &&')
$content = $content.Replace('90`))', '90))')
$content = $content.Replace('90`)', '90)')

[IO.File]::WriteAllText($path, $content)
Write-Host "Fixed syntax really!"
