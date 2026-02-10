# Build script for winhttp proxy DLL
Write-Host "`n=== Building winhttp Proxy DLL ===" -ForegroundColor Cyan

# Game directory
$GamePath = "C:\Program Files (x86)\Steam\steamapps\common\MIO"

# Clean old artifacts
Write-Host "`nCleaning old build artifacts..." -ForegroundColor Yellow
Remove-Item -ErrorAction SilentlyContinue *.obj, *.exp, *.pdb, dllmain.dll, winhttp.dll, dllmain.lib

# Copy system DLL if it doesn't exist in build folder
if (-not (Test-Path ".\winhttp_orig.dll")) {
    Write-Host "Copying system winhttp.dll to winhttp_orig.dll..." -ForegroundColor Yellow
    Copy-Item "$env:windir\System32\winhttp.dll" ".\winhttp_orig.dll"
    Write-Host "System DLL copied successfully" -ForegroundColor Green
} else {
    Write-Host "winhttp_orig.dll already exists, skipping copy" -ForegroundColor Gray
}

# Build the DLL with explicit import library name
Write-Host "`nBuilding proxy DLL..." -ForegroundColor Yellow
cl.exe /LD /MD /D MODDING_API_EXPORTS dllmain.cpp modding_api.cpp /link /OUT:winhttp.dll /IMPLIB:winhttp.lib

# Check if build was successful
if ($LASTEXITCODE -eq 0) {
    Write-Host "`nBuild successful!" -ForegroundColor Green
    
    # Clean up build artifacts (but keep winhttp.lib for mod building)
    Write-Host "Cleaning up build artifacts..." -ForegroundColor Yellow
    Remove-Item -ErrorAction SilentlyContinue *.obj, *.exp
    
    Write-Host "`n=== Build Complete ===" -ForegroundColor Green
    Write-Host "Note: winhttp.lib kept for building mods" -ForegroundColor Gray
    
    # Deploy to game directory
    if (Test-Path $GamePath) {
        Write-Host "`nDeploying to game directory..." -ForegroundColor Yellow
        
        try {
            Copy-Item "winhttp.dll" $GamePath -Force
            Copy-Item "winhttp_orig.dll" $GamePath -Force
            
            # Create mods directory in game folder
            $ModsPath = Join-Path $GamePath "mods"
            if (-not (Test-Path $ModsPath)) {
                New-Item -ItemType Directory -Path $ModsPath | Out-Null
                Write-Host "Created mods directory" -ForegroundColor Green
            }
            
            Write-Host "Deployment successful!" -ForegroundColor Green
            Write-Host "Files copied to: $GamePath" -ForegroundColor Cyan
        } catch {
            Write-Host "Failed to copy files to game directory!" -ForegroundColor Red
            Write-Host "Error: $_" -ForegroundColor Red
            Write-Host "`nYou may need to run PowerShell as Administrator." -ForegroundColor Yellow
        }
    } else {
        Write-Host "`nWarning: Game directory not found!" -ForegroundColor Yellow
        Write-Host "Expected: $GamePath" -ForegroundColor Gray
        Write-Host "`nFiles are ready in the build directory:" -ForegroundColor Cyan
        Write-Host "  - winhttp.dll" -ForegroundColor White
        Write-Host "  - winhttp_orig.dll" -ForegroundColor White
    }
} else {
    Write-Host "`nBuild failed!" -ForegroundColor Red
    exit 1
}