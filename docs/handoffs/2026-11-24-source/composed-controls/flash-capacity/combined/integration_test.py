"""Prove the 8 KiB extent is conditionally acquired only after verified migration.

All input images come from fixture builders. No external NOR file is opened,
no package is generated, and no retained owner data is written.
"""
import argparse
import json
from pathlib import Path
import sys

import layout

def main():
    q=argparse.ArgumentParser()
    q.add_argument('journal_source',type=Path)
    a=q.parse_args()
    sys.path.insert(0,str(a.journal_source.resolve()))
    import migration
    import test_migration

    def planned(image,backup,model):
        return layout.test_plan(8 if migration.may_retire(image,backup,model) else 0,57120,46037)

    rows=[]
    for board in migration.PROFILES:
        for legacy_active in (0,1):
            original=test_migration.fixture(legacy_active)
            backup=migration.pack_backup(original,board)
            before=planned(original,backup,board)
            assert before['data_before_reserve_B']==8192
            migrated,op=migration.migrate(original,backup,board)
            # Interrupt at every named boundary; no reclaimed space is counted
            # until both committed journal records are independently present.
            half=bytearray(original)
            first_admitted=None
            for k,step in enumerate(op):
                migration.write(half,step)
                if migration.may_retire(half,backup,board):
                    first_admitted=k
                    assert bytes(half)==migrated
                else:
                    assert planned(half,backup,board)['data_before_reserve_B']==8192
            assert first_admitted==len(op)-1
            after=planned(migrated,backup,board)
            assert after['data_before_reserve_B']==16384
            assert [a[:2] for a in after['virtual_free_extents'] if a[2].startswith('old PANEL')]==[['0xa5000','0xa7000']]
            # Only restorable immutable data is admitted here. It has an
            # independent source and need not be preserved by legacy rollback.
            mock_companion=bytearray(migrated)
            mock_companion[0xa5000:0xa7000]=b'I'*8192
            assert migration.reader(mock_companion)==migration.reader(migrated)
            assert migration.rollback(mock_companion,backup,board)==original
            rows.append({'board':board,'old_active_copy':legacy_active,
                         'before_KiB':8,'after_KiB':16,
                         'first_confirmed_stage':first_admitted+1})
    assert len(rows)==4
    print(json.dumps(dict(offline=True,verified_original_legacy_rollback=True,
        boards=rows,study_only_not_firmware=True),indent=2))

if __name__=='__main__':main()
