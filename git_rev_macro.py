import subprocess

revision = subprocess.check_output(["git", "rev-parse", "HEAD"]).strip()

branchcmd = "git rev-parse --abbrev-ref HEAD"

branch = subprocess.check_output(branchcmd, shell=True).decode().strip()

print("'-DPIO_SRC_REV=\"%s\"'" % revision)
print("'-DPIO_SRC_BRANCH=\"%s\"'" % branch)
