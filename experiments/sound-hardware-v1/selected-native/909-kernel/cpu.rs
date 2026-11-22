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
fn main(){
 let args:Vec<String>=std::env::args().collect();
 let image=std::fs::read(&args[1]).unwrap();
 let probe=std::fs::read(&args[2]).unwrap();
 let entry=0x02090000;
 let measure=u32::from_str_radix(&args[3],16).unwrap();
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
 let mut cpu=Cpu::new(entry); let mut base=0u32; let mut minsp=u32::MAX;
 while cpu.pc!=0 && cpu.insn_count<10000000 {
 if cpu.pc==measure {base=cpu.sp;}
 cpu.step(&mut bus).unwrap();
 if base!=0 {minsp=minsp.min(cpu.sp);}
 }
 println!("PC={:x} instructions={} checks={} {} {} {} stack_base={:x} depth={}",cpu.pc,cpu.insn_count,bus.read32(0x1c07f00),bus.read32(0x1c07f04),bus.read32(0x1c07f08),bus.read32(0x1c07f0c),base,base.wrapping_sub(minsp));
}