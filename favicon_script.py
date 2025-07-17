Import("env", "projenv")

print(env)
print(projenv)

import fileinput
from shutil import copy
from os import listdir
from os.path import isfile, join, isdir
from random import random

env_folder = "./env_data_folders/"
def_src = env_folder + "default/"
env_src = env_folder + env['PIOENV'] + "/"

dest = "./data/images/"

## files (names) in the default folder are expected in the env folder
files_to_copy = [f for f in listdir(def_src) if isfile(join(def_src, f))]

print("\nRUNNING SCRIPT\n")

def before_buildfs(source, target, env):
    print("Copy files before the building littlefs:")

    print("\tFiles to copy:")
    for fi in files_to_copy :
        print("\t\t" + fi)

    if(isdir(env_src)) :
        print("\nFound " + env['PIOENV'] + " folder. Will use favicons within.\n")
        path = env_src
    else :
        print("\nNo " + env['PIOENV'] + " folder. Will use default icons from " + def_src + "\n")
        path = def_src

    led_name = env.GetProjectOption("led_name")
    ## files not in env specific folder will be copied from default
    for f in files_to_copy:
        src = join(path , f)
        dst = join(dest , f)
        if(isfile(src)):
            print("\t\tCOPY favicon: " + src + "\tto " + dst)
            copy(src, dst)
        else: 
            print("\t\tCOPY from default: " + join(def_src, f) + "\tto " + dst)
            copy(join(def_src, f) , dst)


    fav = join(path , "favicon.ico")
    if(isfile(fav)):
        print("\t\tCOPY: " + fav + "\tto " + "./data/favicon.ico")
        copy(fav, "./data/favicon.ico")

    print("\n\tPreparing index.htm")

    print("\t\tLED Name: " + led_name)
    if(led_name == ""):
        led_name = "LED Control"

    if(isfile(env_folder + "index.htm")):
        copy(env_folder + "index.htm", "./data/index.htm")

    if(isfile(env_folder + "browserconfig.xml")):
        copy(env_folder + "browserconfig.xml", "./data/images/browserconfig.xml")

    if(isfile(env_folder + "site.webmanifest")):
        copy(env_folder + "site.webmanifest", "./data/images/site.webmanifest")

    with fileinput.FileInput("./data/index.htm", inplace=True) as file:
        for line in file:
            print(line.replace("%TITLE%", led_name), end='')

    with fileinput.FileInput("./data/index.htm", inplace=True) as file:
        for line in file:
            print(line.replace("%RANDOM%", str(random())), end='')

    with fileinput.FileInput("./data/images/site.webmanifest", inplace=True) as file:
        for line in file:
            print(line.replace("%TITLE%", led_name), end='')


    print("\nFINISHED Script before building littlefs\n\n")

def before_build(source, target, env):
    print("Preparing Build")
    led_name = env.GetProjectOption("led_name")
    config = env.GetProjectConfig()
    build_version = config.get("common_env_data", "build_version")    


    ## additional defines
    import urllib
    import subprocess
    
    if(led_name == ""):
        led_name = "LED Control"
    
    print("\t\tLED Name: " + led_name)

    led_name_url = str(led_name)
    led_name_url = led_name_url.replace(" ","_")

    led_name_url = urllib.parse.quote(led_name_url)

    print("\t\tLED URL: " + led_name_url)
    print("\n\tSetting CPPDEFINES:")
    print('\t\tLED_NAME:      '+led_name_url)
    print('\t\tBUILD_VERSION: '+build_version)

    d_version = str(subprocess.check_output(["git", "describe"]).strip())
    d_version = d_version.replace("b'", "")
    d_version = d_version.replace("'", "")
    if(build_version != d_version):
        res = subprocess.run(["git", "tag", "-a", "-f", build_version, "-m", "version "+build_version], text=True, capture_output=True)
        print("\n\t"+res.stdout)
    else:
        print("\n\tNot a new version: "+d_version)

    projenv.Append(CPPDEFINES=[("LED_NAME", "\\\""+led_name_url+"\\\"")])
    projenv.Append(CPPDEFINES=[("BUILD_VERSION", "\\\""+build_version+"\\\"")])
    
    print("\n\tAppended the CPPDEFINES:")
    for k in projenv['CPPDEFINES']:
        print("\t\t{}".format(k))

    print("\nFINISHED Script Before Build\n")

def after_build(source, target, env):
    print("\n\tBuild done, Nothing else to do now...")

    print("\nFINISHED Script After Build\n")

def after_buildfs(source, target, env):
    print("\nlittlefs.bin finished, deleting copied files...")
    from os import remove
    if(isfile("./data/images/site.webmanifest")):
        remove("./data/images/site.webmanifest")
    if(isfile("./data/index.htm")):
        remove("./data/index.htm")
    if(isfile("./data/images/browserconfig.xml")): 
        remove("./data/images/browserconfig.xml")
    if(isfile("./data/favicon.ico")): 
        remove("./data/favicon.ico")
    for f in files_to_copy:
        dst = join(dest , f)
        if(isfile(dst)):
            remove(dst)
    
    print("\nFINISHED Script After FS Build\n")

env.AddPreAction("$BUILD_DIR/littlefs.bin", before_buildfs)
env.AddPostAction("$BUILD_DIR/littlefs.bin", after_buildfs)

#env.AddPreAction("", before_build)
env.AddPostAction("buildprog", after_build)

before_build("","",env)

print("\nFINISHED Script\n\n")
