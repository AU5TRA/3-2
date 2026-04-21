# Volcanic SARSA: Step-by-Step Explanation

<img width="1386" height="510" alt="image" src="https://github.com/user-attachments/assets/99a73e97-bb6e-4f14-94d4-b25da564dc43" />

## 1. Settings in the screenshot

The simulator uses:

- `moveReward = 0`
- `passReward = 20`
- `volcanoReward = -50`
- `slipProb = 0`
- `discount = 1`
- `numEpisodes = 1`
- `eta = 0.5`
- `epsilon = 1`
- `rl = "sarsa"`

### Meaning
- Ordinary moves give reward **0**
- Entering the green goal gives a positive terminal reward (the screenshot shows the final received reward is **2**)
- Entering volcano would give **-50**, but it does not happen in this episode
- `slipProb = 0` means actions execute exactly
- `discount = 1` means no discounting
- `epsilon = 1` means the behavior is fully exploratory
- `rl = "sarsa"` means updates use the SARSA target
- 
$$
r + \gamma \hat Q(s',a')
$$

---

## 2. The sampled episode

From the trace on the right, the visited transitions are:

| Step | State | Action | Reward | Next state |
|---|---|---|---:|---|
| 1 | $(2,1)$ | N | 0 | $(1,1)$ |
| 2 | $(1,1)$ | W | 0 | $(1,1)$ |
| 3 | $(1,1)$ | W | 0 | $(1,1)$ |
| 4 | $(1,1)$ | N | 0 | $(1,1)$ |
| 5 | $(1,1)$ | S | 0 | $(2,1)$ |
| 6 | $(2,1)$ | W | 0 | $(2,1)$ |
| 7 | $(2,1)$ | S | 2 | $(3,1)$ terminal |

So the total utility shown is:

$$
U = 0+0+0+0+0+0+2 = 2
$$

---

## 3. SARSA update rule

For each observed quintuple

$$
(s,a,r,s',a')
$$

SARSA updates:

$$
\hat Q(s,a) \leftarrow (1-\eta)\hat Q(s,a) + \eta \[r + \gamma \hat Q(s',a')]
$$

With the settings here:

- $\eta = 0.5$
- $\gamma = 1$

so the rule becomes

$$
\hat Q(s,a) \leftarrow 0.5\,\hat Q(s,a) + 0.5\,\[r + \hat Q(s',a')]
$$

For the terminal step, there is no next action, so the target is just the terminal reward.

---

## 4. Initial values

At the start, all values are:

$$
\hat Q(s,a)=0
$$

for every state-action pair.

---

## 5. To match the screenshot, propagate from the end of the episode backward

The displayed numbers match a **backward propagation of the SARSA targets along the stored episode**.

This is the cleanest way to reconstruct the values in the figure.

---

## 6. Step 7: update $((2,1),S)$

The final step is:

$$
((2,1),S,2,(3,1))
$$

Since $(3,1)$ is terminal, the target is just: 2

So:

$$
\hat Q((2,1),S) \leftarrow 0.5(0) + 0.5(2) = 1
$$

So the south action at $(2,1)$ becomes: 1


This matches the **1** shown in the bottom triangle of state $(2,1)$.

---

## 7. Step 6: update $((2,1),W)$

Now look one step earlier:

$$
((2,1),W,0,(2,1),S)
$$

The next chosen action is $S$ at $(2,1)$, whose current value is now 1.

So the target is:

$$
0 + \hat Q((2,1),S) = 1
$$

Hence:

$$
\hat Q((2,1),W) \leftarrow 0.5(0) + 0.5(1) = 0.5
$$


This matches the **0.5** shown in the left triangle of $(2,1)$.

---

## 8. Step 5: update $((1,1),S)$

Now consider:

$$
((1,1),S,0,(2,1),W)
$$

The next chosen action is $W$ at $(2,1)$, whose value is now 0.5.

So the target is:

$$
0 + 0.5 = 0.5
$$

Update:

$$
\hat Q((1,1),S) \leftarrow 0.5(0) + 0.5(0.5) = 0.25 ≈ 0.3
$$



So the **0.3** in the bottom triangle of $(1,1)$ comes from rounding $0.25$.

---

## 9. Step 4: update $((1,1),N)$

Next:

$$
((1,1),N,0,(1,1),S)
$$

The next chosen action is $S$ at $(1,1)$, whose value is now 0.25.

So the target is:

$$
0 + 0.25 = 0.25
$$

Update:

$$
\hat Q((1,1),N) \leftarrow 0.5(0) + 0.5(0.25) = 0.125 ≈ 0.1
$$


---

## 10. Step 3: first update of $((1,1),W)$

Now:

$$
((1,1),W,0,(1,1),N)
$$

The next chosen action is $N$ at $(1,1)$, whose value is now 0.125.

So the target is:

$$
0 + 0.125 = 0.125
$$

Update:

$$
\hat Q((1,1),W) \leftarrow 0.5(0) + 0.5(0.125) = 0.0625
$$


---

## 11. Step 2: second update of $((1,1),W)$

The action $W$ at $(1,1)$ appears again one step earlier:

$$
((1,1),W,0,(1,1),W)
$$

Now the next chosen action is again $W$ at $(1,1)$, whose current value is 0.0625.

So the target is:

$$
0 + 0.0625 = 0.0625
$$

Update:

$$
\hat Q((1,1),W) \leftarrow 0.5(0.0625) + 0.5(0.0625) = 0.0625 ≈ 0.1
$$



That matches the **0.1** shown in the left triangle of $(1,1)$.

---

## 12. Step 1: update $((2,1),N)$

Finally, the first step is:

$$
((2,1),N,0,(1,1),W)
$$

The next chosen action is $W$ at $(1,1)$, whose value is now 0.0625.

So the target is:

$$
0 + 0.0625 = 0.0625
$$

Update:

$$
\hat Q((2,1),N) \leftarrow 0.5(0) + 0.5(0.0625) = 0.03125 ≈ 0
$$




That is why the top triangle at $(2,1)$ still looks like 0 in the screenshot.

---

## 13. Final values reconstructed from the episode

### State $(2,1)$
- $Q((2,1),N)=0.03125 ≈ 0.0$
- $Q((2,1),W)=0.5$
- $Q((2,1),S)=1$

### State $(1,1)$
- $Q((1,1),N)=0.125 ≈ 0.1$
- $Q((1,1),W)=0.0625 ≈ 0.1$
- $Q((1,1),S)=0.25 ≈ 0.3$

All other shown actions remain 0 because they were never updated.

---

## 14. Why these values make sense

This is exactly how SARSA should behave:

- the action that directly reaches the good terminal state becomes best first
- one step earlier, actions get a smaller positive value
- even earlier actions get weaker positive values
- the reward is propagated backward **one step at a time** through the actual action sequence

So SARSA does **bootstrapping along the sampled path**.
instead of the value of the **actual next action**.

SARSA specifically uses the next action that really occurred in the sampled trajectory.

That is why SARSA is **on-policy**.

