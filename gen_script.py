import subprocess
import os

fileLabel = ["esplay-launcher", "esplay-gnuboy", "esplay-nofrendo", "esplay-smsplusgx"]
fileSize = []

buildCmd = ["cd esplay-launcher", "idf.py build", "cd ../esplay-gnuboy", "idf.py build",
            "cd ../esplay-nofrendo", "idf.py build", "cd ../esplay-smsplusgx", "idf.py build", "cd .."]

cleanCmd = ["cd esplay-launcher", "idf.py clean", "cd ../esplay-gnuboy", "idf.py clean",
            "cd ../esplay-nofrendo", "idf.py clean", "cd ../esplay-smsplusgx", "idf.py clean", "cd .."]


def run_cmd(command):
    p = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
    lines = []
    for line in iter(p.stdout.readline, b''):
        # line = line.strip().decode("GB2312")
        line = line.strip().decode("UTF-8")
        print("% ", line)
        lines.append(line)


def build():
    for bc in buildCmd:
        buildCmd[buildCmd.index(bc)] = bc + "\n"
    with open("build.sh", "w") as f:
        f.writelines(buildCmd)
    run_cmd("chmod +x build.sh")
    print("start build")
    run_cmd(". build.sh")
    print("get file size")
    for fbp in fileLabel:
        tmp = fbp + "/build/" + fbp + ".bin"
        size = os.path.getsize(tmp)
		#64k align
        if (size % (64 * 1024)) != 0:
            size += (64 * 1024) - (size % (64 * 1024))
        fileSize.append(size)
        print("file: " + tmp + "size: " + str(os.path.getsize(tmp)))
    mkfw_cmds = "./mkfw Retro-Emulation assets/tile.raw "
    for i in range(len(fileLabel)):
        mkfw_cmds += "0 " + str(16+i) + " " + str(fileSize[i]) + " " + fileLabel[i] + " " + fileLabel[i] + "/build/" \
                     + fileLabel[i] + ".bin "
    print("mkfw: " + mkfw_cmds)
    with open("build.sh", "w") as f:
        f.write(mkfw_cmds + "\n")
        f.write("rm esplay-retro-emu.fw\n")
        f.write("mv firmware.fw esplay-retro-emu.fw\n")
    run_cmd("chmod +x build.sh")
    print("start mkfw")
    run_cmd(". build.sh")


if __name__ == '__main__':
    build()
