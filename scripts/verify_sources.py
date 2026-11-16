#!/usr/bin/env python3
"""Check immutable user-supplied references byte for byte."""
import hashlib
import json
from pathlib import Path
root = Path(__file__).resolve().parents[1]
entries = json.loads((root / 'docs/source-manifest.json').read_text())
for entry in entries:
    data = (root / entry['path']).read_bytes()
    if len(data) != entry['bytes'] or hashlib.sha256(data).hexdigest() != entry['sha256']:
        raise SystemExit('Reference modified: ' + entry['path'])
print(f'{len(entries)} original references verified (SHA-256)')
