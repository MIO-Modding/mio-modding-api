# Build script for example mods
param(
    [Parameter(Mandatory=$false)]
    [string]$ModName = "position_monitor"
)

Write-Host "`n=== Building Mod: $ModName ===" -ForegroundColor Cyan

# Check if winhttp.lib exists
if (-not (Test-Path "winhttp.lib")) {
    Write-Host "`nError: winhttp.lib not found!" -ForegroundColor Red
    Write-Host "You need to build the main API first with ./build.ps1" -ForegroundColor Yellow
    exit 1
}

# Clean old artifacts
Write-Host "`nCleaning old artifacts..." -ForegroundColor Yellow
Remove-Item -ErrorAction SilentlyContinue "$ModName.obj", "$ModName.lib", "$ModName.exp", "$ModName.pdb"

# Build the mod DLL
Write-Host "`nBuilding mod..." -ForegroundColor Yellow
cl.exe /LD /MD "$ModName.cpp" /link winhttp.lib /OUT:"output_mods/$ModName.dll"

# Check if build was successful
if ($LASTEXITCODE -eq 0) {
    Write-Host "`nBuild successful!" -ForegroundColor Green
    
    # Clean up artifacts
    Write-Host "Cleaning up artifacts..." -ForegroundColor Yellow
    Remove-Item -ErrorAction SilentlyContinue "$ModName.obj", "$ModName.exp", "$ModName.lib"
    
    Write-Host "`n=== Build Complete ===" -ForegroundColor Green
    Write-Host "Mod file: $ModName.dll" -ForegroundColor Cyan
    Write-Host "`nPlace this in the game's 'mods' folder" -ForegroundColor Yellow
} else {
    Write-Host "`nBuild failed!" -ForegroundColor Red
    exit 1
}