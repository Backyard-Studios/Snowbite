import os
import subprocess
import helpers

def GetPremakeGenerator():
	if helpers.IsWindows():
		return 'vs2022'
	elif helpers.IsLinux():
		return 'gmake2'
	elif helpers.IsMac():
		return 'xcode4'

if __name__ == '__main__':
	print('⚙️ Generating project files...')
	premake_executable = f'{helpers.premake_directory}/Bin/{helpers.GetPlatform()}/{helpers.GetExecutableExtension("premake5")}'
	premake_file = f'{helpers.development_directory}/Config/premake5.lua'
	if not os.path.exists(premake_executable):
		print('❌ Premake executable not found!')
		exit(1)
	result = subprocess.run([premake_executable, f'--file={premake_file}', GetPremakeGenerator()], shell=True, capture_output=True, check=False, text=True)
	if result.returncode != 0:
		print('❌ Failed to generate project files! See output below:')
		print(result.stdout)
		exit(1)
	print('✅ Project files generated')