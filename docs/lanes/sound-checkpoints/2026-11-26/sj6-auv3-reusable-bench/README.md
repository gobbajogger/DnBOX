# Reusable direct macOS SuperJ6 AUv3 bench

Build once with `bash build.sh`. This creates one identifiable local app
at `~/DnBOX-AU-Bench/DnBOX AUv3 Bench.app` without changing macOS settings
or installing a service. Open it once in a normal Mac GUI session. Leave
the window open to run more jobs in that same session, or click **Quit
Bench** to stop. Reopening remains subject to the host's normal approval
rules.

Queue requests while it is open:

```sh
python3 submit.py test-01 master
python3 submit.py test-02 dynamic --step 24576:1 --step 61440:0.3345831036567688
```

Profiles: `master`, `fanin`, `static`, `physical` and `dynamic`.
The dynamic profile can take up to eight timestamped #7 writes while
the cue is held; the last must restore the authored value. Recipes
are data only: no shell commands, custom plug-in paths, executable code
or external URLs. The host pins the installed official AUv3 binary,
uses offline audio output, and rejects unknown profiles and duplicate
IDs. It writes only to the local bench directory. Each
`jobs/<id>/result.json` reports success/failure; the same directory holds
the capture data and receipts. The app does not start at login.

Verified on the Mac: launched once, completed the four master captures,
then queued a separate job and completed its four dynamic-cutoff captures
in the same process. An invalid profile was rejected without running.
No app binary is stored in Git.
