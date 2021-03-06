import os
import sys
from util import *

os.chdir(builddir)

#
# Source distribution
#
if label in ["ubuntu-amd64"]:
  source_command = ['make', 'package_source']
  source_result = call('Source', source_command)
  if source_result:
    log('Make returned ' + str(source_result))

#
# Binary Package
#
cpack_options = []
if label in ["fedora-amd64"]:
  cpack_options = ["-G", "RPM"]
elif label in ["ubuntu-x86", "ubuntu-amd64"]:
  cpack_options = ["-G", "DEB"]
elif label in ["windows-x86", "windows-amd64"]:
  # On Windows, a path can be a maximum of 256 characters. Due to the structure
  # of the Jenkins paths, we outrun this limit when generating a package.
  # As a workaround, we generate the package to cpack_temp_path.
  cpack_temp_path = "C:\Scratch\pkg"
  cpack_options = ["-G", "NSIS", "-B", cpack_temp_path]
elif label in ["macosx-x86", "macosx-amd64"]:
  cpack_options = ["-G", "DragNDrop"] 

cpack_command = ['cpack'] + cpack_options
cpack_result = call('CPack', cpack_command)
if cpack_result:
  log('CPack returned ' + str(cpack_result))
  sys.exit(cpack_result)

# Due to the Windows workaround, we need to copy the generated package (.exe) 
# from the temporary path, and remove the generated package.
if label in ["windows-x86", "windows-amd64"]:
  copy_files("{0}\*.exe".format(cpack_temp_path), builddir)
  shutil.rmtree(cpack_temp_path)

os.chdir(workspace)

