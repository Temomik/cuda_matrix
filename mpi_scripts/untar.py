import numpy as np
import os
import sys


def get_files(source):
    paths = []
    for root, dirs, files in os.walk(source):
        for file in sorted(files):
            if file.lower().endswith(('tar.gz')):
                paths.append(os.path.abspath(os.path.join(root, '\'' +  file + '\'')))
    return paths

out = "../mpi_out"
files = get_files(out)

for  file in files:
    os.system("cd " + out + " && tar -xf " + file)