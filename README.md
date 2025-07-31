# Introduction
This is a quadtree implementation, based on this stackoverflow answer: https://stackoverflow.com/a/48330314. There is also a grid implementation, which has similar architecture.

# QuadTree

## Nodes
The Quadtree is composed of Nodes, which are just 4 ints in an array. 

The Nodes are stored in a **Nodes** vector, which gets appended to every time a **Node** splits.

**IF A NODE IS NOT A LEAF:** The 4 ints are the indicies of the children in the **Nodes** vector.

**IF A NODE IS A LEAF:** The 4 ints are repurposed to show its size and the index of the first element. 

## Elements and Things
**"Things"** refers to the actual things in the quadtree, the particles that are moving around and colliding.
**"Elements"** are the actual things that are in a **Node**. A particle is only ever added once as a thing but if it's in multiple **Nodes**, one element is added to each **Node**.

**Things** are stored in a "master list", which is a list of shared_ptrs pointing to each unique **Thing** in the tree. Implementation wise, this is a hash map from `Thing* -> MasterListEntry`. There is only ever one **MasterListEntry** for each Thing. A **MasterListEntry** is composed of a `std::shared_ptr<Thing>` and an `int`, which points to the first **Element** that points to **Thing**

**Elements** are the main workhorse struct of the whole thing. They are stored in a *vector*, which is very important. Every time an **Element** is added to a Node, it's simply pushed back to the end of this vector. This can mean **elements** of the same Node are very far apart in this massive array. So how do you know if elements are in the same Node? **Elements** store two `ints`, one for the next **Element** it points to, and one for the previous *Element*. Basically, all the **Elements** in a **Node** are in a linked list together, but instead of allocating the list in memory, we allocate it inside a vector ourselves. We *could* probably just use an `std::list`, but creating our own list entries means we can refer to positions in our linked list with `ints` rather than cringe iterators.

Each **Element** also holds a raw pointer to the **Thing** it corresponds to and an `int` for the index of the Node it is in.

The final field of **Element** is called `nextCopy`, which is the index of the next **Element** that *points to the same **Thing***.

## Adding to the tree
The Quadtree stores `shared_ptrs` so you have to pass in a `shared_ptr` when adding. Maybe this is not necessary, but this is how I implemented it, in case I ever need a structure  that also owns the objects. There is probably some free optimization by using raw pointers instead.

Recursing through the Quadtree is pretty self-explanatory: the `penetrate` function handles it for us. You give a **Thing**, and it recursively checks each **Node** to see if it collides, if so, it goes into that **Node**. The bounding rect of each **Node** is calculated on the fly, as opposed to being stored in each **Node**.

Eventually a leaf **Node** will be hit. For each leaf hit, an **Element** is created, which points to the **Thing** being added. The new **Element** is appended to the end of the **Elements** vector. Remember that leaf **Nodes** store the index of their first **element**; when a new **Element** is added to a **Node**, the first index field is updated to point to the index of the new **Element**. The new **Element** also has its `next` field point to the index of the previous first **Element** and the previous first **Element** has `prev` now points to the new **Element**.

The **MasterListEntry** that was created for the **Thing** being added is also recursively passed to each leaf, which allows the **MasterListEntry** to point to the new **Element** as new first **Element** that points to the **Thing**. 

If a **node** ever gets too many **Elements**, it splits, which changes its fields to point to each of its new 4 children, and moves each **Element** in. I currently assume that there will not need to be a bunch of splits that happen because all the **Elements** are clumped into one area; if this happens, one child will just awkwardly be at 1 over capacity until a new **Element**  is added to it, at which point it too will split.

**Nodes** will NOT split if they are past max depth, which can be set when constructing the Quadtree. 

## Removing from the tree
Because the **MasterList** is an `std::unordered_map`, it allows us to look up the corresponding **MasterListEntry** for each **Thing** in the tree. This is where a lot of infrastructure that we've been maintaining comes in. Remember how a **MasterListEntry** stores the index of the first **Element** that points to the same **Thing** as the entry. That first **Element** in turns points to the next **Element** that holds the same **Thing** via the `nextCopy` field. By doing this, we can loop through each **Element** that points to the same **Thing** and remove each one. 

When an **Element** is removed from a node, the `prev` field allows us to have the previous **Element** point to the next **Element**. We also subtract 1 from the size of the corresponding **Node** and update the **Node's** index to its first **Element** if necessary.

Currently, empty leaves are not removed; I was just too lazy to do this.

## Finding Collisions
Finding collisions rn is slightly ineffecient: if you want to find all collisions within R radius of point X, it would instead search in a rectangular area and only allow the **nodes** that collide with the circle you provided. Not sure how to make that much better. In my simulation, everything is a circle, so finding collisions involves calling the `forEachNearby` function on all collisions within the radius of that circle.

## Smart Vector
Remember when I said **Elements** are stored in a `vector`? I lied, kind of. The stackoverflow guy  talks about a data structure, which is a Vector except it keeps track of holes when things are removed. It's kind of genius. So instead of a `vector` of, say **Elements**, it's a `vector` of `unions`, each `union` is either an `Element` or an `int`, the `int` being the index of the next hole in the `vector`. It's the exact same principle where distant **Elements** can point to each other in a `vector` and form a linked list. I can not lie, it's pretty genius, as long as the thing you are storing is *trivially destructible*. I used my version, which called **SmartVector** any time I could, so for both **Elements** and **Nodes**. 


## Considerations
There are two branches, `big` and `small`. This is due to two different implementations of **Element** and any corresponding architecture. In the original StackOverflow post, the author's **Element** struct only contains two fields: the **Thing** he's pointing to and the next **Element**. Our **Elements** have ballooned to a whopping 3 extra `ints`: the previous **Element**, the **Node** we are in, and the **nextCopy**. These 3 fields are ALL in service of removing elements as fast as possible. If we didn't have these 3 fields, we would have to find each leaf that a **Thing** is in, and linear search each leaf. This allows us to just iterate through each **Element** and remove them directly, no searching needed. `big` refers to this larger implementation of **Element**, whereas `small` refers to an implementation with a smaller **Element**, more akin to the original post.

- We need the `prev` field because after we remove an **Element**, we need to "stitch" the linked list back together by having the previous **Element** point to the next **Element**.
- We need the `nextCopy` field because that's how this whole thing works, it keeps track of the next copy of **Thing**.
- And we need `node` field because we need to update the size of the parent **Node** after removing. 

The alternative is to remove these 3 fields and recurse down to each leaf, similar to adding, finding the corresponding **Element**, then removing each one. This is akin to finding a collision: you have to iterate through each **Element** of each leaf anyway, so it should be a fairly fast operation.

## Performance
For simplicity, and because it looks cool, I had elements colliding absorb the color of the other element. They basically just set their color to the other element. They do not SWAP colors, the first element processed takes the 2nd element's color. 

On my shitty laptop with:

100 elements per leaf, 10000 elements, each particle is 10 radius, 5 maxDepth, no drawing the tree itself, with drawing each particle, Release mode, -O3:

- `small` hit about 16-18 FPS
- `big` hit about 18 - 20
- `big` jumped to about 29-31 FPS, if rendering was turned off.

So at least on my hardware, using more space was worth it.


# Grid
The main use case of a Quadtree is that it can split and adapt to different particle densities; if the particles all clump in one area, the quadtree can adapt to that. But in my case, the particles were very evenly distributed. So I wrote up a much simpler Grid structure. This thing just divides the a space into squares; there are X squares per row, and each square is some width. When an element fits into a square, it gets added to it. The architecture is almost the same as Quadtree; we still use SmartVector and the "linked list within a vector" type idea a lot. However, **Element** can be made slightly smaller since we no longer care about **Node** size; the only time we care about **Node** is when an **Element** is the first **Element** of a node, so the `prev` field is changed to be the next **Element** if positive, the **Node** it belongs to if negative. In fact, the only thing we care about for each **Node** is the first **Element** index, so **Nodes** are now just ints. 

Grid relies heavily on **Things** being able to calculate their bounding box, as this determines where they go. This basically replaces recursing down to the correct leaf, and is where a LOT of the optimization comes from (probably), as we can now directly calculate where each **Thing** goes without having to do any recursion. 

## Performance
On my shitty laptop with:

10000 elements,  each particle is 10 radius, no drawing the grid itself, with drawing each particle, Release mode, -O3, 1920 width, each node is 100 pixels big:

- We run at a whopping 34-36 FPS, never lower than 30 even on a bad day.
- **WITHOUT RENDERING THE PARTICLES, WE RUN AT NEARLY 60 FPS!!**

Definitely a lot better and arguably easier to implement, albeit mainly specialized for this use case. 