
struct CutoffStep: Codable {
    let frame: Int
    let value: Float
}
struct BenchJob: Codable {
    let id: String
    let profile: String
    let cutoffSteps: [CutoffStep]?
}
typealias ProfileCapture = (String, String, AVAudioUnit, URL, [CutoffStep]?) throws -> Void
struct BenchProfile {
    let run: ProfileCapture
    let sequence: [(String,String)]
}
let authoredCutoff = Float(0.3345831036567688)
let physicalSequence = ["physical-authored","physical-open","physical-authored-repeat","physical-open-repeat"].map{($0,"authored")}
let profiles: [String:BenchProfile] = [
    "master": BenchProfile(run:capture_master,sequence:[("main-only","authored"),("main-only","half"),("sub-only","authored"),("sub-only","half")]),
    "fanin": BenchProfile(run:capture_fanin,sequence:["all","osc1-only","osc2-only","sub-only","all-repeat","osc1-only-repeat","osc2-only-repeat","sub-only-repeat"].map{($0,"authored")}),
    "static": BenchProfile(run:capture_static,sequence:["osc2-only","osc2-only-repeat"].map{($0,"authored")}),
    "physical": BenchProfile(run:capture_physical,sequence:physicalSequence),
    "dynamic": BenchProfile(run:capture_dynamic,sequence:physicalSequence),
    "jupiter-gain": BenchProfile(run:capture_gain,sequence:[
        ("osc1-only","authored"),("osc1-only","half"),("osc2-only","authored"),("osc2-only","half"),
        ("osc1-only-repeat","authored"),("osc2-only-repeat","authored"),("both-muted","authored")])
]

final class Bench: NSObject, NSApplicationDelegate, NSWindowDelegate {
    let root = Bundle.main.bundleURL.deletingLastPathComponent()
    let renderQueue = DispatchQueue(label:"org.dnbox.auv3.bench.render")
    let app = NSApplication.shared
    var timer: Timer?
    var pending=false
    var token=0
    var job: BenchJob?
    var currentDir: URL?
    var messages=[String]()
    let window = NSWindow(contentRect:NSRect(x:0,y:0,width:630,height:320),styleMask:[.titled,.closable,.miniaturizable],backing:.buffered,defer:false)
    let label=NSTextField(labelWithString:"Starting local AUv3 bench…")

    func applicationDidFinishLaunching(_ notification: Notification) {
        window.title="DnBOX AUv3 Bench"
        window.delegate=self
        label.frame=NSRect(x:16,y:55,width:598,height:245)
        label.maximumNumberOfLines=14
        window.contentView?.addSubview(label)
        let quit=NSButton(title:"Quit Bench",target:self,action:#selector(closeBench))
        quit.frame=NSRect(x:505,y:14,width:110,height:30)
        window.contentView?.addSubview(quit)
        window.center()
        window.makeKeyAndOrderFront(nil)
        publish("Ready. Waiting for a validated local test request.")
        let t=Timer(timeInterval:0.75,repeats:true){[weak self] _ in self?.poll()}
        timer=t
        RunLoop.main.add(t,forMode:.common)
    }
    @objc func closeBench() {app.terminate(nil)}
    func windowWillClose(_ notification: Notification) {app.terminate(nil)}
    func write(_ text:String,to url:URL) {try? text.write(to:url,atomically:true,encoding:.utf8)}
    func publish(_ message:String) {
        messages.append(message)
        if messages.count>40 {messages.removeFirst()}
        label.stringValue=messages.suffix(12).joined(separator:"\n")
        write(label.stringValue+"\n",to:root.appendingPathComponent("session-status.txt"))
        if let dir=currentDir {write(messages.joined(separator:"\n")+"\n",to:dir.appendingPathComponent("status.txt"))}
    }
    func reject(_ file:URL,_ message:String) {
        let failure=file.deletingPathExtension().appendingPathExtension("rejected")
        try? FileManager.default.moveItem(at:file,to:failure)
        publish("Rejected \(file.lastPathComponent): \(message)")
    }
    func poll() {
        if pending {return}
        let fm=FileManager.default, inbox=root.appendingPathComponent("requests",isDirectory:true)
        guard let urls=try? fm.contentsOfDirectory(at:inbox,includingPropertiesForKeys:[.isRegularFileKey,.isSymbolicLinkKey,.fileSizeKey],options:[.skipsHiddenFiles]) else {return}
        for f in urls.sorted(by:{$0.lastPathComponent<$1.lastPathComponent}) where f.pathExtension=="json" {
            let id=f.deletingPathExtension().lastPathComponent
            guard id.range(of:"^[A-Za-z0-9][A-Za-z0-9_-]{0,47}$",options:.regularExpression) != nil,
                  let m=try? f.resourceValues(forKeys:[.isRegularFileKey,.isSymbolicLinkKey,.fileSizeKey]),m.isRegularFile==true,m.isSymbolicLink==false,(m.fileSize ?? 65536)<16384,
                  let data=try? Data(contentsOf:f),let j=try? JSONDecoder().decode(BenchJob.self,from:data),j.id==id,let prof=profiles[j.profile],validate(j)
            else {reject(f,"invalid recipe or unsupported profile");continue}
            let dir=root.appendingPathComponent("jobs",isDirectory:true).appendingPathComponent(j.id,isDirectory:true)
            do {
                try fm.createDirectory(at:dir,withIntermediateDirectories:false,attributes:[.posixPermissions:0o700])
                try fm.moveItem(at:f,to:dir.appendingPathComponent("request.json"))
            } catch {reject(f,"job path already exists or is not writable");continue}
            pending=true
            job=j
            currentDir=dir
            messages=[]
            publish("Running \(j.id): \(j.profile), process \(ProcessInfo.processInfo.processIdentifier)")
            let binary=URL(fileURLWithPath:"/Applications/Synth One J6.app/Contents/PlugIns/SuperJ6 AU.appex/Contents/MacOS/SuperJ6 AU")
            guard let bytes=try? Data(contentsOf:binary,options:.mappedIfSafe),
                  digest(bytes)=="1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257"
            else {complete(false,"official AUv3 binary differs from pinned reference");return}
            next(j,prof,0,dir)
            return
        }
    }
    func validate(_ j:BenchJob)->Bool {
        guard let s=j.cutoffSteps else {return true}
        guard j.profile=="dynamic",!s.isEmpty,s.count<=8,s.last?.value==authoredCutoff else {return false}
        var last=16383
        for step in s {
            guard step.frame>last,step.frame<82534,step.value.isFinite,step.value>=0,step.value<=1 else {return false}
            last=step.frame
        }
        return true
    }
    func complete(_ success:Bool,_ reason:String) {
        guard let dir=currentDir else {return}
        publish((success ? "PASS " : "FAILED ")+reason)
        let fields=["state":success ? "pass" : "failed","detail":reason,"pid":String(ProcessInfo.processInfo.processIdentifier)]
        if let bytes=try? JSONSerialization.data(withJSONObject:fields,options:[.prettyPrinted,.sortedKeys]) {
            try? bytes.write(to:dir.appendingPathComponent("result.json"),options:.atomic)
        }
        token += 1
        currentDir=nil
        job=nil
        pending=false
    }
    func next(_ j:BenchJob,_ prof:BenchProfile,_ i:Int,_ dir:URL,_ optionIndex:Int=0) {
        guard i<prof.sequence.count else {complete(true,"all \(i) native renders complete");return}
        let (mode,level)=prof.sequence[i]
        let options:[AudioComponentInstantiationOptions] = [[.loadOutOfProcess],[]]
        guard optionIndex<options.count else {complete(false,"both normal loading modes failed");return}
        var desc=AudioComponentDescription(componentType:0x61756d75,componentSubType:0x6a366374,componentManufacturer:0x41755072,componentFlags:0,componentFlagsMask:0)
        guard AudioComponentFindNext(nil,&desc) != nil else {complete(false,"installed component not visible in this GUI session");return}
        token+=1
        let generation=token
        publish("Loading \(mode) \(level)")
        DispatchQueue.main.asyncAfter(deadline:.now()+15) {[weak self] in
            guard let self,self.token==generation else {return}
            self.next(j,prof,i,dir,optionIndex+1)
        }
        AVAudioUnit.instantiate(with:desc,options:options[optionIndex]) {[weak self] node,error in
            DispatchQueue.main.async {
                guard let self,self.token==generation else {return}
                self.token+=1
                guard let node else {self.publish("Load returned \(String(describing:error))");self.next(j,prof,i,dir,optionIndex+1);return}
                self.publish("Rendering \(mode) \(level)")
                self.renderQueue.async {
                    do {
                        try prof.run(mode,level,node,dir,j.cutoffSteps)
                        DispatchQueue.main.async {self.publish("Rendered \(mode) \(level)");self.next(j,prof,i+1,dir)}
                    } catch {
                        DispatchQueue.main.async {self.complete(false,String(describing:error))}
                    }
                }
            }
        }
    }
}
let app = NSApplication.shared
let bench=Bench()
app.setActivationPolicy(.accessory)
app.delegate=bench
app.run()
