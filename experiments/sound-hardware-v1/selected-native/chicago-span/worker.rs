use fm1_core::{Bus,Cpu};
struct Memory(Vec<u8>);
impl Bus for Memory{
 fn read8(&mut self,a:u32)->u8{self.0[a as usize]}
 fn read16(&mut self,a:u32)->u16{u16::from_le_bytes(self.0[a as usize..][..2].try_into().unwrap())}
 fn read32(&mut self,a:u32)->u32{u32::from_le_bytes(self.0[a as usize..][..4].try_into().unwrap())}
 fn write8(&mut self,a:u32,x:u8){self.0[a as usize]=x}
 fn write16(&mut self,a:u32,x:u16){self.0[a as usize..][..2].copy_from_slice(&x.to_le_bytes())}
 fn write32(&mut self,a:u32,x:u32){self.0[a as usize..][..4].copy_from_slice(&x.to_le_bytes())}
}
fn run(image:&[u8],probe:&[u8],measure:u32,factory:u32,phase:u32,fx:u32) {
 let entry=0x02090000u32;
 let mut bus=Memory(vec![0;0x020c0000]);
 let mut cursor = 0;
 while cursor < image.len() {
   let addr = u32::from_le_bytes(image[cursor..cursor+4].try_into().unwrap()) as usize;
   let size = u32::from_le_bytes(image[cursor+4..cursor+8].try_into().unwrap()) as usize;
   cursor += 8;
   bus.0[addr..addr+size].copy_from_slice(&image[cursor..cursor+size]);
   cursor += size;
 }
 bus.0[entry as usize..entry as usize+probe.len()].copy_from_slice(&probe);
 
 let result=0x1c07f00;
 bus.write32(result+32,factory); bus.write32(result+36,phase); bus.write32(result+40,fx);
 let mut cpu=Cpu::new(entry);
 let mut caller_sp=0u32; let mut caller_ssp=0u32; let mut caller_user=false;
 let mut in_job=false; let mut ret=0u32; let mut sp=0; let mut floor=0;
 let mut stack_pc=0;let mut n=0u32; let mut peak=0; let mut depth=[0;6];let mut min_irq=0;
 while cpu.pc!=0 && cpu.insn_count<10000000 {
   if cpu.pc==measure {
     assert!(!in_job);
     caller_sp=cpu.sp; caller_ssp=cpu.ssp; caller_user=cpu.user;
     cpu.sp=0x01c75100-8; cpu.ssp=0x01c75200; cpu.user=true; cpu.core_id=1;
     in_job=true; ret=cpu.rets; sp=cpu.sp; floor=sp; min_irq=cpu.ssp;
   }
   let pc=cpu.pc; cpu.step(&mut bus).unwrap();
   if in_job {
      if cpu.sp < floor {floor=cpu.sp; if sp-floor>peak {peak=sp-floor;stack_pc=pc;}}
      assert_eq!(cpu.ssp,min_irq);
      if cpu.pc==ret {assert_eq!(cpu.sp,sp); assert!(floor>=0x1c74100);
          depth[n as usize]=sp-floor;n+=1;in_job=false;
          cpu.sp=caller_sp; cpu.ssp=caller_ssp; cpu.user=caller_user; cpu.core_id=0;}
   }
 }
 print!("{factory},{phase},{fx},{},{},{},{},{},{},{n},{peak},{stack_pc:x}",cpu.pc,cpu.insn_count,bus.read32(result+12),bus.read32(result+16),bus.read32(result+24),bus.read32(result+28));
 for d in depth{print!(",{d}");} println!();
}
fn main(){
 let a:Vec<String>=std::env::args().collect();
 let image=std::fs::read(&a[1]).unwrap();let probe=std::fs::read(&a[2]).unwrap();
 let measure=u32::from_str_radix(&a[3],16).unwrap();
 println!("factory,phase,fx,pc,instructions,ended,live,selected,signal,jobs,peak,deepest_pc,d0,d1,d2,d3,d4,d5");
 let mut configs=vec![];
 if a.len()==7 {configs.push((a[4].parse().unwrap(),a[5].parse().unwrap(),a[6].parse().unwrap()));}
 else {for f in 0..81{configs.push((f,0,255));}for fx in 0..12{for phase in 0..64{configs.push((0,phase,fx));}}}
 for (factory,phase,fx) in configs{run(&image,&probe,measure,factory,phase,fx);}
}
