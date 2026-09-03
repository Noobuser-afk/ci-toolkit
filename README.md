# corebench

A tiny CPU core exercise utility for ARM64 SBCs. Runs a deterministic
workload across N worker threads, verifies the checksum, and prints a
one-line summary. Useful as a smoke test after flashing a new board
image, or as a warm-up job for CI runners.

## Build

```sh
make            # builds ./corebench
make test       # runs the self-check suite
```

## Usage

```sh
./corebench --threads 4 --iterations 200000 --json
```

Output (JSON mode):

```json
{"tool":"corebench","threads":4,"iterations":200000,"elapsed_s":1.42,"checksum":"a3f19c"}
```

Exit code 0 if every worker's checksum matches the reference vector,
non-zero otherwise.

## Why

New SBC images are sometimes subtly broken (bad throttling, half-wired
RAM channels). `corebench` gives a 30-second pass/fail signal without
needing stress-ng and its dependency tree. It is intentionally
single-file C with no external dependencies so it builds anywhere,
including minimal CI containers.

## License

MIT
