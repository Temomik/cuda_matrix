import numpy as np
import PIL
from PIL import Image
import os
import sys

def get_files(source):
    paths = []
    for root, dirs, files in os.walk(source):
        for file in sorted(files):
            paths.append(os.path.abspath(os.path.join(root, '\'' +  file+ '\'')))
    return paths
        
def delegate(arr,chunks):
    return [arr[i * len(arr) // chunks : (i+1) * len(arr) // chunks] for i in range(chunks)]

if len(sys.argv) == 2:
    chunk_count = int(sys.argv[1])
else:
    chunk_count = 6

images_path = get_files("../../images")
chunks = delegate(images_path,chunk_count)

for i in range(chunk_count):
    os.system("rm -rf " + str(i + 1))
    os.system("mkdir " + str(i + 1))
    f = open(str(i + 1) + "/filenames.txt","w+")
    for path in chunks[i]:
        command = "cp " +  path + " " + os.path.abspath(str(i + 1))
        os.system(command)
        f.write(path.split("\'")[1] + "\n");
    f.close()
    os.system("tar -zcf " + str(i + 1) + ".tar.gz " + str(i + 1) + "/")
    os.system("rm -rf " + str(i + 1))

f = open("path_to_data.txt","w+")
f.write(os.path.abspath("") + "/")