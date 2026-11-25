# PI32v2 actual-link receipt: conservative SETTINGS/PANEL codec specimen

Actual JieLi PI32v2 compiler and linker (JieLi 20260730.1 restored toolchain), r3/FPU flags as in the product tool script, freestanding, function/data section garbage collection, released flash XIP base 0x02000120. No product firmware link, device, installer writes, or inclusion in an app. The sample explicitly roots only the selector or selector+encoder; callbacks remain externally supplied, no fake board memory or OS. Shared CRC routine/table come from the real firmware header.

| Shape | -Os .text inc. const | -Oz .text inc. const | Native initialized/zero data |
|---|---:|---:|---:|
| Same nibble CRC/reference wrapper baseline | 128 B | 128 B | 0/0 |
| Select from new A/B, temp or real legacy A/B | 1,180 B | 1,212 B | 0/0 |
| Select + encode the actual candidate journal bytes | 1,424 B | 1,452 B | 0/0 |

Useful standalone integration starting point is -Os, not -Oz. Subtraction versus a separate CRC-only link gives 1,052 B for selector / 1,296 B for both. The exact full-product incremental APP cost needs a real link (already-shared CRC, compile-unit layout and existing legacy routines may change that delta). The migration writer, actual flash driver calls and installer UI are not included in native codec size.

Native static assertion: 200-byte caller-provided snapshot. Selector owns 32+224 B byte scratch on stack. Measured -Os frame instructions: vmj_select 52 B saved registers + 300 B stack (includes its 256 B scratch, call arguments, spills), load_v1 52+36 B, shared nested CRC 4+8 B. Longest in-specimen chain through that nested CRC is 452 B, barring additional external read callback consumption (the callback path may add its own stack). Encoder frame is 44 B +12 B CRC call chain; caller supplies independent 32+224 B output buffer. No global pool, static RAM or persistent audio memory. Do not use on audio IRQ; actual stack/callback/migration policies for firmware are separate.

Legacy SETTINGS bound = 4-byte format + 40-byte settings_t (real explicit compile-time source assert) + 4-byte optional part-off tail = 48 B. Real linked 73749f55 Elite and Pro old firmware symbol table panel = 32 B each (see legacy-symbols.txt). This is the firmware's panel configuration object at A5000/A6000, not hardware USRTRIM at C1000. Both current and previous maxima fit 4*16+2*(48+32)=224 bytes in one body page, plus 32-byte commit in a separate page. The former panel sectors occupy exactly 2*4096 B. No SOUND/PAT/SONG/BOOT/BREAK/sample relocation in this conservative candidate.

Python↔C host test runs actual reader/encoder at -O2 and compares every byte against the runnable offline migration codec. It covers 480 representative cut images across both model IDs, active A/B sides and missing legacy objects, plus max-size 224-byte encoding, both per-key histories and injected callback I/O failures. No claim of execution on PI32, full firmware fit, production timing, or hardware migration. The standalone link is genuine PI32, separate from host functional tests.

Commands: bash build.sh; PYTHONDONTWRITEBYTECODE=1 python test_codec.py. Output receipt, map, linked disassembly, sizes, and SHA256s under out/.
