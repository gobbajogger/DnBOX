#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Check the actual interactive R22 slabs in every ordinary GLOBAL and non-PART list."""
import argparse
from collections import defaultdict
import csv
import hashlib
import io
import json
from pathlib import Path
import re
import tarfile

from PIL import Image, ImageChops

from check_synthone_layout import list_row_ink, measure_ink


MENU = re.compile(r'(global-section--?\d+|list-view-(\d+)-part-(\d+)-voice-\d+)-row-(\d+)')


def selected_states(names):
    groups = defaultdict(set)
    for name in names:
        match = MENU.fullmatch(name)
        if not match:
            continue
        group, view, part, row = match.groups()
        if view and int(view) == 1 and int(part) < 4:
            continue  # Ordinary drum and bass PART screens have their own nine-row fixture.
        groups[group].add(int(row))
    return groups


def read_catalog(directory, groups):
    with (directory / 'screen-index.tsv').open() as stream:
        rows = csv.DictReader(stream, delimiter='\t')
        members = defaultdict(dict)
        for row in rows:
            name = row['state_id']
            if name.rpartition('-row-')[0] in groups:
                members[row['archive']][row['png_path']] = (name, row['rgb_sha256'])
    for filename, wanted in members.items():
        with tarfile.open(directory / filename) as archive:
            for member in archive:
                if member.name in wanted:
                    name, digest = wanted.pop(member.name)
                    with archive.extractfile(member) as stream:
                        yield name, stream.read(), digest
        if wanted:
            raise ValueError(f'Missing menu screens in {filename}: {sorted(wanted)}')


def longest_blank_run(image):
    longest = consecutive = 0
    for y in range(20, 158):  # The lower LCD belongs to the ordinary menu footer.
        line = image.crop((0, y, 240, y + 1))
        flat = Image.new('RGB', line.size, line.getpixel((0, 0)))
        consecutive = 0 if ImageChops.difference(line, flat).getbbox() else consecutive + 1
        longest = max(longest, consecutive)
    return longest


def measure(name, image, group_rows, original):
    group, _, suffix = name.rpartition('-row-')
    selected = int(suffix)
    count = max(group_rows) + 1
    visible = min(6, count)
    first = max(0, selected - 5)
    pitch = 23 if original else (6 * 23) // visible
    extra = (pitch - 23) // 2
    panels = []
    for index in range(visible):
        absolute_row = first + index
        if absolute_row not in group_rows:
            continue  # An unselectable section heading has no interactive surface.
        y = 20 + index * pitch + (pitch - 23 - extra) // 2
        ink = list_row_ink(image, y, 19 + extra)
        panel = measure_ink(ink, absolute_row)
        outline = max(image.getpixel((120, y + 1)))
        if outline < 32 or (outline > 110) != (absolute_row == selected):
            panel['status'] = 'FAIL'
            panel['surfaceOrSelectionMismatch'] = True
        panels.append(panel)
    blank = longest_blank_run(image)
    return {'id': name, 'panels': panels, 'uniformBodyScanlines': blank,
            'pass': all(p['status'] == 'PASS' for p in panels) and 100 * blank <= 12 * 240}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument('--catalog', type=Path, help='directory containing screen-index.tsv')
    source.add_argument('--captures', type=Path, help='native geometry walker PNG directory')
    parser.add_argument('--original-layout', action='store_true', help='measure historical 23px row spacing')
    parser.add_argument('--output', type=Path, help='write all measured panels without truncation')
    args = parser.parse_args()
    if args.catalog:
        with (args.catalog / 'screen-index.tsv').open() as stream:
            groups = selected_states(r['state_id'] for r in csv.DictReader(stream, delimiter='\t'))
        images = read_catalog(args.catalog, groups)
    else:
        files = sorted(args.captures.glob('*.png'))
        groups = selected_states(path.stem for path in files)
        images = ((p.stem, p.read_bytes(), None) for p in files if p.stem.rpartition('-row-')[0] in groups)
    records = []
    for name, data, digest in images:
        with Image.open(io.BytesIO(data)) as source_image:
            if source_image.format != 'PNG' or source_image.size != (240, 240):
                raise ValueError(f'Expected a native PNG at {name}')
            image = source_image.convert('RGB')
        if digest and hashlib.sha256(image.tobytes()).hexdigest() != digest:
            raise ValueError(f'Unexpected framebuffer pixels at {name}')
        group = name.rpartition('-row-')[0]
        records.append(measure(name, image, groups[group], args.original_layout))
    panels = [p for record in records for p in record['panels']]
    bad = [r for r in records if not r['pass']]
    panel_failures = sum(p['status'] == 'FAIL' for p in panels)
    if len(records) < 378 or len(panels) < 2088:
        raise ValueError(f'Incomplete ordinary menu inventory: {len(records)} frames, {len(panels)} panels')
    if args.output:
        args.output.write_text(json.dumps(sorted(records, key=lambda r: r['id']), separators=(',', ':')) + '\n')
    for record in bad:
        failed_rows = [(p['row'], p['coveragePercent']) for p in record['panels'] if p['status'] == 'FAIL']
        print(f"{record['id']} failing rows {failed_rows}, uniform scanlines {record['uniformBodyScanlines']}")
    print(f'Ordinary menu layout: {len(records)} frames, {len(panels)} slabs, '
          f'{len(bad)} failing frames, {panel_failures} failing slabs')
    return bool(bad)


if __name__ == '__main__':
    raise SystemExit(main())
