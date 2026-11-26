import re,sys,pathlib
s=pathlib.Path(sys.argv[1]).read_text()
pos=s.index('#if X0X_S1_EXTRA\n    part->extra_live_mask')
end=s.index('    return ok;\n}',pos)
body=s[pos:end]
assert 'part->program = (uint16_t)program;' in body
ins=s.index('int s1_part_program(')
s=s[:ins]+'static void s1_finish(s1_dry_part *part, unsigned program)\n{\n    s1c *e = &part->e;\n'+body+'}\n\n'+s[ins:pos]+'    s1_finish(part, program);\n'+s[end:]
print(s,end='')
