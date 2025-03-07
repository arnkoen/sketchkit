@echo off
REM Set script to exit on error
setlocal enabledelayedexpansion

echo Building shaders...

shaderc -f model.vs -o vk/model.vs.bin --varyingdef model_varying.def --type v --platform windows --profile spirv
shaderc -f model.fs -o vk/model.fs.bin --varyingdef model_varying.def --type f --platform windows --profile spirv

shaderc -f phong.vs -o vk/phong.vs.bin --varyingdef phong_varying.def --type v --platform windows --profile spirv
shaderc -f phong.fs -o vk/phong.fs.bin --varyingdef phong_varying.def --type f --platform windows --profile spirv

if %errorlevel% neq 0 (
    echo Build failed!
    exit /b %errorlevel%
)

echo Done.
