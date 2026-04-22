# Volcanic Q-Learning : Step-by-Step Explanation
<img width="1234" height="540" alt="image" src="https://github.com/user-attachments/assets/aa74666a-8c94-4c2f-85c8-b1e5692396b0" />

## 1. Settings in the screenshot

The simulator is using:

- `moveReward = 0`
- `passReward = 20`
- `volcanoReward = -50`
- `slipProb = 0`
- `discount = 1`
- `numEpisodes = 1`
- `eta = 0.5`
- `epsilon = 1`
- `rl = "q"`

### Meaning
- Ordinary moves give reward **0**
- Reaching the green terminal state gives a positive terminal reward
- Entering volcano would give **-50**, but it does not happen in this episode
- `slipProb = 0` means actions execute exactly as chosen
- `discount = 1` means no discounting
- `epsilon = 1` means the behavior is fully exploratory
- `rl = "q"` means the algorithm is **Q-learning**

---

## 2. The sampled episode

From the trace on the right, the visited transitions are:

| Step | State | Action | Reward | Next state |
|---|---|---:|---|
| 1 | $(2,1)$ | E | 0 | $(2,2)$ |
| 2 | $(2,2)$ | W | 0 | $(2,1)$ |
| 3 | $(2,1)$ | N | 0 | $(1,1)$ |
| 4 | $(1,1)$ | S | 0 | $(2,1)$ |
| 5 | $(2,1)$ | S | 2 | $(3,1)$ terminal |

So the total utility shown is:

$$
U = 0 + 0 + 0 + 0 + 2 = 2
$$

---

## 3. Q-learning update rule

For each observed transition

$$
(s,a,r,s')
$$

Q-learning updates

$$
\hat Q(s,a) \leftarrow (1-\eta)\hat Q(s,a) + \eta \left\[r + \gamma \max_{a'} \hat Q(s',a')\right]
$$

With the simulator settings:

- $\eta = 0.5$
- $\gamma = 1$

this becomes

$$
\hat Q(s,a) \leftarrow 0.5\,\hat Q(s,a) + 0.5\left\[r + \max_{a'} \hat Q(s',a')\right]
$$

Equivalently,

$$
\hat Q(s,a) \leftarrow \hat Q(s,a) + 0.5\left(r + \max_{a'} \hat Q(s',a') - \hat Q(s,a)\right)
$$

---

## 4. Initial values

Initially, all Q-values are assumed to be

$$
\hat Q(s,a)=0
$$

for every state-action pair.

---

## 5. Main Q-learning idea here

Unlike SARSA, Q-learning does **not** use the next action that was actually taken.

Instead, it uses

$$
\max_{a'} \hat Q(s',a')
$$

So from the next state, Q-learning asks:

> “What is the best action value I currently know at that next state?”

That is why Q-learning learns toward the **optimal** value function.

---

## 6. Reconstruct the values from the end backward

The screenshot values match cleanly if we propagate the updates from the end of the sampled episode backward.

This is also the clearest way to understand how the positive reward spreads through the grid.

---

## 7. Step 5: update $((2,1),S)$

The last transition is

$$
((2,1),S,2,(3,1))
$$

Since $(3,1)$ is terminal, there is no future continuation value, so

$$
\max_{a'} \hat Q((3,1),a') = 0
$$

Thus the target is

$$
2 + 0 = 2
$$

Apply the update:

$$
\hat Q((2,1),S) = 0.5(0) + 0.5(2) = 1
$$

So

$$
\boxed{\hat Q((2,1),S)=1}
$$

This matches the **1** in the bottom triangle of state $(2,1)$.

---

## 8. Step 4: update $((1,1),S)$

Now consider

$$
((1,1),S,0,(2,1))
$$

Q-learning uses the **best** action value at the next state $(2,1)$.

At this point, the best value at $(2,1)$ is

$$
\max_{a'} \hat Q((2,1),a') = 1
$$

because the south action at $(2,1)$ already has value 1.

So the target is

$$
0 + 1 = 1
$$

Update:

$$
\hat Q((1,1),S) = 0.5(0) + 0.5(1) = 0.5
$$

So

$$
\boxed{\hat Q((1,1),S)=0.5}
$$

This matches the **0.5** shown at state $(1,1)$.

---

## 9. Step 3: update $((2,1),N)$

Now take

$$
((2,1),N,0,(1,1))
$$

Again Q-learning looks at the best action from the next state $(1,1)$.

We already learned

$$
\hat Q((1,1),S)=0.5
$$

and the others there are still 0, so

$$
\max_{a'} \hat Q((1,1),a') = 0.5
$$

Thus the target is

$$
0 + 0.5 = 0.5
$$

Update:

$$
\hat Q((2,1),N) = 0.5(0) + 0.5(0.5) = 0.25
$$

So

$$
\boxed{\hat Q((2,1),N)=0.25}
$$

Rounded to one decimal place, this is displayed as

$$
\boxed{0.3}
$$

That matches the **0.3** shown in the left-side upper cell.

---

## 10. Step 2: update $((2,2),W)$

Now consider

$$
((2,2),W,0,(2,1))
$$

At the next state $(2,1)$, the best currently known action value is still

$$
\max_{a'} \hat Q((2,1),a') = 1
$$

because the south action at $(2,1)$ has value 1.

So the target is

$$
0 + 1 = 1
$$

Update:

$$
\hat Q((2,2),W) = 0.5(0) + 0.5(1) = 0.5
$$

So

$$
\boxed{\hat Q((2,2),W)=0.5}
$$

This matches the **0.5** shown at state $(2,2)$.

---

## 11. Step 1: update $((2,1),E)$

Finally, look at the first step

$$
((2,1),E,0,(2,2))
$$

At state $(2,2)$, the best action value is now

$$
\max_{a'} \hat Q((2,2),a') = 0.5
$$

because the west action at $(2,2)$ has value 0.5.

So the target is

$$
0 + 0.5 = 0.5
$$

Update:

$$
\hat Q((2,1),E) = 0.5(0) + 0.5(0.5) = 0.25
$$

So

$$
\boxed{\hat Q((2,1),E)=0.25}
$$

Rounded to one decimal place, this is

$$
\boxed{0.3}
$$

That matches the **0.3** shown in the right triangle of state $(2,1)$.

---

## 12. Final reconstructed values

### State $(2,1)$
- $Q((2,1),E)=0.25 \approx 0.3$
- $Q((2,1),N)=0.25 \approx 0.3$
- $Q((2,1),S)=1$

### State $(1,1)$
- $Q((1,1),S)=0.5$

### State $(2,2)$
- $Q((2,2),W)=0.5$

All other actions remain 0 because they were never updated.

---

## 13. Why these values make sense

This picture is showing **backward propagation of a good terminal reward** under Q-learning.

The good reward of **2** reaches the action directly before the terminal state first:

$$
Q((2,1),S)=1
$$

Then Q-learning spreads that value backward through states that can lead to $(2,1)$:

- one step back gives values of **0.5**
- another step back gives values of **0.25**, displayed as **0.3**

So the farther away an action is from the terminal reward, the weaker the backed-up value after only one episode.

---

## 14. Why this differs from SARSA

This is the most important conceptual point.

For the transition from $(2,1)$ to $(2,2)$, Q-learning uses

$$
\max_{a'} Q((2,2),a')
$$

not the action that was actually taken next.

So even though the trajectory is exploratory, Q-learning assumes that from the next state onward we can choose the **best known action**.

That is why it is **off-policy**.

SARSA would instead use the value of the actual next sampled action.

---

## 15. Why this differs from Monte Carlo

Monte Carlo would use the full episode return, which here is 2, for every visited state-action pair.

Q-learning does not do that.

It only backs the reward up **one step at a time** through the max backup:

$$
r + \max_{a'} Q(s',a')
$$

So the values decay gradually as we move farther from the terminal reward.


So the screenshot is showing the positive terminal reward of **2** being propagated backward using the Q-learning max backup.
