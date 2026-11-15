"""Build only this original proxy from trusted local source; retain all failures."""
import argparse, datetime, hashlib, json, os, shutil, subprocess, sys
from pathlib import Path

def run(output,archive=None,reference=None):
    root=Path(__file__).resolve().parent
    if (archive is None)!=(reference is None):raise ValueError("archive and reference are a pair")
    output=output.resolve();output.mkdir(exist_ok=False,parents=False)
    cc=shutil.which("cc",path=os.defpath)
    if not cc:raise RuntimeError("trusted installed cc missing")
    env={"PATH":os.defpath,"LANG":"C","LC_ALL":"C","TMPDIR":str(output),"PYTHONDONTWRITEBYTECODE":"1"}
    records=[]
    def call(name,argv,extra=None):
        e=dict(env);e.update(extra or {})
        p=subprocess.run([str(x) for x in argv],cwd=root,env=e,capture_output=True,text=True)
        (output/(name+".stdout")).write_text(p.stdout);(output/(name+".stderr")).write_text(p.stderr)
        r=dict(name=name,argv=[str(x) for x in argv],exit=p.returncode,completed_utc=datetime.datetime.now(datetime.timezone.utc).isoformat())
        records.append(r);return p.returncode
    common=[cc,"-std=c99","-Wall","-Wextra","-Werror","-Wno-misleading-indentation","-ffp-contract=off","-fno-fast-math","-fno-builtin"]
    call("projection",["python3","-B","-m","unittest","-v","test_projection.py"])
    for opt in ("O2","Os"):
        exe=output/opt
        if not call("compile-"+opt,[*common,"-"+opt,"sj6x.c","test_sj6x.c","-o",exe]):call("run-"+opt,[exe])
    san=output/"san"
    if not call("compile-san",[*common,"-O1","-g","-fsanitize=address,undefined,float-cast-overflow","-fno-sanitize-recover=all","-fno-omit-frame-pointer","sj6x.c","test_sj6x.c","-o",san]):
        call("run-san-default",[san])
        call("run-san-no-leaks",[san],{"ASAN_OPTIONS":"detect_leaks=0"})
    corpus="NOT_RUN_no_private_input"
    if archive is not None:
        so=output/"proxy.so"
        if not call("compile-corpus",[*common,"-O2","-shared","-fPIC","sj6x.c","-o",so]):
            corpus=call("corpus",["python3","-B","run_corpus.py","--archive",archive.resolve(),"--reference",reference.resolve(),"--library",so])
    no_leaks=next((x for x in records if x["name"]=="run-san-no-leaks"),None)
    exceptions=[]
    for rec in records:
        if rec["name"]=="run-san-default" and rec["exit"]==1 and no_leaks and no_leaks["exit"]==0 and "LeakSanitizer does not work under ptrace" in (output/"run-san-default.stderr").read_text():
            exceptions.append("run-san-default")
    failed=[x["name"] for x in records if x["exit"] and x["name"] not in exceptions]
    result={"scope":"original proxy, not SuperJ6 DSP/fidelity/firmware","source_sha256":hashlib.sha256((root/"sj6x.c").read_bytes()).hexdigest(),
            "records":records,"private_corpus":corpus,"required_failures":failed,"mandatory_checks_pass":not failed,"known_environment_exceptions":exceptions,"default_sanitizer_failure_is_retained":True,
            "not_run":["reference_app_audio","target_execution","firmware","product_stack_cpu_fit","device"]}
    (output/"RESULT.json").write_text(json.dumps(result,indent=2)+"\n")
    return result
if __name__=="__main__":
    p=argparse.ArgumentParser();p.add_argument("--output",type=Path,required=True);p.add_argument("--archive",type=Path);p.add_argument("--reference",type=Path);a=p.parse_args()
    result=run(a.output,a.archive,a.reference);print(json.dumps(result,indent=2));sys.exit(0 if result["mandatory_checks_pass"] else 1)
