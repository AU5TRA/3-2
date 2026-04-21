# Volcanic Model-Free Monte Carlo: Step-by-Step Explanation
<img width="1438" height="747" alt="image" src="https://github.com/user-attachments/assets/caa84517-727f-4dde-8d6f-fd1d02e92150" />

## 1. Environment settings in the screenshot

From the simulator:

- `moveReward = 0`
- `passReward = 20`
- `volcanoReward = -50`
- `slipProb = 0`
- `discount = 1`
- `numEpisodes = 1`
- `eta = 0.5`
- `epsilon = 1`
- `rl = "monte-carlo"`

### What these mean
- Every normal move gives reward **0**
- Reaching the green goal gives **+20**
- Falling into volcano gives **-50**
- `slipProb = 0` means actions work exactly as chosen
- `discount = 1` means no future reward discounting
- `epsilon = 1` means the agent is exploring randomly
- Since `rl = "monte-carlo"`, values are updated from the **full return of the episode**

---

## 2. The sampled episode

The simulator shows the following trajectory:

| Step | Action | Reward | New state |
|---|---|---:|---|
| start | — | — | $(2,1)$ |
| 1 | W | 0 | $(2,1)$ |
| 2 | W | 0 | $(2,1)$ |
| 3 | N | 0 | $(1,1)$ |
| 4 | E | 0 | $(1,2)$ |
| 5 | S | 0 | $(2,2)$ |
| 6 | S | 0 | $(3,2)$ |
| 7 | E | 0 | $(3,3)$ |
| 8 | N | -50 | $(2,3)$ |

The episode ends by entering the volcano at $(2,3)$.

So the total utility shown at the bottom is:

$$
U = 0 + 0 + 0 + 0 + 0 + 0 + 0 + (-50) = -50
$$

Because $\gamma = 1$, there is no discounting, so the total return is just **-50**.

---

## 3. Why all updated Monte Carlo targets are -50

In Monte Carlo, the value of a visited state-action pair is updated using the **return from that point to the end of the episode**.

Here, every intermediate reward before the last step is **0**, and the episode always ends with **-50**.

So for every visited step, the return from that time onward is:

$$
u_t = -50
$$

That is why every visited action in the path gets pushed toward **-50**.

---

## 4. Initial values

The simulator appears to start all action-values at:

$$
\hat{Q}(s,a) = 0
$$

for every state-action pair.

Unvisited actions stay at **0**.

Visited actions are updated.

---

## 5. Monte Carlo update rule used here

Because the simulator uses a fixed step size `eta = 0.5`, each update is:

$$
\hat{Q}(s,a) \leftarrow (1-\eta)\hat{Q}(s,a) + \eta u
$$

With $\eta = 0.5$ and $u = -50$:

$$
\hat{Q}(s,a) \leftarrow 0.5\hat{Q}(s,a) + 0.5(-50)
$$

If the current value is initially 0, then after one visit:

$$
\hat{Q}(s,a) = 0.5(0) + 0.5(-50) = -25
$$

So a state-action pair visited **once** becomes **-25**.

---

## 6. Explaining each value in the picture

The path visits these state-action pairs:

1. $((2,1), W)$
2. $((2,1), W)$ again
3. $((2,1), N)$
4. $((1,1), E)$
5. $((1,2), S)$
6. $((2,2), S)$
7. $((3,2), E)$
8. $((3,3), N)$

Now update them one by one.

### (a) Pair $((2,1),W)$ appears twice
First visit:

$$
\hat{Q}((2,1),W) = 0.5(0) + 0.5(-50) = -25
$$

Second visit:

$$
\hat{Q}((2,1),W) = 0.5(-25) + 0.5(-50) = -37.5
$$

So this is why one triangle shows:

$$
-37.5
$$

This is the **only** value that is not -25, because this same state-action pair occurred twice in the episode.

---

### (b) Pair $((2,1),N)$ appears once

$$
\hat{Q}((2,1),N) = 0.5(0) + 0.5(-50) = -25
$$

---

### (c) Pair $((1,1),E)$ appears once

$$
\hat{Q}((1,1),E) = -25
$$

---

### (d) Pair $((1,2),S)$ appears once

$$
\hat{Q}((1,2),S) = -25
$$

---

### (e) Pair $((2,2),S)$ appears once
$$
\hat{Q}((2,2),S) = -25
$$

---

### (f) Pair $((3,2),E)$ appears once
$$
\hat{Q}((3,2),E) = -25
$$

---

### (g) Pair $((3,3),N)$ appears once
$$
\hat{Q}((3,3),N) = -25
$$

---

## 7. Why many other actions remain 0

All other actions in the grid were **not taken** in this episode.

Monte Carlo only updates from visited experience. So unvisited actions remain at their initial value:

$$
0
$$

That is why many triangles in the grid still show **0**.

---

## 8. Why the episode utility is -50 but most Q-values are -25

This is because the simulator is not replacing values directly by the return.

It is using the step-size update with `eta = 0.5`.

So each single visit only moves halfway from 0 toward -50:

$$
0 \to -25
$$

If the same state-action is seen again with the same bad outcome, it moves further:

$$
-25 \to -37.5
$$

With many repeated visits, it would keep moving closer to -50.

---

## 9. Important observation: this is every-visit style updating

Since the pair $((2,1),W)$ was updated twice and became **-37.5**, the simulator is effectively updating on **each occurrence** of the state-action pair in the episode.

If it were pure **first-visit Monte Carlo**, that pair would have been updated only once and would stay at **-25**.

---
