import os
import tarfile
import zipfile
import urllib.request
import helpers
import shutil

def GetPremakeDownloadUrl(version):
	baseUrl = f'https://github.com/premake/premake-core/releases/download/v{version}/premake-{version}'
	if helpers.IsWindows():
		return f'{baseUrl}-windows.zip'
	elif helpers.IsLinux():
		return f'{baseUrl}-linux.tar.gz'
	elif helpers.IsMac():
		return f'{baseUrl}-macosx.tar.gz'

def CheckPremakeVersion(version, targetPath):
	filePath = targetPath + '/version.txt'
	if not os.path.exists(filePath):
		return False
	versionFile = open(filePath, 'r')
	downloadedVersion = versionFile.read()
	versionFile.close()
	if downloadedVersion != version:
		return False
	return True

def DownloadPremake(version, targetPath):
	downloadUrl = GetPremakeDownloadUrl(version)
	targetZip = f'{targetPath}/premake.zip'

	if not os.path.exists(targetPath):
		os.makedirs(targetPath, exist_ok=True)
	else:
		shutil.rmtree(targetPath)
		os.makedirs(targetPath, exist_ok=True)
	urllib.request.urlretrieve(downloadUrl, targetZip)
	if downloadUrl.endswith('zip'):
		with zipfile.ZipFile(targetZip, 'r') as zipFile:
			zipFile.extract('premake5.exe', targetPath)
	else:
		with tarfile.open(targetZip, 'r') as tarFile:
			tarFile.extract('./premake5', targetPath)
	os.remove(targetZip)
	filePath = targetPath + '/version.txt'
	versionFile = open(filePath, 'w')
	versionFile.write(version)
	versionFile.close()

if __name__ == '__main__':
	premake_version = '5.0.0-beta2'
	targetPath = f'{helpers.premake_directory}/Bin/{helpers.GetPlatform()}'
	print(f'⬇️ Downloading premake v{premake_version}...')
	if CheckPremakeVersion(premake_version, targetPath):
		print(f'⚠️ Premake v{premake_version} already downloaded')
		exit(0)
	DownloadPremake(premake_version, targetPath)
	print('✅ Done')