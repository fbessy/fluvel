#!/usr/bin/env bash
# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

cd "$(dirname "$0")/../"

python3 -m http.server 8000
