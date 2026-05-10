# 9 — Final comparison

**Goal**: produce one plot that tells the whole story, and turn the numbers you measured into a few sentences you can defend.

## The single plot

You should now have CSVs for:

- Pure Python: `entrywise`, `columnwise`, `outerproduct`, `numpy` — `results/python.csv`
- C++ at three opt levels: `matmul_entrywise_*`, `matmul_columnwise_*`, `matmul_outerproduct_*`, `matmul_eigen_*` — `results/cpp-O0.csv`, `results/cpp-O2.csv`, `results/cpp-O3.csv`

Re-run the plot:

```bash
cd python && uv run python bench/plot_results.py
```

The plot is busy. That's fine — but reduce it to five lines by editing `plot_results.py` to filter to the variants you care about, or by exporting the CSV to a spreadsheet and plotting there. The five lines you want:

1. Best pure-Python variant (whichever was fastest of your three)
2. NumPy
3. Brute-force C++ at `-O3` (`matmul_entrywise`)
4. Cache-friendly C++ at `-O3` (`matmul_outerproduct`)
5. Eigen at `-O3`

Save as `results/comparison.png`.

## Things worth writing down

There is no graded deliverable, but the chapters have raised five questions worth answering for yourself — explaining the gap forces you to commit to a model of *why*, which a passive read of these docs doesn't give you. [`your_results.md`](../your_results.md) has those five questions stubbed out as an optional template; or use a notebook, or anywhere else.

A few hints on tone if you do write them down:

- Be specific. "NumPy was much faster" is bad; "NumPy was 312× faster than my best Python at n = 256" is good.
- Use your own numbers. Don't paste in the canonical-looking values from these docs.
- For the production-decision question, the right answer depends on context. *"I'd write `A @ B` in NumPy"* is fine if you justify it. *"I'd hand-code a cache-friendly outer-product loop"* is also fine if you justify it (e.g., embedded target with no Python). What's not fine is "the fastest one" without saying which.

## What's the takeaway

You started with a black-box `A @ B` call. You now have a four-line answer to "why is NumPy fast":

1. It pushes the per-element loop out of the interpreter.
2. It uses a packed `double` array, not boxed Python floats.
3. It dispatches to BLAS for sizes where BLAS wins.
4. It uses SIMD where SIMD wins.

Eigen does the same things in C++. The reason your `-O3` `matmul_outerproduct` was already within a single-digit factor of Eigen is that the C++ compiler does a lot of (1) and (3) for you automatically — the gap between Python and NumPy is wider because Python imposes overhead the C++ compiler can erase.

This is the lesson the rest of the course will build on: every time you write a `model.forward(x)` or `tensor @ tensor`, *somebody* did this work. Now you know roughly how much.

## Wrap up

Commit and push your results so the CI runs:

```bash
git add results/
git add your_results.md   # only if you used the template
git commit -m "D5 deep-dive — benchmark results"
git push
```

The CI workflows on the repo will run `pytest` and `ctest` on every push. If they're green, the technical work is done. Whether and where you write the reflection is up to you.

---

**Optional extensions** (not graded; for the very curious):

- Implement `matmul_blocked` — break the matrices into `64×64` blocks, multiply blocks, accumulate. Beat your own `matmul_outerproduct`.
- Implement matmul with OpenMP — `#pragma omp parallel for` on the outer loop. Compare to single-threaded.
- Try AVX2 intrinsics in the inner loop. Beat your own `-O3` compiler.
- Compile with `-march=native` and re-benchmark. Most students see another 20–40% speedup.

These are the kinds of optimisations the BLAS authors spent years on. You won't beat them, but you will *understand* what they did.
