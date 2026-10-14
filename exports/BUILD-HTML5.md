# Finite Yield HTML5 export

The finished submission is `Finite-Yield-HTML5.zip`. It contains exactly four
runtime files, with `index.html` at the ZIP root. The ZIP is about 183 KB, below the 500 MB limit.

The export uses a 1280×720 canvas and a responsive 16:9 page, as required by the
[jam instructions](https://docs.google.com/document/d/1G8v71OsidWXZhNe7QYW0On3yt3ioNXWyJT4_VU9CZzc/edit?tab=t.0).
Only the export copy's dimensions changed; original source and assets were
verified unchanged. Emscripten 6.0.12 built the game with size optimization and
link-time optimization. JavaScript syntax, the WebAssembly module, file
references, and ZIP integrity passed checks. Browser gameplay was not tested. The reported startup memory error is under
investigation; the export now uses a 1 MiB stack, stack-overflow checks, runtime
assertions, and named function traces. All seven GIFs passed a separate
WebAssembly decoding test, but the full browser startup still needs a retest.

## Preview the finished export

From the original project directory:

```sh
python3 -m http.server 8000 --bind 127.0.0.1 --directory exports/finite-yield-html5
```

Open `http://127.0.0.1:8000` manually in your browser. Press Control+C in the
terminal to stop the server. Use a local HTTP server rather than opening
`index.html` with a `file://` URL, because the game fetches its WebAssembly and
asset files. Upload the ZIP as the HTML5 game submission.

## Rebuild after editing the game

Run these commands from the original project directory. They copy current
sources into a new temporary directory and leave the original project intact.
The SDK installed for this export is temporary; if that folder no longer
exists, follow the official [Emscripten SDK installation instructions](https://emscripten.org/docs/getting_started/downloads.html)
and source your installed SDK's `emsdk_env.sh` instead.

```sh
source /private/tmp/emsdk-main/emsdk_env.sh
FY_EXPORT_STAGE=$(mktemp -d /private/tmp/finite-yield-rebuild.XXXXXX)
export FY_EXPORT_STAGE
mkdir -p "$FY_EXPORT_STAGE/project/assets"
cp -R src "$FY_EXPORT_STAGE/project/"
cp CMakeLists.txt "$FY_EXPORT_STAGE/project/"
cp assets/Plains.gif assets/PinkTree.gif assets/Mountain.gif \
   assets/LiveFactory.gif assets/DeadFactory.gif \
   assets/LiveOpenPitMine.gif assets/DeadOpenPitMine.gif \
   "$FY_EXPORT_STAGE/project/assets/"
python3 - <<'PY'
import os, re
from pathlib import Path
stage = Path(os.environ['FY_EXPORT_STAGE'])
config = stage / 'project/src/app/Config.h'
text = config.read_text()
text = re.sub(r'(windowWidth\s*=\s*)\d+', r'\g<1>1280', text)
text = re.sub(r'(windowHeight\s*=\s*)\d+', r'\g<1>720', text)
config.write_text(text)
shell = Path('exports/finite-yield-html5/index.html').read_text()
shell = shell.replace('<script async src="finite_yield.js"></script>', '{{{ SCRIPT }}}')
(stage / 'shell.html').write_text(shell)
PY
emcmake cmake -S "$FY_EXPORT_STAGE/project" -B "$FY_EXPORT_STAGE/build" \
  -DPLATFORM=Web -DCMAKE_BUILD_TYPE=MinSizeRel -DBUILD_TESTING=OFF \
  -DFETCHCONTENT_SOURCE_DIR_RAYLIB="$PWD/build/_deps/raylib-src" \
  -DCMAKE_C_FLAGS=-flto -DCMAKE_CXX_FLAGS=-flto \
  "-DCMAKE_EXE_LINKER_FLAGS=-flto -sSTACK_SIZE=1048576 -sASSERTIONS=2 -sSTACK_OVERFLOW_CHECK=2 --profiling-funcs --shell-file $FY_EXPORT_STAGE/shell.html"
cmake --build "$FY_EXPORT_STAGE/build" -j 4
```

This generates `finite_yield.html`, `finite_yield.js`, `finite_yield.wasm`, and
`finite_yield.data` in the temporary build directory. The cached raylib source
path above assumes the existing desktop `build/` directory remains present.
If it does not, omit `-DFETCHCONTENT_SOURCE_DIR_RAYLIB=...` to let CMake download
raylib; that requires internet access.

Package the rebuilt runtime files as a new ZIP in the temporary directory:

```sh
python3 - <<'PY'
import os, shutil, zipfile
from pathlib import Path
stage = Path(os.environ['FY_EXPORT_STAGE'])
package = stage / 'html5'
package.mkdir()
html = (stage / 'shell.html').read_text().replace(
    '{{{ SCRIPT }}}', '<script async src="finite_yield.js"></script>')
(package / 'index.html').write_text(html)
for name in ('finite_yield.js', 'finite_yield.wasm', 'finite_yield.data'):
    shutil.copy2(stage / 'build' / name, package / name)
archive = stage / 'Finite-Yield-HTML5.zip'
with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as z:
    for path in sorted(package.iterdir()):
        z.write(path, path.name)
print(archive)
PY
```

Keep all four runtime files together. Source code, SDK files, build caches,
unused assets, documentation, and native executables do not belong in the ZIP.
