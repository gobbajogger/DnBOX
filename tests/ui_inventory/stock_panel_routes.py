#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Exercise native calibration and updater entry using the linked SMK image."""
import argparse
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import tempfile

from PIL import Image

from stock_boot_fixture import ROOT, digest, load_symbols, require

# Teach a rotated button mapping so a default or no-op save cannot pass.
BUTTONS = ("SCALE","CHORD","NOTEREPEAT","PARA","GLOBE","PATCH","BT",
           "ARP","SEQ","SEQPLAY","SEQREC","OCTAVE-","OCTAVE+","FX")
BUTTON_IDS = (8,9,7,13,10,12,11,6,15,16,17,4,5,14)
DIR = (1,-1,1,-1,1,-1,1)
EXPECTED_PANEL = struct.pack("<I14B7B7b", 0x50584E53, *BUTTON_IDS, *range(1,8), *DIR)
CAL_IDS = ["calibration-title"] + [f"hardware-calibration-button-{i:02d}" for i in range(14)]
CAL_IDS += [f"hardware-calibration-encoder-{i:02d}" for i in range(7)]
USB_MESSAGES = "1800:f07e7f0601f7;2000:f02224357df7"
LUT_5 = [(i >> 3)*255//31 for i in range(256)]
LUT_6 = [(i >> 2)*255//63 for i in range(256)]


def calibration_keys():
    events = ["0:OCTAVE-", "0:OCTAVE+", "1200:OCTAVE-^", "1200:OCTAVE+^"]
    for i, button in enumerate(BUTTONS):
        t = 1450 + 150*i
        events.extend((f"{t}:{button}", f"{t+60}:{button}^"))
    for i, direction in enumerate(DIR):
        events.append(f"{3650+525*i}:e{i+1}:{2*direction}")
    return ",".join(events)


def record(command, directory, env, wanted, log):
    # Map fm1-live's RGB565 expansion to the archive's exact RGB888 expansion.
    # Only the low bits differ. All 57,600 source pixels are compared per match.
    hits = {}
    previous = None
    count = 0
    with log.open("wb") as stderr, subprocess.Popen(command, cwd=directory, env=env,
            stdout=subprocess.PIPE, stderr=stderr) as proc:
        with proc.stdout as stream:
            while header := stream.readline():
                require(header == b"P6\n" and stream.readline() == b"240 240\n" and
                        stream.readline() == b"255\n", "invalid LCD movie header")
                pixels = stream.read(240*240*3)
                require(len(pixels) == 240*240*3, "incomplete LCD movie frame")
                if wanted and pixels != previous:
                    canonical = Image.frombytes("RGB", (240,240), pixels).point(LUT_5+LUT_6+LUT_5)
                    state = wanted.get(digest(canonical.tobytes()))
                    if state and state not in hits:
                        hits[state] = count*16
                previous = pixels
                count += 1
        require(proc.wait() == 0, "emulator exit; inspect " + log.name)
    require(count > 0, "no recorded LCD frames")
    return hits, count


def run(args):
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    emu, fwsc, elf = (x.resolve() for x in (args.emulator,args.fwsc,args.elf))
    symbols = load_symbols(elf)
    hashes = json.loads((ROOT/"docs/lanes/fwui/current/native-routes/report.json").read_text())["rgbSha256"]
    receipt = {"model":args.model, "emulatorSha256":digest(emu.read_bytes()),
               "fwscSha256":digest(fwsc.read_bytes()), "elfSha256":digest(elf.read_bytes()), "cases":{}}
    with tempfile.TemporaryDirectory() as tmp:
        folder = Path(tmp)

        def shot(name, ms, initial=None, keys="", expected=(), usb=False):
            directory = folder/name
            (directory/"work").mkdir(parents=True)
            flash = directory/"flash.bin"
            if initial is not None:
                flash.write_bytes(initial)
            ram = directory/"ram.bin"
            env = {k:v for k,v in os.environ.items() if not k.startswith("FM1_")}
            env.update(FM1_STRICT="1", FM1_BENCH_KEYS=keys, FM1_DUMPRAM=str(ram), FM1_RESET_TRACE="1")
            command = [emu,"--fwsc",fwsc,"--mhz","360","--lcd-only","--flash",flash,
                       "--shot",str(ms),"--record","-"]
            if usb:
                command.append("--usb")
                env["FM1_USB_SEND"] = USB_MESSAGES
            logfile = out/(name+".log")
            hits, count = record(command, directory, env, {hashes[k]:k for k in expected}, logfile)
            require(list(hits) == list(expected), name+" complete-frame order differs: "+str(hits))
            state = ram.read_bytes()
            panel_addr, panel_bytes = symbols["panel"]
            dbg_addr, dbg_bytes = symbols["x0x_dbg"]
            require(panel_bytes==32 and dbg_bytes==52, "unexpected firmware ABI")
            panel = state[panel_addr:panel_addr+panel_bytes]
            dbg = struct.unpack_from("<13I", state, dbg_addr)
            log = logfile.read_text()
            match = re.findall(r"^flash: (\d+) erases / programs$", log, re.M)
            require(len(match)==1, "missing flash count")
            image = flash.read_bytes()
            receipt["cases"][name] = {"shotMs":ms, "observedFrames":count,
                "firstMatchingFrameMs":hits, "panelBytes":panel.hex(),
                "flashOperations":int(match[0]), "bootCount":dbg[10], "uiFrames":dbg[7],
                "crashSeen":dbg[12], "audioHalves":dbg[1], "timerIrqs":dbg[6],
                "finalFlashSha256":digest(image), "keys":keys}
            return image, panel, dbg, log

        base, _, _, _ = shot("seed", 1)
        require(len(base)==0x100000 and receipt["cases"]["seed"]["flashOperations"]==0,
                "unexpected initial flash")
        base = bytearray(base)
        base[0xAF000:0xAF010] = b"SMK37DATAX0X"+bytes(4)
        saved, mapping, cal_dbg, _ = shot("calibration",8100,base,calibration_keys(),CAL_IDS)
        require(cal_dbg[7]>0 and cal_dbg[10]==1 and not cal_dbg[12], "calibration did not return to UI")
        require(mapping==EXPECTED_PANEL and saved[0xA5100:0xA5120]==EXPECTED_PANEL,
                "taught mapping differs in RAM or panel object")
        require(saved[:0xA5000]==base[:0xA5000] and saved[0xA6000:]==base[0xA6000:],
                "calibration touched another storage sector")
        reloaded, loaded, dbg, _ = shot("cold-reload",1500,saved)
        require(loaded==mapping and saved==reloaded and receipt["cases"]["cold-reload"]["flashOperations"]==0,
                "cold reload did not retain the panel mapping")
        require(dbg[7]>0 and dbg[10]==1 and not dbg[12], "cold reload did not reach UI")

        for name, ms, keys, state, usb in (
            ("usb-updater",2600,"","boot-uboot-usb",True),
            ("octave-updater",7100,"1500:OCTAVE-,1500:OCTAVE+","boot-uboot",False)):
            final, _, dbg, log = shot(name,ms,base,keys,[state],usb)
            require(final==base and receipt["cases"][name]["flashOperations"]==0,
                    name+" changed emulated flash")
            reset = re.search(r"chip reset at (\d+) ms \(PR_PWR\)", log)
            require(reset and dbg[10]>=2 and not dbg[12], name+" absent reset or stale boot")
            receipt["cases"][name]["resetMs"] = int(reset[1])
            if usb:
                detach = re.search(r"\[\s*(\d+) ms\] usb: Detached", log)
                require(detach and "usb: Configured" in log and "usb in:  f0 7e 7f 06 02" in log and
                        "usb out: f0 22 24 35 7d f7" in log and int(detach[1])<int(reset[1]),
                        "inquiry, updater request, detach and reset sequence")
                receipt["cases"][name].update(detachMs=int(detach[1]), usbMessages=USB_MESSAGES)
    (out/"panel-routes.json").write_text(json.dumps(receipt,indent=2)+"\n")


if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--model",choices=("elite","pro"),required=True)
    parser.add_argument("--emulator",type=Path,required=True)
    parser.add_argument("--fwsc",type=Path,required=True)
    parser.add_argument("--elf",type=Path,required=True)
    parser.add_argument("output",type=Path)
    run(parser.parse_args())
