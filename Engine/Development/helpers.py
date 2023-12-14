import os
import platform

workspace_directory = os.getcwd()
project_directory = f'{workspace_directory}/Engine'
development_directory = f'{project_directory}/Development'
premake_directory = f'{development_directory}/Premake'

def IsWindows():
	return platform.system() == 'Windows'

def IsLinux():
	return platform.system() == 'Linux'

def IsMac():
	return platform.system() == 'Darwin'

def GetPlatform():
	if IsWindows():
		return 'Windows'
	elif IsLinux():
		return 'Linux'
	elif IsMac():
		return 'MacOS'
	else:
		return 'Unknown'

def GetExecutableExtension(executable):
	if IsWindows():
		return f'{executable}.exe'
	else:
		return executable