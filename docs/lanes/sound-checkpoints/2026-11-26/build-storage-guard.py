#!/usr/bin/env python3
"""Gate one firmware link and place large temporaries on the workspace.

Use one pinned checkout and toolchain. Save exact source, build commands, pins,
logs and any unique evidence to Git or Library before pruning inactive copies.
Do not prune live trees or silently remove failed-run evidence. Do not run the
linker from a hand-copied repository. This guard does not delete work.

Nov 26, 2026 recovery: 2,862,755 bytes of a temporary Git duplicate restored
shell startup. Two passes rehashed and removed 17,839 exact Git archive copies;
workspace free grew from 5 MiB to 6,047 MiB and /tmp from full to 1,741 MiB.
The verified restoration source was Git commit 73749f55d17a0643f1ddd072114172d29eecc2ae.
Use the guard per native link and the preflight alone before other bulk work."""
import argparse,fcntl,os,shutil,subprocess,tempfile
from pathlib import Path

MiB=1024*1024
GAP=1024*MiB
SMALL_TMP=128*MiB

def free(p, floor):
    v=os.statvfs(p)
    n=v.f_bavail*v.f_frsize
    print(f'{p}: {n//MiB} MiB free, {v.f_favail} free entries', flush=True)
    return n>=floor and (not v.f_files or v.f_favail>=2048)

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('workspace',type=Path)
    ap.add_argument('command',nargs=argparse.REMAINDER)
    a=ap.parse_args()
    root=a.workspace.resolve(strict=True)
    command=a.command[1:] if a.command[:1]==['--'] else a.command
    okay=free(root,GAP) & free('/tmp',SMALL_TMP)
    if not okay: ap.exit(75,'Insufficient scratch: archive or reclaim verified copies, then retry.\n')
    if not command:return 0
    scratch=root/'.firmware-build-scratch'
    if scratch.is_symlink(): ap.exit(75,'Unexpected symlink at build scratch.\n')
    scratch.mkdir(mode=0o700, exist_ok=True)
    with (scratch/'native-link.lock').open('a') as lock:
        try: fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
        except BlockingIOError: ap.exit(75,'Another native firmware link owns this workspace.\n')
        if not free(root,GAP) or not free('/tmp',SMALL_TMP):return 75
        stage=tempfile.mkdtemp(prefix='native-',dir=scratch)
        env=dict(os.environ,TMPDIR=stage,TMP=stage,TEMP=stage)
        print('Explicit firmware temporary directory:',stage,flush=True)
        rc=subprocess.run(command,env=env).returncode
        # Keep leftovers for review, but don't stack runs on hidden copies.
        if not os.listdir(stage):Path(stage).rmdir()
        else:print('Unarchived leftovers:',stage,'bytes available:',shutil.disk_usage(root).free)
        return rc
if __name__=='__main__':raise SystemExit(main())
