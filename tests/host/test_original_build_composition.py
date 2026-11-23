#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Check replacement/legacy source and flag selection without invoking a compiler."""
import importlib.util
import inspect
import re
import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock

ROOT = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location("x0x_build", ROOT / "tools/build.py")
build = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(build)


def compile_plan(env):
    original, wobble = build.synth_build_config(env)
    dev_wobble = env.get("X0X_DEV_WOBBLE", "1" if original else wobble)
    flags = [*build.CFLAGS, *build.FPU, "-DBOARD_SMK37=1", "-DSMK_MODEL=1",
             f"-DX0X_WOBBLE={wobble}", f"-DX0X_DEV_WOBBLE={dev_wobble}"]
    commands, objects = build.app_compile_commands(flags, original, wobble)
    sources = {}
    for command in commands:
        path = Path(command[command.index("-c") + 1])
        if path.is_absolute():
            path = path.relative_to(build.SRC)
        sources[str(path)] = command
    return commands, objects, sources


class OriginalBuildCompositionTests(unittest.TestCase):
    def test_smk_generation_and_compile_isolated_from_host_headers(self):
        class StopBeforeLink(Exception):
            pass

        for model, outname in (("elite", "smk37-elite"), ("pro", "smk37")):
            with self.subTest(model=model), tempfile.TemporaryDirectory() as tmp:
                root = Path(tmp).resolve()
                out = root / "build"
                host_gen = out / "gen"
                host_gen.mkdir(parents=True)
                host_bank = host_gen / "x0x_break_bank.h"
                host_bank.write_text("#define X0X_NBREAKS 2\n")
                native_gen = out / outname / "gen"
                captured = []

                def simulate_generation():
                    build.GEN.mkdir(parents=True, exist_ok=True)
                    (build.GEN / "x0x_break_bank.h").write_text("#define X0X_NBREAKS 0\n")

                def stop_before_link(*units):
                    captured.extend(units)
                    raise StopBeforeLink

                with mock.patch.multiple(build, SRC=root, OUT=out, GEN=host_gen,
                                         LDR=out / "loader", MODEL=None, PRODUCT=None,
                                         BOARD=None, VERSION=None, GIT_SHA=None,
                                         RELEASE_LABEL=None, BOARD_DEFS=dict(build.BOARD_DEFS),
                                         BOARD_PREFIX=dict(build.BOARD_PREFIX),
                                         APP_SLOT=build.APP_SLOT, APP_CODE_MAX=build.APP_CODE_MAX), \
                     mock.patch.dict(os.environ, {}, clear=True), \
                     mock.patch.object(sys, "argv", ["build.py", "--board", "smk37", "--model", model]), \
                     mock.patch.object(build, "require_original_synth_sources"), \
                     mock.patch.object(build.subprocess, "run"), \
                     mock.patch.object(build.fm1pkg_make, "set_board"), \
                     mock.patch.object(build.fm1pkg_make, "SDK", None), \
                     mock.patch.object(build, "check_sdk"), \
                     mock.patch.object(build, "generate", side_effect=simulate_generation), \
                     mock.patch.object(build, "build_loader"), \
                     mock.patch.object(build, "clang_std_inc", return_value=[]), \
                     mock.patch.object(build, "tc_all", side_effect=stop_before_link):
                    build.subprocess.run.return_value.returncode = 0
                    with self.assertRaises(StopBeforeLink):
                        build.main()
                self.assertEqual(host_bank.read_text(), "#define X0X_NBREAKS 2\n")
                self.assertEqual((native_gen / host_bank.name).read_text(), "#define X0X_NBREAKS 0\n")
                c_units = [cmd for cmd in captured if str(cmd[cmd.index("-c") + 1]).endswith(".c")]
                self.assertTrue(c_units)
                for cmd in c_units:
                    self.assertIn("-I" + str(native_gen.relative_to(root)), cmd)
                    self.assertNotIn("-Ibuild/gen", cmd)

    def test_synthone_editor_and_provider_use_the_same_model_default(self):
        targets = (("smk37", "elite", "0", "1"), ("smk37", "pro", "0", "1"),
                   ("smk37", "mke", "0", "0"), ("fm1", "pro", "0", "0"),
                   ("smk37", "pro", "1", "0"))
        for board, model, factory, default in targets:
            for override in (None, "0", "1"):
                env = dict(os.environ)
                env.pop("X0X_SYNTHONE_PATCH", None)
                if override is not None:
                    env["X0X_SYNTHONE_PATCH"] = override
                with self.subTest(board=board, model=model, factory=factory, override=override), \
                     mock.patch.object(build, "BOARD", board), mock.patch.object(build, "MODEL", model), \
                     mock.patch.object(build, "SUPERJ6_FACTORY", factory), mock.patch.dict(os.environ, env, clear=True):
                    _, _, sources = compile_plan({})
                    expected = "-DX0X_SYNTHONE_PATCH=" + (override or default)
                    for source, command in sources.items():
                        if source.endswith(".c"):
                            defines = [flag for flag in command if str(flag).startswith("-DX0X_SYNTHONE_PATCH=")]
                            self.assertEqual(defines, [expected], source)
        with mock.patch.dict(os.environ, {"X0X_SYNTHONE_PATCH": "invalid"}):
            with self.assertRaisesRegex(SystemExit, "X0X_SYNTHONE_PATCH: 0 or 1"):
                compile_plan({})

    def test_compact_chicago_preserves_hot_unrolling_and_other_boards(self):
        for board, model, compact in (("smk37", "elite", True),
                                      ("smk37", "pro", True),
                                      ("smk37", "mke", False),
                                      ("fm1", "pro", False)):
            with mock.patch.object(build, "BOARD", board), mock.patch.object(build, "MODEL", model):
                _, _, sources = compile_plan({"X0X_ORIGINAL_SYNTH": "1"})
            for path, command in sources.items():
                with self.subTest(board=board, model=model, source=path):
                    self.assertNotIn("-fno-unroll-loops", command)
                    native_only = Path(path).suffix == ".S" or Path(path).stem in {"chicago_dynamics", "chicago_modulation", "original_runtime"} or path in {"firmware/src/dsp/drum909.c", "firmware/src/dsp/master.c", "firmware/src/dsp/fxbus.c", "firmware/src/app/gfinal/ui_chance_exact.c"}
                    self.assertEqual("-flto" in command, compact and not native_only)
                    expected = compact and Path(path).stem == "chicago_packed"
                    self.assertEqual("-DX0X_CHICAGO_COMPACT_HEADERS=1" in command, expected)
                    if "-O2" in command:
                        self.assertIn("-ffp-contract=off", command)
                        self.assertNotIn("-ffast-math", command)

    def assert_chance_plan(self, model, setting=None):
        env = dict(os.environ)
        env.pop("X0X_GFINAL_CHANCE_PROBE", None)
        if setting is not None:
            env["X0X_GFINAL_CHANCE_PROBE"] = setting
        with mock.patch.dict(os.environ, env, clear=True), mock.patch.object(build, "BOARD", "smk37"), mock.patch.object(build, "MODEL", model):
            commands, objects, sources = compile_plan({"X0X_ORIGINAL_SYNTH": "1"})
        path = "firmware/src/app/gfinal/ui_chance_exact.c"
        enabled = setting != "0"
        self.assertEqual(path in sources, enabled, "CHANCE_PROVIDER_PRESENCE")
        self.assertEqual(len(commands), len(sources), "CHANCE_DUPLICATE_UNIT")
        self.assertEqual(len(objects), len(set(objects)), "CHANCE_DUPLICATE_OBJECT")
        for source, command in sources.items():
            if not source.endswith(".S"):
                self.assertIn("-DX0X_GFINAL_CHANCE_PROBE=" + ("1" if enabled else "0"), command, "CHANCE_EXPLICIT_DEFINE")
        if enabled:
            command = sources[path]
            self.assertNotIn("-flto", command, "CHANCE_NO_LTO")
            self.assertNotIn("-ffast-math", command, "CHANCE_STRICT_MATH")
            self.assertIn("-ffp-contract=off", command, "CHANCE_NO_CONTRACTION")
            self.assertIn("-fno-fast-math", command, "CHANCE_STRICT_MATH")
            self.assertEqual(objects.count(build.OUT / "gfinal/chance_native.o"), 1, "CHANCE_PROVIDER_OBJECT")
        else:
            self.assertNotIn(build.OUT / "gfinal/chance_native.o", objects, "CHANCE_DISABLED_OBJECT")

    def test_chance_default_and_explicit_native_config(self):
        for model in ("elite", "pro"):
            for setting in (None, "1", "0"):
                with self.subTest(model=model, setting=setting):
                    self.assert_chance_plan(model, setting)
        with mock.patch.dict(os.environ, {"X0X_GFINAL_CHANCE_PROBE": "invalid"}):
            with self.assertRaisesRegex(SystemExit, "X0X_GFINAL_CHANCE_PROBE: 0 or 1"):
                compile_plan({})

    def test_chance_contract_rejects_actual_build_plan_mutations(self):
        source = inspect.getsource(build.app_compile_commands)
        mutations = [
            ('f not in ("-flto", "-ffast-math")', 'f not in ("-ffast-math",)', "CHANCE_NO_LTO", None),
            ('uf += ["-ffp-contract=off", "-fno-fast-math"]', 'uf += ["-ffp-contract=off", "-fno-fast-math", "-ffast-math"]', "CHANCE_STRICT_MATH", None),
            ('if chance_probe == "1":', 'if False:', "CHANCE_PROVIDER_PRESENCE", None),
            ('objs.append(obj)', 'objs.append(obj); units.append(units[-1])', "CHANCE_DUPLICATE_UNIT", None),
            ('"-DX0X_GFINAL_CHANCE_PROBE=" + chance_probe', '"-DX0X_GFINAL_CHANCE_PROBE=1"', "CHANCE_EXPLICIT_DEFINE", "0"),
        ]
        for old, new, marker, setting in mutations:
            self.assertEqual(source.count(old), 1, "mutation anchor changed")
            namespace = dict(build.__dict__, BOARD="smk37", MODEL="elite")
            exec(compile(source.replace(old, new), str(ROOT / "tools/build.py"), "exec"), namespace)
            with self.subTest(marker=marker), mock.patch.object(build, "app_compile_commands", namespace["app_compile_commands"]):
                with self.assertRaisesRegex(AssertionError, marker):
                    self.assert_chance_plan("elite", setting)

    def test_removed_engines_have_no_build_hook(self):
        # Even a currently unreachable build branch can accidentally compile a
        # deleted source after a later selector change. Keep the plan clean.
        code = (ROOT / "tools/build.py").read_text()
        plan = code.split("def app_compile_commands(", 1)[1].split("def build_app(", 1)[0]
        self.assertNotIn("experiments/wobble/wobble.c", plan)
        self.assertNotIn("dsp/bass303.c", plan)

    def test_default_is_original_and_zero_is_rejected(self):
        commands, objects, sources = compile_plan({})
        self.assertEqual((commands, objects), compile_plan({"X0X_ORIGINAL_SYNTH": "1"})[:2])
        self.assertNotIn("firmware/src/dsp/bass303.c", sources)
        self.assertNotIn("experiments/wobble/wobble.c", sources)
        for unsupported in ("0", "invalid"):
            with self.subTest(unsupported=unsupported):
                with self.assertRaisesRegex(SystemExit, "only 1 is supported"):
                    compile_plan({"X0X_ORIGINAL_SYNTH": unsupported})
        with self.assertRaisesRegex(SystemExit, "only 1 is supported"):
            build.app_compile_commands([], False, "0")

    def test_replacement_is_exclusive_and_preserves_independent_sources(self):
        commands, objects, sources = compile_plan({"X0X_ORIGINAL_SYNTH": "1"})
        self.assertNotIn("firmware/src/dsp/bass303.c", sources)
        self.assertNotIn("experiments/wobble/wobble.c", sources)
        self.assertNotIn("experiments/original_synth/master_unmuted.c", sources)
        for name in ("drum909", "drum808", "breaks", "fxbus", "master", "x0x_param"):
            self.assertIn(f"firmware/src/dsp/{name}.c", sources)
        for name in ("sequencer", "tb3po", "pattern", "motion"):
            self.assertIn(f"firmware/src/seq/{name}.c", sources)
        for name in ("original_chicago_backend", "original_miami_backend"):
            self.assertIn(f"firmware/src/dsp/{name}.c", sources)
        for name in ("miami_fx_conversion", "miami_decimator_adapter", "miami_decimator_component"):
            self.assertIn(f"experiments/wobble/{name}.c", sources)
        self.assertIn("experiments/original_synth/chicago_program.c", sources)
        self.assertIn("experiments/original_synth/miami_program.c", sources)
        self.assertIn("experiments/original_synth/chicago_fx2_compact.c", sources)
        self.assertNotIn("experiments/original_synth/chicago_fx_history.c", sources)
        self.assertNotIn("experiments/original_synth/chicago_fx2_bank.c", sources)
        self.assertEqual(len(objects), len(set(objects)), "object names must not collide")
        self.assertEqual(len(commands), len(sources), "each unit must be compiled once")
        self.assertIn(build.OUT / "o2/dsp_master.o", objects)
        self.assertIn(build.OUT / "original/master.o", objects)

    def test_reference_arithmetic_flags_and_existing_board_flags(self):
        _, _, sources = compile_plan({"X0X_ORIGINAL_SYNTH": "1"})
        for path, command in sources.items():
            if path.endswith(".S"):
                continue
            self.assertIn("-DBOARD_SMK37=1", command)
            self.assertIn("-DSMK_MODEL=1", command)
            self.assertIn("-DX0X_ORIGINAL_SYNTH=1", command)
            self.assertIn("-DX0X_WOBBLE=0", command)
            self.assertIn("-DX0X_DEV_WOBBLE=1", command)
            self.assertIn("-ffp-contract=off", command)
            self.assertNotIn("-ffast-math", command)
            if path.startswith("experiments/") or path.endswith("_backend.c"):
                # The admitted per-sample units use -O2; the remaining
                # original units retain -Os to fit the flash budget.
                optimized = Path(path).stem in build.ORIGINAL_SYNTH_O2_UNITS
                self.assertIn("-O2" if optimized else "-Os", command)
                self.assertNotIn("-Os" if optimized else "-O2", command)
                for flag in ("-fno-fast-math", "-DNDEBUG", "-DORIGINAL_USE_SOFT_FMA",
                             "-DMIAMI_FX_NO_HOST_FENV", "-Iexperiments/original_synth",
                             "-Iexperiments/wobble"):
                    self.assertIn(flag, command)
                self.assertNotIn("-Oz", command)
        self.assertIn("-O2", sources["firmware/src/dsp/drum909.c"])
        self.assertIn("-Oz", sources["firmware/src/x0x.c"])

    def test_conflicting_or_invalid_selectors_fail(self):
        for value in ("1", "invalid"):
            with self.subTest(flag="X0X_WOBBLE", value=value):
                with self.assertRaisesRegex(SystemExit, "X0X_WOBBLE"):
                    compile_plan({"X0X_ORIGINAL_SYNTH": "1", "X0X_WOBBLE": value})
        with self.assertRaisesRegex(SystemExit, "X0X_DEV_WOBBLE"):
            compile_plan({"X0X_ORIGINAL_SYNTH": "1", "X0X_DEV_WOBBLE": "invalid"})
        with self.assertRaisesRegex(SystemExit, "X0X_ORIGINAL_SYNTH: only 1 is supported"):
            compile_plan({"X0X_ORIGINAL_SYNTH": "invalid"})
        self.assertEqual(build.synth_build_config({"X0X_ORIGINAL_SYNTH": "1",
                                                  "X0X_WOBBLE": "0",
                                                  "X0X_DEV_WOBBLE": "0"}), (True, "0"))

    def test_original_rate_pads_do_not_compile_legacy_wobble(self):
        commands, objects, sources = compile_plan({
            "X0X_ORIGINAL_SYNTH": "1", "X0X_DEV_WOBBLE": "1"})
        self.assertNotIn("experiments/wobble/wobble.c", sources)
        self.assertNotIn("firmware/src/dsp/bass303.c", sources)
        self.assertIn("firmware/src/dsp/original_miami_backend.c", sources)
        self.assertIn("firmware/src/dsp/original_chicago_backend.c", sources)
        for command in commands:
            if "-c" not in command or Path(command[command.index("-c") + 1]).suffix == ".S":
                continue  # startup assembly has no C preprocessor flags
            self.assertIn("-DX0X_ORIGINAL_SYNTH=1", command)
            self.assertIn("-DX0X_WOBBLE=0", command)
            self.assertIn("-DX0X_DEV_WOBBLE=1", command)
        self.assertEqual(len(objects), len(set(objects)))

    def test_original_ui_rate_flag_does_not_activate_legacy_dsp_sources(self):
        # X0X_DEV_WOBBLE=1 enables the ported *pad UI* for original Miami.
        # Only the original Miami rate-pad UI may use the DEV_WOBBLE
        # selector. The retired WBL DSP is never selectable by this flag.
        for path in ("firmware/src/app/engine.c", "firmware/src/app/ui.c",
                     "firmware/src/app/main_fm1.c"):
            with self.subTest(path=path):
                source = (ROOT / path).read_text()
                self.assertNotRegex(source, r"(?m)^#(?:if|elif) X0X_DEV_WOBBLE\s*$")
                self.assertNotRegex(source, r"(?m)^#if X0X_DEV_WOBBLE && !X0X_ORIGINAL_SYNTH$")
        ui = (ROOT / "firmware/src/app/ui.c").read_text()
        self.assertIn("#if BOARD_SMK37 && X0X_DEV_WOBBLE && X0X_ORIGINAL_SYNTH", ui)
        self.assertIn("static int division_pad_owner(void)", ui)
        self.assertIn("ORIGINAL_MIAMI_WOBBLE_RATE", ui)
        self.assertNotIn("shape_pad_owner(", ui)

    def test_rate_pad_port_preserves_574f_press_ownership_and_native_eight_rates(self):
        # The physical Free Play route was ported from 574f/6ae. No shape-pad
        # functionality belongs to this lane. Verify press-time ownership and
        # the eight original Miami divisions, not the retired WBL1 backend.
        ui = (ROOT / "firmware/src/app/ui.c").read_text()
        hal = (ROOT / "firmware/hal/smk_input.h").read_text()
        plat = (ROOT / "firmware/src/app/plat_fm1.c").read_text()
        for guard in ("!ui.freeplay[bidx()]",
                      "engine_bass_bank(bidx()) != ORIGINAL_SOUND_MIAMI",
                      "smk_clip_mode_on()", "smc_ui_active()", "project_busy",
                      "perf_testing()", "ui.rec", "ui.help",
                      "ui.overlay != O_NONE"):
            with self.subTest(guard=guard):
                self.assertIn(guard, ui)
        self.assertIn("int pad = plat_division_pad((unsigned)owner)", ui)
        self.assertIn("if (owner && pad >= 0 && pad < 8)", ui)
        self.assertIn("ORIGINAL_MIAMI_WOBBLE_RATE", ui)
        self.assertIn("smk_div_held & owned", hal)
        self.assertIn("if (!vel) smk_div_held &= (uint16_t)~owned", hal)
        # Physical capture covers all existing Free Play pads. The UI rate
        # consumer above remains limited to eight; other captured pads retain
        # the committed baseline's shape/empty routing, never a second note.
        self.assertIn("if (vel && pad < SMK_NPAD && smk_div_owner && !smk_pad_note[pad])", hal)
        self.assertIn("smk_div_pending = (uint8_t)(pad + 1u);", hal)
        self.assertIn("smk_div_pending = 0;", hal)
        self.assertIn("smk_div_owner = (uint8_t)owner;", hal)
        self.assertRegex(plat, r"(?s)uint32_t flags = irq_save\(\);"
                              r"\s*int hit = smk__division_poll\(owner\);"
                              r"\s*irq_restore\(flags\);")
        for pad, name in enumerate(("1/2", "1/4", "1/8", "1/16",
                                    "1/3", "1/6", "1/12", "1/24")):
            with self.subTest(pad=pad, name=name):
                value = (pad * 127 + 3) // 7
                self.assertEqual((value * 7 + 63) // 127, pad)
                self.assertIn('RATE ' + name, ui)
        self.assertIn('say("WOBBLE ", division_pad_label())', ui)
        self.assertIn('cv_text(dx, 6, &FONT_T, division_pad_label(), C_AMB)', ui)
        self.assertNotIn("shape_pad_owner(", ui)

    def test_division_pad_owner_cached_once_in_header_and_knobs_sig(self):
        # draw_header and knobs_sig used to call the owner predicate several
        # times in one frame; keep a single local and reuse it.
        ui = (ROOT / "firmware/src/app/ui.c").read_text()

        def extract(signature):
            i = ui.index(signature)
            brace = ui.index("{", i)
            depth = 0
            for j, ch in enumerate(ui[brace:], brace):
                if ch == "{":
                    depth += 1
                elif ch == "}":
                    depth -= 1
                    if depth == 0:
                        return ui[i:j + 1]
            self.fail("could not extract " + signature)

        header = extract("static UI_DRAW void draw_header(void)")
        knobs = extract("static UI_DRAW uint32_t knobs_sig(void)")
        self.assertEqual(header.count("division_pad_owner()"), 1)
        self.assertIn("int div_owner = division_pad_owner();", header)
        self.assertIn("!div_owner", header)
        self.assertIn("if (div_owner && !pickup && !clip_mode)", header)
        self.assertEqual(knobs.count("division_pad_owner()"), 1)
        self.assertIn("int div_owner = division_pad_owner();", knobs)
        self.assertIn("MIXIN(div_owner);", knobs)
        self.assertIn("if (div_owner) MIXIN(division_pad_index());", knobs)

    def test_host_build_allows_original_rate_pad_flag_without_legacy_sources(self):
        # The host app should accept the same UI-only build flag as firmware.
        host = (ROOT / "host/build_host.sh").read_text()
        self.assertIn('DEV_WOBBLE="${X0X_DEV_WOBBLE:-1}"', host)
        self.assertIn('case "$DEV_WOBBLE" in 0|1)', host)
        self.assertIn('-DX0X_ORIGINAL_SYNTH=1 -DX0X_WOBBLE=0 '
                      '-DX0X_DEV_WOBBLE=$DEV_WOBBLE', host)
        self.assertIn("*build.ORIGINAL_SYNTH_UNITS, *build.ORIGINAL_SYNTH_BACKENDS", host)

    def test_original_rate_pad_default_and_explicit_opt_out(self):
        for opts, enabled in (({}, "1"),
                              ({"X0X_DEV_WOBBLE": "1"}, "1"),
                              ({"X0X_DEV_WOBBLE": "0"}, "0")):
            with self.subTest(options=opts):
                commands, objects, sources = compile_plan(
                    {"X0X_ORIGINAL_SYNTH": "1", **opts})
                self.assertEqual(len(objects), len(set(objects)))
                self.assertIn("firmware/src/dsp/original_miami_backend.c", sources)
                self.assertNotIn("experiments/wobble/wobble.c", sources)
                self.assertNotIn("firmware/src/dsp/bass303.c", sources)
                for command in commands:
                    if "-c" not in command or Path(command[command.index("-c") + 1]).suffix == ".S":
                        continue  # startup assembly is not compiled as C
                    self.assertIn("-DX0X_ORIGINAL_SYNTH=1", command)
                    self.assertIn("-DX0X_WOBBLE=0", command)
                    self.assertIn(f"-DX0X_DEV_WOBBLE={enabled}", command)

    def test_legacy_profile_never_builds_even_with_wobble_disabled(self):
        for selector in ({"X0X_WOBBLE": "0"}, {"X0X_DEV_WOBBLE": "0"}):
            with self.subTest(selector=selector):
                with self.assertRaisesRegex(SystemExit, "only 1 is supported"):
                    compile_plan({"X0X_ORIGINAL_SYNTH": "0", **selector})

    def test_missing_replacement_fails_before_any_tool_is_called(self):
        with tempfile.TemporaryDirectory() as directory:
            with mock.patch.object(build, "SRC", Path(directory)), \
                 mock.patch.dict(os.environ, {"X0X_ORIGINAL_SYNTH": "1"}, clear=True), \
                 mock.patch.object(sys, "argv", ["build.py"]), \
                 mock.patch.object(build, "tc") as tool, \
                 mock.patch.object(build, "tc_all") as tools, \
                 mock.patch.object(build, "clang_std_inc") as includes, \
                 mock.patch.object(build.subprocess, "run") as command:
                for entry in (build.build_app, build.main):
                    with self.assertRaisesRegex(SystemExit, "missing replacement sources") as failure:
                        entry()
                    self.assertIn("original_chicago_backend.c", str(failure.exception))
                    self.assertIn("original_miami_backend.c", str(failure.exception))
                tool.assert_not_called()
                tools.assert_not_called()
                includes.assert_not_called()
                command.assert_not_called()

    def test_invalid_external_inputs_fail_before_any_tool_is_called(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            required = [*build.ORIGINAL_SYNTH_UNITS, *build.ORIGINAL_SYNTH_BACKENDS,
                        *(str(Path(p).with_suffix(".h")) for p in build.ORIGINAL_SYNTH_BACKENDS)]
            for name in required:
                path = root / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.touch()
            with mock.patch.object(build, "SRC", root), \
                 mock.patch.dict(os.environ, {"X0X_ORIGINAL_SYNTH": "1"}, clear=True), \
                 mock.patch.object(sys, "argv", ["build.py"]), \
                 mock.patch.object(build, "tc") as tool, \
                 mock.patch.object(build, "tc_all") as tools, \
                 mock.patch.object(build, "clang_std_inc") as includes, \
                 mock.patch.object(build.subprocess, "run") as command:
                for entry in (build.build_app, build.main):
                    with self.assertRaisesRegex(SystemExit, "invalid external inputs"):
                        entry()
                tool.assert_not_called()
                tools.assert_not_called()
                includes.assert_not_called()
                command.assert_not_called()


if __name__ == "__main__":
    unittest.main(verbosity=2)
