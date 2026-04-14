$path = "c:\Users\shibl\Desktop\Nitro Nexus\Nitro Nexus\iMain.cpp"
$content = Get-Content $path -Raw

$content = $content.Replace(" `&&", " &&")
$content = $content.Replace("90`))", "90))")

[IO.File]::WriteAllText($path, $content)
Write-Host "Fixed syntax!"
