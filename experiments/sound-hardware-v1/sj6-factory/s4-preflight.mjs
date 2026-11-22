#!/usr/bin/env node
// Read-only provenance check; this script does not render an Audio Unit.
import assert from 'node:assert/strict';
import {createHash} from 'node:crypto';
import {readFileSync} from 'node:fs';
import {dirname, join} from 'node:path';
import {fileURLToPath} from 'node:url';

function preflight(matrix, release) {
  assert.equal(matrix.sample_rate, 44100);
  assert.equal(matrix.channels, 2);
  assert.equal(matrix.capture_frames, 132300);
  assert.equal(matrix.block_frames, 128);
  assert.equal(matrix.gain, 1);
  assert.equal(matrix.level_limit_db, 1);
  assert.equal(matrix.spectral_limit_db, 3);
  assert.deepEqual(matrix.band_hz, [20, 16000]);
  assert.equal(matrix.programs.length, 14);
  assert.equal(release.programs.length, 14);
  assert.equal(matrix.events.length, 8);
  assert.equal(new Set(matrix.programs.map(p => p.program_uid)).size, 14);
  assert.equal(new Set(matrix.programs.map(p => p.bank_uuid)).size, 12);
  assert.deepEqual(matrix.events.map(e => e.frame), [0,22050,33075,33075,33075,66150,66150,66150]);
  assert(matrix.events.every(e => e.frame >= 0 && e.frame < matrix.capture_frames));
  const releaseByUid = new Map(release.programs.map(r => [r.uid, r]));
  assert.equal(releaseByUid.size, 14);
  let authored = 0, decimal = 0, metadata = 0;
  for (const p of matrix.programs) {
    const row = releaseByUid.get(p.program_uid);
    assert(row, `missing release row ${p.program_uid}`);
    assert.equal(row.supplied_bank_uuid, p.bank_uuid, p.program_uid);
    assert(row.parameter_count >= 1 && Number.isInteger(row.parameter_count));
    authored += row.parameter_count;
    decimal += Number(row.decimal_differences.length > 0);
    metadata += Number(row.metadata_differences.length > 0);
  }
  assert.equal(authored, 1926);
  return {
    result: 'PASS_STATIC_SOURCE_AUDIT_ONLY',
    source: 's4-inputs.json and original/bundled-bank-audit.json',
    selected_programs: matrix.programs.length,
    selected_banks: new Set(matrix.programs.map(p => p.bank_uuid)).size,
    matched_selected_uids_and_bank_ids: matrix.programs.length,
    authored_parameter_records: authored,
    entries_with_decimal_spelling_differences: decimal,
    entries_with_metadata_differences: metadata,
    original_au_digest_claimed_by_audit: release.au_sha256,
    original_ipa_digest_claimed_by_audit: release.ipa_sha256,
    frames_per_stereo_program: matrix.capture_frames,
    events_per_program: matrix.events.length,
    qualifying_original_stereo_renders: 0,
    qualifying_firmware_stereo_pairs: 0,
    caveat: 'Original AU executable was not executed, no original stereo PCM generated; recorded checks are fixture metadata only.'
  };
}

const base = dirname(fileURLToPath(import.meta.url));
const matrixFile = join(base, 's4-inputs.json');
const auditFile = join(base, 'original', 'bundled-bank-audit.json');
const matrixBytes = readFileSync(matrixFile);
const releaseBytes = readFileSync(auditFile);
const report = preflight(JSON.parse(matrixBytes), JSON.parse(releaseBytes));
report.matrix_sha256 = createHash('sha256').update(matrixBytes).digest('hex');
report.audit_sha256 = createHash('sha256').update(releaseBytes).digest('hex');
console.log(JSON.stringify(report, null, 2));
