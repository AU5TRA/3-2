# Markov Decision Process (MDP)

## 1. Markov Decision Process

A **Markov Decision Process (MDP)** is a mathematical framework for sequential decision-making under uncertainty.

An MDP is typically defined by:

* **States**: the set of all possible states.
* **Start state**: $s_{\text{start}} \in \text{States}$.
* **Actions**: $\text{Actions}(s)$, the set of actions available in state $s$.
* **Transition model**: $T(s,a,s')$, the probability of reaching state $s'$ after taking action $a$ in state $s$.
* **Reward function**: $\text{Reward}(s,a,s')$, the reward received for the transition $(s,a,s')$.
* **Terminal test**: $\text{IsEnd}(s)$, which tells whether $s$ is an end state.
* **Discount factor**: $0 \leq \gamma \leq 1$, which controls how much future rewards matter.

---

## 2. Policy

A **policy** $\pi$ is a mapping from each state to an action:

$$
\pi : s \mapsto a, \qquad a \in \text{Actions}(s).
$$

So, a policy tells the agent what action to take in every state.

---

## 3. Path, Utility, and Randomness

If we follow a policy, the resulting sequence of states is generally **random**, because transitions may be probabilistic.

A sample path looks like:

$$
s_0, a_1, r_1, s_1, a_2, r_2, s_2, \dots
$$

where each step contains:

* the chosen action,
* the reward received,
* the next state.

The **utility** of a path is the discounted sum of rewards:

$$
U = r_1 + \gamma r_2 + \gamma^2 r_3 + \gamma^3 r_4 + \cdots
$$

Since the path is random, the utility is also a **random variable**.

---

## 4. Value of a Policy

The **value** of a policy at a state is the **expected utility** obtained by following that policy from that state onward.

We write:

$$
V_\pi(s)
$$

to mean the expected utility received by following policy $\pi$ starting from state $s$.

So,

$$
V_\pi(s) = \mathbb{E}[U \mid \text{start at } s \text{ and follow } \pi].
$$

---

## 5. Q-value of a Policy

The **Q-value** of a policy is slightly more specific.

$$
Q_\pi(s,a)
$$

means:

* first take action $a$ from state $s$,
* then follow policy $\pi$ afterward,
* and compute the expected utility.

So $V_\pi(s)$ is the value of following the policy immediately, while $Q_\pi(s,a)$ is the value of forcing one action first and then following the policy.

---

## 6. Policy Evaluation

**Policy evaluation** computes the value function $V_\pi$ for a fixed policy $\pi$.

### Iterative policy evaluation

1. Initialize:

$$
V_\pi^{(0)}(s) = 0 \qquad \text{for all states } s.
$$

2. Then repeatedly update:

$$
V_\pi^{(t)}(s)
\leftarrow
\sum_{s'} T(s,\pi(s),s') \left[\text{Reward}(s,\pi(s),s') + \gamma V_\pi^{(t-1)}(s')\right].
$$

This equation says:

* use the action chosen by the policy,
* look at all possible next states $s'$,
* weight each outcome by its transition probability,
* add immediate reward plus discounted future value.

The bracketed part is exactly the policy Q-value for the previous iteration:

$$
Q_\pi^{(t-1)}(s,\pi(s)).
$$

### Time complexity

Suppose:

* there are $S$ states,
* each state has at most $A$ actions,
* each state-action pair has at most $S'$ successor states.

Then policy evaluation takes:

$$
O(t_{\text{PE}} S S')
$$

because for each iteration and each state, we sum over successors of the policy-selected action.

---

## 7. Optimal Value

The **optimal value** of a state is the maximum value achievable by **any** policy.

It is denoted by:

$$
V_{\text{opt}}(s).
$$

That is,

$$
V_{\text{opt}}(s) = \max_\pi V_\pi(s).
$$

This is the best possible expected return starting from state $s$.

---

## 8. Value Iteration

**Value iteration** directly computes the optimal value function.

### Iterative update

1. Initialize:

$$
V_{\text{opt}}^{(0)}(s) = 0 \qquad \text{for all states } s.
$$

2. Repeatedly update:

$$
V_{\text{opt}}^{(t)}(s)
\leftarrow
\max_{a \in \text{Actions}(s)}
\sum_{s'} T(s,a,s')
\left[\text{Reward}(s,a,s') + \gamma V_{\text{opt}}^{(t-1)}(s')\right].
$$

This means:

* try every action available at state $s$,
* compute the expected return for that action,
* keep the maximum.

The inner term is the optimal Q-value at the previous iteration:

$$
Q_{\text{opt}}^{(t-1)}(s,a).
$$

### Time complexity

Since we must try all actions for each state, the time complexity is:

$$
O(t_{\text{VI}} S A S').
$$

---

## 9. Convergence of Value Iteration

Value iteration is an **iterative process**. Under standard conditions, it converges to the correct answer.

A useful theorem states that value iteration converges if either:

* $\gamma < 1$, or
* the MDP state-transition graph is **acyclic**.

So under either of these conditions,

$$
V_{\text{opt}}^{(t)}(s) \to V_{\text{opt}}(s)
$$

as the number of iterations increases.

---

## 10. Big Picture

The overall flow is:

1. Define the MDP: states, actions, transitions, rewards.
2. Define a policy $\pi$.
3. Compute its value using **policy evaluation**.
4. Search for the best possible value using **value iteration**.
5. Use convergence results to justify repeated updates.

---

## 11. Key Intuition

* **Utility** = discounted sum of rewards along one random path.
* **Value** = expected utility from a state.
* **Q-value** = expected utility of taking one action first, then continuing.
* **Policy evaluation** = value of a fixed policy.
* **Value iteration** = best achievable value over all policies.

---

## 12. Summary of Main Equations

### Utility of a path

$$
U = r_1 + \gamma r_2 + \gamma^2 r_3 + \gamma^3 r_4 + \cdots
$$

### Value of a policy

$$
V_\pi(s) = \mathbb{E}[U \mid s, \pi]
$$

### Q-value of a policy

$$
Q_\pi(s,a) = \mathbb{E}[U \mid \text{take } a \text{ at } s, \text{ then follow } \pi]
$$

### Policy evaluation update

$$V_\pi^{(t)}(s) = \sum_{s'} T(s,\pi(s),s') \left[\text{Reward}(s,\pi(s),s') + \gamma V_\pi^{(t-1)}(s')\right]$$

### Value iteration update

$$V_{\text{opt}}^{(t)}(s) = 
\max_{a \in \text{Actions}(s)}
\sum_{s'} T(s,a,s') \left[\text{Reward}(s,a,s') + \gamma  V_{\text{opt}}^{(t-1)}(s')\right]
$$


---

## From MDP to Reinforcement Learning

### MDP (Planning / Offline)
- Known model of the world:
  - Transition: $T(s,a,s')$
  - Reward: $R(s,a,s')$
- Goal: compute optimal policy using planning (e.g., value iteration)
- Think **before acting**

### Reinforcement Learning (Online)
- Model is **unknown**
- Learn by interaction:
  - take action → observe next state + reward
- Gradually improve policy
- Act **while learning**

---

## Model-Based RL (Monte Carlo Estimation)

### Key Idea
- Learn the MDP model from data:
  - Transition: $\hat{T}(s,a,s')$
  - Reward: $\hat{R}(s,a,s')$

### Data (Experience)
- Collected as trajectories:

$$ 
s_0, a_1, r_1, s_1, a_2, r_2, s_2, \dots 
$$

---

### Estimating Transitions
$$
\hat{T}(s,a,s') = \frac{ \text{ ⵌ times } (s,a,s') \text{ occurs}}{\text{ ⵌ times } (s,a) \text{ occurs}}
$$

---

### Estimating Rewards
$$
\hat{R}(s,a,s') = \text{observed } r \text{ in } (s,a,r,s')
$$

---

### Monte Carlo Idea
- Estimate expectations by **averaging over samples**.

---

### Exploration Issue
- Deterministic policy $\pi(s)$ may **miss some (s,a)** pairs.
- Then we cannot estimate their values.

👉 Need **exploration**:
- Use stochastic policy (e.g., randomization)
- Ensure all $(s,a)$ pairs are visited sufficiently

---

### Convergence
- If all $(s,a)$ are explored infinitely often:
  - $\hat{T} \\to T$
  - $\hat{R} \\to R$

---

### Notation
- $\hat{\cdot}$ denotes **estimated quantities** (from data)
- True quantities have no hat (e.g., $T, Q^*$)


---
---

## Model-Free Monte Carlo

### Data (following policy $\pi$)
We collect trajectories:

$$
s_0, a_1, r_1, s_1, a_2, r_2, s_2, \dots, s_n
$$

---

### Recall: Q-value

$$
Q_\pi(s,a) = \mathbb{E}[\text{utility starting at } s,\ \text{take } a,\ \text{then follow } \pi]
$$

---

### Return (Utility)
Define return starting at time $t$:

$$
u_t = r_t + \gamma r_{t+1} + \gamma^2 r_{t+2} + \cdots
$$

Key observation:

$$
Q_\pi(s_{t-1}, a_t) = \mathbb{E}[u_t]
$$

---

### Monte Carlo Estimation
We estimate by averaging observed returns:

$$
\hat{Q}_\pi(s,a) = \text{average of } u_t \text{ over all times where } s_{t-1}=s,\ a_t=a
$$

- No need for transition model $T(s,a,s')$
- No need for reward function $R(s,a,s')$
- Only uses **sampled trajectories**

> Intuition: treat each $(s,a)$ as producing random returns, estimate its expectation by averaging.

---

### First-visit simplification
Often we consider only occurrences where:
- $(s,a)$ does **not appear earlier in the same trajectory**

This simplifies analysis (first-visit MC), but is not strictly required.

---

### Exploration issue
- If policy is deterministic, some $(s,a)$ may **never be visited**
- Then $\hat{Q}_\pi(s,a)$ cannot be estimated

👉 Need **exploration**, e.g.:
- stochastic policy
- $\epsilon$-greedy

---

### On-policy vs Off-policy
- **On-policy**: estimate value of the policy that generates data
- **Off-policy**: estimate value of a different policy

👉 Model-free Monte Carlo is typically **on-policy**, because:
- $\hat{Q}_\pi$ depends on trajectories generated by $\pi$

---

### Incremental (Online) Update
Instead of storing all returns:


$$
\eta = \frac{1}{1 + \text{ⵌ updates to}(s,a)}
$$

$$
\hat{Q}_\pi(s,a) \leftarrow (1-\eta)\hat{Q}_\pi(s,a) + \eta u
$$

- Equivalent to computing running average

---

### Interpolation View
Update can be seen as:
- moving estimate toward new sample $u$

$$
\hat{Q} \leftarrow (1-\eta)\hat{Q} + \eta u
$$

- $\eta$ controls how much new data influences estimate

---

### SGD (Stochastic Gradient) View
Rewrite:

$$
\hat{Q}_\pi(s,a) \leftarrow \hat{Q}_\pi(s,a) - \eta \big(\hat{Q}_\pi(s,a) - u\big)
$$

- Prediction: $\hat{Q}_\pi(s,a)$  
- Target: $u$

👉 Equivalent to minimizing:

$$
(\hat{Q}_\pi(s,a) - u)^2
$$

So MC = **SGD on least-squares regression**.

---

### Practical Note (Non-stationary case)
If policy keeps changing:

- Use slower decay:

$$
\eta \propto \frac{1}{\sqrt{\text{ⵌ updates to}(s,a)}}
$$

- Gives more weight to recent data

---

### Key Takeaways
- Model-free: no $T$ or $R$
- Uses full returns (no bootstrapping)
- Requires complete episodes
- Strong dependence on exploration
- Naturally fits regression / SGD view

---
---

## SARSA

### Core Update Rule
For each transition

$$
(s, a, r, s', a')
$$

SARSA updates:

$$
\hat{Q}_\pi(s,a) \leftarrow (1-\eta)\hat{Q}_\pi(s,a) + \eta\left[r + \gamma \hat{Q}_\pi(s',a')\right]
$$

Equivalent form:

$$
\hat{Q}_\pi(s,a) \leftarrow \hat{Q}_\pi(s,a) + \eta \left[r + \gamma \hat{Q}_\pi(s',a') - \hat{Q}_\pi(s,a)\right]
$$

---

### Why SARSA?
In model-free Monte Carlo, the target was the full return

$$
u = r_t + \gamma r_{t+1} + \gamma^2 r_{t+2} + \cdots
$$

That target is based on a **single sampled episode**. If the episode is long, this can be a noisy estimate of the true value.

SARSA replaces that full-return target with:

$$
r + \gamma \hat{Q}_\pi(s',a')
$$

So the target combines:
- **data**: the immediate reward $r$
- **estimate**: the predicted future value $\hat{Q}_\pi(s',a')$

This is the key idea of **bootstrapping**.

So SARSA trades:
- a little more **bias**
for
- much smaller **variance**
and faster updates

---

### On-Policy Nature
SARSA is an **on-policy** algorithm.

That means:
- it learns the value of the same policy that generates the data
- the next action $a'$ is chosen by the current policy $\pi$

So SARSA estimates:

$$
Q_\pi(s,a)
$$

not directly

$$
Q^*(s,a)
$$

---

#### Compared with Monte Carlo
- Monte Carlo uses complete returns
- SARSA uses one-step lookahead plus current estimate

---

### Intuition
Suppose you take action $a$ in state $s$, receive reward $r$, move to $s'$, and then choose action $a'$.

Instead of asking:

> “What was the total return all the way to the end?”

SARSA asks:

> “I got reward $r$ now, and my current guess for continuing from $(s',a')$ is $\hat{Q}_\pi(s',a')$. So the target should be their sum.”

That makes SARSA more incremental and practical.

---

### Key Takeaways
- SARSA is a **model-free, on-policy, bootstrapping** method.
- Update target:

$$
r + \gamma \hat{Q}_\pi(s',a')
$$

- It can learn **online**, step by step.
- Compared to Monte Carlo:
  - more biased
  - less variance
  - updates earlier
- It learns $Q_\pi$, not directly $Q^*$.


---
---

## Q-Learning

### Motivation
So far:

- **Model-free Monte Carlo** estimates $Q_\pi$
- **SARSA** also estimates $Q_\pi$
- **Model-based value iteration** can estimate optimal values, but it is model-based

This raises the question:

> Can we estimate the **optimal** action-value function $Q_{\text{opt}}$ in a **model-free** way?

Q-learning answers **yes**.

---

### Core Update Rule
For each observed transition

$$
(s, a, r, s')
$$

Q-learning updates:

$$
\hat{Q}_{\text{opt}}(s,a)
\leftarrow
(1-\eta)\hat{Q}_{\text{opt}}(s,a)
+
\eta\left(r + \gamma \hat{V}_{\text{opt}}(s')\right)
$$

where

$$
\hat{V}_{\text{opt}}(s') = \max_{a' \in \mathrm{Actions}(s')} \hat{Q}_{\text{opt}}(s',a')
$$

So the update is usually written as:

$$
\hat{Q}_{\text{opt}}(s,a)
\leftarrow
(1-\eta)\hat{Q}_{\text{opt}}(s,a)
+
\eta\left[
r + \gamma \max_{a' \in \mathrm{Actions}(s')} \hat{Q}_{\text{opt}}(s',a')
\right]
$$

Equivalent form:

$$
\hat{Q}_{\text{opt}}(s,a)
\leftarrow
\hat{Q}_{\text{opt}}(s,a)
+
\eta\left[
r + \gamma \max_{a'} \hat{Q}_{\text{opt}}(s',a') - \hat{Q}_{\text{opt}}(s,a)
\right]
$$

---

### Intuition
Q-learning uses:

- **prediction**: current estimate $\hat{Q}_{\text{opt}}(s,a)$
- **target**:

$$
r + \gamma \max_{a'} \hat{Q}_{\text{opt}}(s',a')
$$

So after taking action $a$ at state $s$ and observing reward $r$ and next state $s'$, it says:

> “The value of $(s,a)$ should move toward the immediate reward plus the best possible future value from $s'$.”

---

### Where Q-learning Comes From
Q-learning can be viewed as a sample-based version of the Bellman optimality equation.

In value iteration, the optimal action-value satisfies:

$$
Q_ {\text{opt}}(s,a)=\sum_{s'} T(s,a,s')
\left \[R(s,a,s') + \gamma V_ {\text{opt}}(s')\right]
$$

with

$$
V_{\text{opt}}(s') = \max_{a'} Q_{\text{opt}}(s',a')
$$

Q-learning modifies this in several ways:

1. We do **not** take expectation over all possible $s'$  
   We only use the **single observed sample** $s'$.

2. We use the **observed reward** $r$ instead of the full reward function $R(s,a,s')$.

3. We do not replace the old estimate fully; we **interpolate** using learning rate $\eta$.

4. We replace true unknown values with current estimates:

$$
V_{\text{opt}}(s') \approx \hat{V}_{\text{opt}}(s')
$$

---

### Key Difference from SARSA
SARSA update:

$$
\hat{Q}_\pi(s,a)
\leftarrow
(1-\eta)\hat{Q}_\pi(s,a)
+
\eta\left[r + \gamma \hat{Q}_\pi(s',a')\right]
$$

Q-learning update:

$$
\hat{Q}_{\text{opt}}(s,a)
\leftarrow
(1-\eta)\hat{Q}_{\text{opt}}(s,a)
+
\eta\left[r + \gamma \max_{a'} \hat{Q}_{\text{opt}}(s',a')\right]
$$

Main distinction:

- **SARSA** uses the value of the **action actually chosen next**
- **Q-learning** uses the **best possible next action** according to current estimates

So:

- SARSA learns the value of the current policy
- Q-learning aims at the optimal value function

---

### On-policy vs Off-policy
Q-learning is **off-policy**.

Why?

Because it can learn the optimal policy even while the data is generated by some other exploratory policy.

For example:

- behavior policy: $\epsilon$-greedy exploration
- target being learned: greedy optimal policy

So Q-learning can use data from one policy while learning values for another.

In contrast:

- **Model-free Monte Carlo** is typically on-policy
- **SARSA** is on-policy
- **Q-learning** is off-policy

---

### Comparison with the Earlier RL Methods

#### Model-based Monte Carlo
- estimates:
  - $\hat{T}(s,a,s')$
  - $\hat{R}(s,a,s')$
- then planning can recover optimal values

#### Model-free Monte Carlo
- estimates:
  - $\hat{Q}_\pi(s,a)$
- target:
  - full return $u$
- no bootstrapping

#### SARSA
- estimates:
  - $\hat{Q}_\pi(s,a)$
- target:
  - $r + \gamma \hat{Q}_\pi(s',a')$
- uses bootstrapping
- on-policy

#### Q-learning
- estimates:
  - $\hat{Q}_{\text{opt}}(s,a)$
- target:
  - $r + \gamma \max_{a'} \hat{Q}_{\text{opt}}(s',a')$
- uses bootstrapping
- off-policy

---

## Comparison Table: Four RL / Planning Approaches

| Method | What it Estimates | Main Update / Equation | Target / Based On | Policy Type | Main Idea |
|---|---|---|---|---|---|
| **Model-Based Monte Carlo** | $\hat{T}, \hat{R}$ | $\hat{T}(s,a,s') = \frac{\\\#(s,a,s')}{\\\#(s,a)}$ and estimate rewards from observed transitions | raw experience $(s,a,r,s')$ | effectively off-policy for model estimation | first learn the MDP model, then plan |
| **Model-Free Monte Carlo** | $\hat{Q}_\pi(s,a)$ | $\hat{Q}_ \pi(s,a) \leftarrow (1-\eta)\hat{Q}_ \pi(s,a) + \eta u$| full return $u = r_t + \gamma r_{t+1} + \cdots$ | on-policy | estimate value directly from complete sampled returns |
| **SARSA** | $\hat{Q}_\pi(s,a)$ | $\hat{Q}_ {\pi}(s,a) = (1-\eta) \hat{Q}_ \pi(s,a) + \eta\[r + \gamma \hat{Q} _ \pi(s',a')]$ | immediate reward + estimated next action value | on-policy | bootstrap using the action actually taken next |
| **Q-learning** | $\hat{Q}_{\text{opt}}(s,a)$ | $\hat{Q}_ {\text{opt}}(s,a) = (1-\eta) \hat{Q}_ {\text{opt}}(s,a) + \eta\[r + \gamma \max_{a'}\hat{Q}_ {\text{opt}}(s',a')]$ | immediate reward + best estimated next action value | off-policy | learn optimal action values directly without learning the model |

---

## Short Difference Summary

### 1. Model-Based Monte Carlo vs the rest
- Model-based MC learns the **environment model**
- The others learn **value functions directly**

### 2. Model-Free Monte Carlo vs SARSA
- Monte Carlo uses the **full return**
- SARSA uses **one-step bootstrapping**

So:
- Monte Carlo: lower bias, higher variance
- SARSA: higher bias, lower variance

### 3. SARSA vs Q-learning
- SARSA uses the next action actually selected by the policy
- Q-learning uses the maximum over next actions

So:
- SARSA learns $Q_\pi$
- Q-learning learns toward $Q_{\text{opt}}$

### 4. On-policy vs Off-policy
- Model-free Monte Carlo: on-policy
- SARSA: on-policy
- Q-learning: off-policy
- Model-based MC: model estimation itself does not depend strongly on the exact policy, as long as there is enough exploration


