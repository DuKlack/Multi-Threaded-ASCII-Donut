# Multi-Threaded ASCII Donut (`donutPT.c`)

`donutPT.c` is a parallelized C implementation of Andy Sloane's iconic 3D ASCII rotating donut algorithm. It utilizes **POSIX Threads (`pthread`)** to accelerate frame calculations through domain decomposition across CPU cores.

---

## 🎯 Features

* **Multi-Threaded Rendering:** Parallelizes 3D torus surface projection using POSIX threads.
* **Thread-Local Z-Buffering:** Prevents race conditions by granting each worker thread an isolated depth and character buffer.
* **Depth Composite Engine:** Merges thread buffers after execution via 3D z-buffer depth comparison.
* **Performance Benchmarking:** Measures wall-clock execution time and reports total runtime, average time per frame (ms), and average Frames Per Second (FPS).

---

## 📐 Math & Domain Decomposition

The torus geometry is parameterized using two angles:
* $\theta$ (theta): Sweeps the cross-sectional circle ($0 \to 2\pi$).
* $\phi$ (phi): Sweeps around the center of revolution ($0 \to 2\pi$).

### Thread Workload Partitioning

To split work across multiple threads, the $\theta$ domain ($2\pi$ total radians) is sliced evenly:

$$
\text{Slice Size} = \frac{2\pi}{\text{NUM\_THREADS}}
$$

For `NUM_THREADS = 2`:
* **Thread 0:** Calculates $\theta \in [0, \pi)$
* **Thread 1:** Calculates $\theta \in [\pi, 2\pi)$

---

## 🛠️ Build & Run Instructions

### Prerequisites

* GCC or Clang compiler
* POSIX-compliant platform (Linux, macOS, or WSL on Windows)
* C Math Library (`-lm`) and POSIX Threads (`-lpthread`)

### Compilation

Compile with compiler optimizations (`-O3`) for maximum performance:

```bash
gcc -O3 donutPT.c -o donutPT -lm -lpthread
```

### Execution

Run the binary in your terminal:

```bash
./donutPT
```

> **Tip:** Set your terminal dimension to **80x40** or larger for optimal rendering output.

---

## ⚙️ Configuration Parameters

Key constant parameters inside `donutPT.c` can be tweaked prior to compilation:

| Constant | Default | Description |
| :--- | :--- | :--- |
| `NUM_THREADS` | `2` | Number of worker threads spawned per frame |
| `TOTAL_FRAMES` | `1000` | Target frame count for benchmark run |
| `SCREEN_WIDTH` | `80` | Terminal output column width |
| `SCREEN_HEIGHT` | `40` | Terminal output row height |
| `theta_spacing` | `0.07f` | Angular step size for section circle sweep |
| `phi_spacing` | `0.02f` | Angular step size for revolution sweep |

---

## 🔬 Architectural Overview

```
                 [ Main Thread: render_frame() ]
                               |
       +-----------------------+-----------------------+
       |                                               |
[ Worker Thread 0 ]                             [ Worker Thread 1 ]
  - theta: [0, pi)                                - theta: [pi, 2pi)
  - Computes 3D transform & luminance             - Computes 3D transform & luminance
  - Writes to thread_data[0].zbuffer              - Writes to thread_data[1].zbuffer
       |                                               |
       +-----------------------+-----------------------+
                               |
                    [ pthread_join() ]
                               |
               [ Merge Z-Buffers by Depth ]
                               |
              [ Dump Terminal Output (\x1b[H) ]
```
