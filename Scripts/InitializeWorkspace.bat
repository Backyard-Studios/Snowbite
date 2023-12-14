@echo off
pushd %~dp0\..\
python .\Engine\Development\download_premake.py
python .\Engine\Development\generate_project_files.py
popd