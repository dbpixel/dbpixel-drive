# Converte lateral.png e cabecalho.png em BMP de 24 bits (o formato que o NSIS aceita)
Add-Type -AssemblyName System.Drawing
foreach ($n in "lateral", "cabecalho") {
    $src = [Drawing.Image]::FromFile("$PSScriptRoot\$n.png")
    $bmp = New-Object Drawing.Bitmap $src.Width, $src.Height, ([Drawing.Imaging.PixelFormat]::Format24bppRgb)
    $g = [Drawing.Graphics]::FromImage($bmp); $g.DrawImage($src, 0, 0, $src.Width, $src.Height); $g.Dispose()
    $bmp.Save("$PSScriptRoot\$n.bmp", [Drawing.Imaging.ImageFormat]::Bmp); $bmp.Dispose(); $src.Dispose()
}
"bmp ok"
