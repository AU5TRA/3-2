# Prompting Techniques

## 1. Output Parameters You Should Know

These are not prompting techniques by themselves, but they strongly affect how a prompt behaves.

### Output length / max tokens
- Controls how long the response can be.
- A lower limit **does not automatically make the answer concise**; it only cuts generation earlier.
- Useful when you want short labels, JSON fields, or to prevent long rambling outputs.
- Especially important for multi-step methods like **ReAct**, where the model may keep producing extra text if left unconstrained.

**Use it when:** you need fixed-size output, faster responses, or lower cost.

**Example:**
> Return the answer in **one sentence**. Max 40 tokens.

### Temperature
- Controls randomness.
- **Low temperature** -> more stable, deterministic, factual.
- **High temperature** -> more diverse, creative, less predictable.

**Good defaults:**
- Reasoning, classification, extraction, math: low (`0` to `0.2`)
- Brainstorming, writing, idea generation: higher (`0.7` to `1`)

**Example:**
- Tax form classification -> low temperature
- Ad slogan brainstorming -> high temperature

### Top-K
- Restricts the next token choice to the top `K` likely tokens.
- Lower Top-K -> safer, narrower, more factual
- Higher Top-K -> more varied, more exploratory

**Simple intuition:** Top-K asks, “From how many candidate tokens may the model choose?”

### Top-P (nucleus sampling)
- Chooses from the smallest set of tokens whose cumulative probability reaches `P`.
- Lower Top-P -> conservative
- Higher Top-P -> more flexible

**Simple intuition:** Top-P asks, “How much of the probability mass should be included before sampling?”

### Practical rule of thumb
- **Single correct answer tasks**: use low temperature, often `0`
- **Creative generation**: use higher temperature, optionally higher Top-K / Top-P
- **Structured output**: also explicitly ask for the structure in the prompt

---

## 2. Core Prompting Techniques

## Zero-shot prompting
### Pattern
Give the task directly without examples.

### Best for
- Simple classification
- Straightforward summarization
- Basic extraction
- When the task is already obvious to the model

### Strength
- Fast and simple
- Low prompt length

### Weakness
- Can fail when the format is unusual or the task is subtle

### Small example
> Classify the sentiment of this review as POSITIVE, NEGATIVE, or NEUTRAL: “The visuals were stunning but the story was weak.”

### Why choose it in an exam scenario?
Choose zero-shot when the task is **clear, standard, and does not need demonstration**.

---

## One-shot prompting
### Pattern
Give exactly one example, then ask for a similar output.

### Best for
- Teaching one output format
- Getting the model to imitate a pattern
- Slightly more controlled output than zero-shot

### Strength
- Cheap way to demonstrate the target style

### Weakness
- One example may not cover edge cases

### Small example
> Example: “Meeting moved to 3 PM” -> `{ "type": "schedule_change" }`
>
> Now classify: “Tomorrow’s class starts at 10 instead of 9.”

### Why choose it?
Use one-shot when **format matters**, but the task is still not complex enough to need many examples.

---

## Few-shot prompting
### Pattern
Provide several examples that illustrate the desired mapping or format.

### Best for
- Structured extraction
- Classification with subtle classes
- Style imitation
- Pattern-based tasks

### Strength
- Stronger guidance than zero-shot or one-shot
- Helps the model infer the pattern you want

### Weakness
- Consumes context length
- Bad examples can confuse the model

### Small example
> Input: “Large coffee, no sugar” -> `{ "drink": "coffee", "size": "large", "sugar": false }`
>
> Input: “Medium tea with sugar” -> `{ "drink": "tea", "size": "medium", "sugar": true }`
>
> Now parse: “Small coffee with sugar”

### Why choose it?
Choose few-shot when the examiner’s scenario involves **format sensitivity, ambiguity, edge cases, or label confusion**.

---

## System prompting
### Pattern
Add high-level instructions about what the model should do and how the output should look.

### Best for
- Enforcing output rules
- Safety/tone control
- JSON / schema-based responses
- Restricting verbosity

### Strength
- Good for controlling behavior globally
- Helpful when exact format matters

### Weakness
- By itself, it does not supply task-specific examples

### Small example
> You are an information extraction assistant. Return **valid JSON only** with fields `name`, `deadline`, and `priority`.

### Why choose it?
Use system prompting when the key need is **control**: format, tone, safety, or strict structure.

---

## Role prompting
### Pattern
Assign the model a role or identity.

### Best for
- Tone adaptation
- Audience-specific explanations
- Domain-oriented communication

### Strength
- Improves voice, framing, and relevance

### Weakness
- Does not automatically improve reasoning accuracy
- Can sound polished but still be wrong

### Small example
> Act as a patient high-school physics tutor. Explain capacitance using a water-tank analogy.

### Why choose it?
Choose role prompting when the scenario depends on **who is speaking** or **how the answer should sound**.

---

## Contextual prompting
### Pattern
Provide relevant background information for the current task.

### Best for
- Personalized answers
- Task-specific constraints
- Situations where missing context would cause weak answers

### Strength
- Makes the answer more relevant and grounded

### Weakness
- Too much irrelevant context can distract the model

### Small example
> Context: The user is a first-year CSE student who knows Python but not calculus. Explain gradient descent.

### Why choose it?
Use contextual prompting when the task requires **situational background** more than examples.

---

## Step-back prompting
### Pattern
First ask a broader conceptual question, then use that answer to solve the specific task.

### Best for
- Problems where principles matter
- Creative design tasks
- Analytical tasks where direct answering becomes generic or shallow

### Strength
- Activates relevant background knowledge first
- Often improves quality and depth

### Weakness
- Slower than direct prompting
- Overkill for simple tasks

### Small example
> Step 1: What makes a good emergency evacuation plan for a school?
>
> Step 2: Using those principles, design an evacuation plan for a 6-story school building with two staircases.

### Why choose it?
Choose step-back when the scenario asks for a **specific answer built on general principles**.

---

## Chain of Thought (CoT)
### Pattern
Ask the model to reason through intermediate steps before answering.

### Best for
- Multi-step reasoning
- Logic, math, planning
- Questions where a direct answer is error-prone

### Strength
- Often improves reasoning accuracy
- Makes the path to the answer visible

### Weakness
- Longer output
- More cost and latency

### Small example
> A bus had 42 passengers. 17 got off and 9 got on. How many are on the bus now? Think step by step.

### Why choose it?
Use CoT when the task can be solved by **breaking it into steps**, especially when a wrong direct answer is likely.

---

## Self-consistency
### Pattern
Generate multiple reasoning paths for the same problem and choose the most common final answer.

### Best for
- Difficult reasoning tasks
- Ambiguous classification
- Cases where one chain of reasoning may be unreliable

### Strength
- More robust than one single CoT run
- Reduces chance of a lucky but wrong chain

### Weakness
- Expensive: multiple generations are required
- Usually not ideal for low-cost real-time systems

### Small example
> Suppose a tricky email might be phishing or harmless. Instead of trusting one answer, run the CoT prompt several times and take the majority label.

### Why choose it?
Choose self-consistency when **accuracy matters more than cost**, and one reasoning trace may not be dependable.

---

## Tree of Thoughts (ToT)
### Pattern
Explore multiple reasoning branches, not just one linear chain.

### Best for
- Search-heavy problems
- Puzzle solving
- Planning with alternatives
- Tasks where early choices strongly affect the final result

### Strength
- Better for exploration than normal CoT
- Can compare candidate paths before committing

### Weakness
- Much more complex and expensive
- Often unnecessary for routine tasks

### Small example
> Plan a 2-day hackathon schedule while balancing mentor availability, room constraints, judging time, and meal breaks. Explore multiple scheduling strategies before selecting one.

### Why choose it?
Choose ToT when the scenario involves **branching choices, backtracking, or search over alternatives**.

---

## ReAct (Reason + Act)
### Pattern
The model alternates between reasoning and tool use: think -> act -> observe -> continue.

### Best for
- Tasks needing external information
- Retrieval, search, calculator, code execution, API calls
- Agent-like workflows

### Strength
- Lets the model gather missing facts instead of guessing
- Strong for real-world multi-step tasks

### Weakness
- Requires tool setup
- More engineering overhead

### Small example
> Find the cheapest nonstop flight from Dhaka to Bangkok next Friday, compare baggage rules, then summarize the best option.

This is a good ReAct scenario because the model should search, compare, and update its reasoning based on retrieved information.

### Why choose it?
Choose ReAct when the model **cannot solve the task from memory alone** and must use tools or external data.

---

## Automatic Prompt Engineering (APE)
### Pattern
Use an LLM to generate candidate prompts, evaluate them, and keep the best ones.

### Best for
- Prompt optimization at scale
- Repeated production use
- Benchmarking prompt variants

### Strength
- Useful when humans want to systematically improve prompts

### Weakness
- Not usually the answer for a one-off scenario question in an exam

### Small example
> Generate 10 alternative prompts for extracting invoice fields, test them on a validation set, and keep the best-performing version.

### Why choose it?
Choose APE only when the scenario is about **building or optimizing a prompting pipeline**, not just solving one task.

---

## Code prompting
This is not one single technique, but a family of prompt use-cases for programming.

### Common forms
- **Write code**
- **Explain code**
- **Translate code**
- **Debug/review code**

### Best for
- Software development support
- Code learning
- Rapid prototyping

### Small examples
> Write code: “Write a Python script to rename all `.txt` files in a folder.”

> Explain code: “Explain this recursion function line by line.”

> Translate code: “Convert this Bash script into Python.”

> Debug/review code: “Find the bug and suggest improvements.”


### Why choose it?
Choose code prompting when the task is clearly about **program generation, interpretation, conversion, or debugging**.

---

## 3. Quick Comparison: Which Technique Fits Which Situation?

| Situation | Best pick | Why |
|---|---|---|
| Simple, direct task | Zero-shot | Minimal instruction is enough |
| Need one clear pattern | One-shot | One example teaches the format |
| Need reliable pattern following | Few-shot | Multiple examples reduce ambiguity |
| Need strict output format | System prompting | Best for JSON, schema, labels, rules |
| Need a specific tone or persona | Role prompting | Controls style and voice |
| Need task background | Contextual prompting | Supplies relevant situation details |
| Need principles before specifics | Step-back | Broad reasoning improves the final task |
| Need stepwise reasoning | CoT | Breaks down the problem |
| Need more reliable reasoning than one CoT | Self-consistency | Majority vote over several reasoning paths |
| Need exploration over alternatives | ToT | Supports branching search |
| Need tools or live information | ReAct | Combines reasoning with actions |
| Need to optimize prompts systematically | APE | Generates and evaluates prompts |
| Need programming help | Code prompting | Specialized for code tasks |

---

## 4. Common Exam Logic: How to Argue Your Choice

When you are given a scenario, justify your pick using this structure:

1. **What is the task type?**
   - classification, reasoning, planning, creative writing, retrieval, coding?
2. **What is the main difficulty?**
   - ambiguity, missing context, need for format, multi-step reasoning, external data?
3. **Which technique directly addresses that difficulty?**
4. **Why are simpler techniques not enough?**
5. **What settings would you use?**
   - low temperature for factual/reasoning tasks, higher for creative tasks

### Example argument template
> I would choose **few-shot prompting** because the task requires the model to follow a specific label pattern, and zero-shot prompting may interpret borderline cases inconsistently. A few diverse examples make the class boundaries clearer and improve output consistency. Since this is not a creative task, I would use a low temperature.

---

## 5. Scenario-Based Practice Examples

## Scenario 1: Scholarship application screening
You need to classify applicant statements into `ACADEMIC`, `FINANCIAL`, `PERSONAL HARDSHIP`, or `OTHER`.

### Best technique
**Few-shot prompting**

### Why
The classes are subtle and can overlap. A few balanced examples help the model learn the differences between similar categories.

### Better than
- **Zero-shot**: may confuse overlapping classes
- **Role prompting**: tone is not the main issue
- **CoT**: possible, but examples are more useful here than step-by-step reasoning

---

## Scenario 2: Explain database normalization to a weak student
The student is in first year, dislikes theory, and understands examples better than definitions.

### Best technique
**Role prompting + contextual prompting**

### Why
The role sets the teaching style, and the context tells the model about the learner’s level and preferences.

### Better than
- **Zero-shot**: too generic
- **Few-shot**: examples may help, but audience adaptation is the real need

---

## Scenario 3: Solve a tricky logical scheduling problem
A lab has 4 groups, 2 rooms, 1 projector, limited timeslots, and several constraints.

### Best technique
**Chain of Thought**, possibly **Tree of Thoughts** if the constraints are dense

### Why
This is a multi-step reasoning task. If several paths must be compared, ToT becomes stronger than plain CoT.

### Better than
- **Role prompting**: style does not solve the logic
- **Few-shot**: examples may not generalize well to a new constraint structure

---

## Scenario 4: Find the latest government circular and summarize it
The answer depends on the newest published document.

### Best technique
**ReAct**

### Why
The model needs to search, retrieve, inspect, and then summarize. This is not just reasoning; it requires actions.

### Better than
- **CoT**: reasoning alone cannot fetch the latest circular
- **Contextual prompting**: background is not enough without retrieval

---

## Scenario 5: Design a disaster-response poster slogan campaign
You want 20 diverse slogan ideas for flood awareness.

### Best technique
**Zero-shot or role prompting with higher temperature**

### Why
This is creative generation. Diversity matters more than deterministic correctness.

### Better than
- **CoT**: unnecessary unless you also want justification of each slogan
- **Few-shot**: only needed if you want a very specific style pattern

---

## Scenario 6: Write policy-compliant JSON from messy customer messages
The output must always match a fixed schema.

### Best technique
**System prompting**, optionally combined with **few-shot**

### Why
Strict structure is the main requirement. If the messages are messy, a few examples further improve consistency.

### Better than
- **Role prompting**: irrelevant
- **CoT**: may add unnecessary verbosity unless reasoning is needed internally

---

## 6. Final Takeaways to Memorize

- **Zero-shot** -> simplest direct instruction
- **One-shot** -> teach with one example
- **Few-shot** -> teach a pattern with several examples
- **System** -> control format and behavior
- **Role** -> control voice and perspective
- **Contextual** -> provide background for relevance
- **Step-back** -> think generally before answering specifically
- **CoT** -> solve step by step
- **Self-consistency** -> run several CoTs and vote
- **ToT** -> explore multiple branches
- **ReAct** -> reason and use tools
- **APE** -> generate and optimize prompts automatically
- **Code prompting** -> write, explain, translate, or debug code

### One-line memory aid
> If the issue is **pattern**, use **few-shot**; if it is **format**, use **system**; if it is **audience/tone**, use **role/context**; if it is **reasoning**, use **CoT**; if it is **search/tools**, use **ReAct**.
