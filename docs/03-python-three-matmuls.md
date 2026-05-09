# 3 — Three matmul implementations

**Goal**: implement `matmul_entrywise`, `matmul_columnwise`, and `matmul_outerproduct`, and verify they agree.

This is the heart of the exercise. The linear algebra companion proves (Exercise 6.4) that these three views compute the same thing. You will turn that proof into running code.

## The three views

Let $A \in \mathbb{R}^{m \times k}$ and $B \in \mathbb{R}^{k \times n}$. The product $C = AB \in \mathbb{R}^{m \times n}$ can be computed in three ways:

**View 1 — entry-wise (textbook)**

$$C_{ij} = \sum_{p=1}^{k} A_{ip} B_{pj}$$

A triple loop. Each output entry is a dot product of one row of $A$ with one column of $B$.

**View 2 — column-by-column**

$$C_{:,j} = A \cdot B_{:,j} \qquad \text{for each } j = 1, \ldots, n$$

Each column of $C$ is the matrix-vector product of $A$ with one column of $B$. Internally this is still $O(mkn)$, but the inner work — matrix times vector — emphasises the *linear-map* view.

**View 3 — sum of outer products**

$$C = \sum_{p=1}^{k} A_{:,p} \, B_{p,:}^{\top}$$

Each term is a rank-1 $m \times n$ matrix. The sum has rank up to $k$. Same total work, very different internal structure.

## What to fill in

In [`python/matrix/matrix.py`](../python/matrix/matrix.py):

- `matmul_entrywise` — TODO 3.1
- `matmul_columnwise` — TODO 3.2
- `matmul_outerproduct` — TODO 3.3

Each takes another `Matrix` and returns a new `Matrix`. None of them may call the others.

## Verify

```bash
cd python && uv run pytest tests/test_matrix.py -v -k matmul
```

You're done when:

- `test_each_variant_matches_known_product` passes for all three.
- `test_three_variants_agree_random` passes — this is the experimental restatement of Exercise 6.4.
- `test_each_variant_agrees_with_numpy` passes — your variants match `numpy.matmul`.
- `test_associativity_with_random_matrices` and `test_matrix_vector_via_matmul` pass.
- `test_softmax_dotproduct_reference_weights` passes — the cat-sat-on-it scores match the lecture.

## Hints

<details markdown="1">
<summary>How do I write the column-by-column variant cleanly?</summary>

For each column `j` of `other`, build the column vector, do a matrix-vector product, then write the result into column `j` of `out`. Pseudocode:

```python
out = Matrix.zeros(self.rows, other.cols)
for j in range(other.cols):
    col = other.get_column(j)                # list of length k
    for i in range(self.rows):
        s = 0.0
        for p in range(self.cols):
            s += self[i, p] * col[p]
        out[i, j] = s
return out
```

That's a triple loop with a slightly nicer middle layer. Same flop count as `entrywise`, slightly different memory pattern.

</details>

<details markdown="1">
<summary>The outer-product variant blew up my memory. What went wrong?</summary>

You probably stored every rank-1 intermediate as its own `Matrix` and then summed them. That uses `k` times more memory than necessary. Accumulate in place:

```python
out = Matrix.zeros(self.rows, other.cols)
for p in range(self.cols):
    col = self.get_column(p)        # length self.rows
    row = other.get_row(p)          # length other.cols
    for i in range(self.rows):
        for j in range(other.cols):
            out[i, j] += col[i] * row[j]
return out
```

This is the only sensible implementation. You'll be glad you wrote it this way when you measure memory in chapter 4.

</details>

<details markdown="1">
<summary>Why doesn't column-wise read column-major?</summary>

It does, kind of — but only the column extraction does. The work on each column is the same triple loop as entrywise. The point of view 2 isn't speed in pure Python; it's that you can describe matmul as "apply this linear map to a stack of input vectors". This view is how multi-head attention is often explained.

</details>

→ Continue with [04 — Benchmarking Python](04-python-benchmarking.md)
