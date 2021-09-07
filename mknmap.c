#include "mknmap.h"

/*
 * HeightOf -- Get Tree Height
 */
static size_t HeightOf(const struct node *p)
{
	return p == NULL ? 0 : p->h;
}

/*
 * HigherOf -- Choose Higher
 */
static struct node *HigherOf(const struct node *a, const struct node *b)
{
	size_t ah = HeightOf(a), bh = HeightOf(b);

	return (struct node *)(ah > bh ? a : b);
}

/*
 * CalcHeight -- Calculate Height
 */
static size_t CalcHeight(const struct node *p)
{
	return p == NULL ? 0 : HeightOf(HigherOf(p->l, p->r)) + 1;
}

/*
 * BiasOf -- Calculate Bias
 */
static int BiasOf(const struct node *p)
{
	return p == NULL ? 0 : (int)HeightOf(p->l) - (int)HeightOf(p->r);
}

/*
 * RotateR, RotateL -- Tree Rotation
 */
static struct node *RotateR(struct node *p)
{
	struct node *tmp = p == NULL ? NULL : p->l;

	if (tmp == NULL) return p;	/* cannot be rotatable */
	p->l = tmp->r;
	tmp->r = p;
	p->h = CalcHeight(p);
	tmp->h = CalcHeight(tmp);
	return tmp;
}

static struct node *RotateL(struct node *p)
{
	struct node *tmp = p == NULL ? NULL : p->r;

	if (tmp == NULL) return p;	/* cannot be rotatable */
	p->r = tmp->l;
	tmp->l = p;
	p->h = CalcHeight(p);
	tmp->h = CalcHeight(tmp);
	return tmp;
}

/*
 * RotateLR, RotateRL -- Tree Double-Rotation
 */
static struct node *RotateLR(struct node *p)
{
	if (!p || !p->l) return p;	/* cannot be rotatable */
	p->l = RotateL(p->l);
	return RotateR(p);
}

static struct node *RotateRL(struct node *p)
{
	if (!p || !p->r) return p;	/* cannot be rotatable */
	p->r = RotateR(p->r);
	return RotateL(p);
}

/*
 * Balance -- Balance Tree
 */
static struct node *Balance(struct node *p)
{
	int b = BiasOf(p);

	if (abs(b) < 2) return p;	/* already balanced */
	if (b < 0) p = BiasOf(p->r) < 0 ? RotateL(p) : RotateRL(p);
	else p = BiasOf(p->l) > 0 ? RotateR(p) : RotateLR(p);
	return p;
}

/*
 * NodeOf -- Get the Node that maps to Key
 */
static struct node *NodeOf(const struct node *root, const void *key,
				int (*keycmp)(const void *, const void *))
{
	int cmp;

	while (root != NULL && (cmp = (*keycmp)(root->k, key)) != 0)
		root = cmp < 0 ? root->l : root->r;
	return (struct node *)root;
}

/*
 * RefToRelNodesOf -- Get the List of Reference to Related Nodes
 */
static struct node ***RefToRelNodesOf(const struct node *root, const void *key,
				int (*keycmp)(const void *, const void *))
{
	struct node ***v = NULL, ***tmp;
	size_t len = 1;
	int cmp = 1;

	if ((v = malloc(sizeof(*v))) == NULL) return NULL;
	while (root != NULL && (cmp = (*keycmp)(root->k, key)) != 0) {
		if ((tmp = realloc(v, ++len * sizeof(*v))) == NULL) {
			free(v);
			return NULL;
		}
		v = tmp;
		if (cmp < 0) {
			v[len - 2] = (struct node **)&root->l;
			root = root->l;
		} else {
			v[len - 2] = (struct node **)&root->r;
			root = root->r;
		}
	}
	v[len - 1] = NULL;
	return v;
}

/*
 * LengthOf -- Calculate Langth Of List
 */
static size_t LengthOf(struct node ***v)
{
	size_t i;

	for (i = 0; v[i] != NULL; i++);
	return i;
}

/*
 * NewNode -- Create New Node
 */
static struct node *NewNode(void)
{
	struct node *p = malloc(sizeof(*p));

	if (p == NULL) return NULL;
	p->l = p->r = NULL;
	p->k = p->v = NULL;
	p->h = 1;
	return p;
}

/*
 * Insert -- Insert a Node to Tree
 */
static struct node *Insert(struct node *root, const void *key, const void *val,
				int (*keycmp)(const void *, const void *),
				void *(*keycpy)(void *, const void *),
				void *(*valcpy)(void *, const void *))
{
	struct node ***v = RefToRelNodesOf(root, key, keycmp);
	struct node **pdest, *dest, *tmp;
	size_t l = 0;

	if (v == NULL) return NULL;
	if (root == NULL) {
		if ((root = dest = NewNode()) == NULL) {
			free(v);
			return NULL;
		}
	} else {
		l = LengthOf(v);
		pdest = l == 0 ? &root : v[l - 1];
		if ((dest = *pdest) == NULL) {
			if ((dest = NewNode()) == NULL) {
				free(v);
				return NULL;
			}
			*pdest = dest;
		}
	}
	dest->k = (*keycpy)(dest->k, key);
	dest->v = (*valcpy)(dest->v, val);
	for (l > 0 && l--; l > 0 && BiasOf(*v[l - 1]) != 0; l--) {
		(*v[l - 1])->h = CalcHeight(*v[l - 1]);
		if ((tmp = Balance(*v[l - 1])) != *v[l - 1]) {
			*v[l - 1] = tmp;
			break;
		}
	}
	free(v);
	return Balance(root);
}

/*
 * MinOf, MaxOf -- Return Maximum/Minimum Node
 */
static struct node *MinOf(const struct node *p)
{
	while (p != NULL && p->l != NULL)
		p = p->l;
	return (struct node *)p;
}

static struct node *MaxOf(const struct node *p)
{
	while (p != NULL && p->r != NULL)
		p = p->r;
	return (struct node *)p;
}

/*
 * IsLeaf -- true if the Node is Leaf Node
 */
static int IsLeaf(const struct node *p)
{
	return p == NULL || p->l == NULL && p->r == NULL;
}

/*
 * AChildOf -- return a Child Node if the Node has only one child
 */
static struct node *AChildOf(const struct node *p)
{
	if (p == NULL || !((p->l == NULL) ^ (p->r == NULL))) return NULL;
	return p->l != NULL ? p->l : p->r;
}

/*
 * RefToRelNodesOfNextToLastOf -- Append to List of Reference to Related Nodes
 */
static struct node ***RefToRelNodesOfNextToLastOf(struct node ***v)
{
	struct node ***tmp, *p;
	size_t l;

	if (v == NULL || (l = LengthOf(v)) == 0) return v;
	p = *v[l - 1];
	l += 2;
	if ((tmp = realloc(v, l * sizeof(*v))) == NULL) {
		free(v);
		return NULL;
	}
	v = tmp;
	v[l - 2] = &p->r;
	p = p->r;

	while (p != NULL && p->l != NULL) {
		if ((tmp = realloc(v, ++l * sizeof(*v))) == NULL) {
			free(v);
			return NULL;
		}
		v = tmp;
		v[l - 2] = &p->l;
		p = p->l;
	}
	v[l - 1] = NULL;
	return v;
}

/*
 * RemoveNode -- Remove the Node
 */
static void RemoveNode(struct node *p, void (*keyfree)(void *),
				void (*valfree)(void *))
{
	if (p == NULL) return;
	(*keyfree)(p->k);
	(*valfree)(p->v);
	free(p);
	return;
}

/*
 * Remove -- Remove Node from Tree
 */
static struct node *Remove(struct node *root, const void *key,
				int (*keycmp)(const void *, const void *),
				void (*keyfree)(void *),
				void (*valfree)(void *))
{
	struct node ***v = RefToRelNodesOf(root, key, keycmp), ***vtmp;
	struct node **pdest, **pnext;
	struct node *dest, *tmp, *next;
	size_t l, ldest;

	if (v == NULL) return NULL;
	if ((l = LengthOf(v)) == 0) {
		if ((vtmp = realloc(v, ++l * sizeof(*v))) == NULL) {
			free(v);
			return NULL;
		}
		v = vtmp;
		v[0] = &root;
		v[1] = NULL;
	}
	pdest = v[l - 1];
	if ((dest = *pdest) == NULL) return root;	/* not found */

	if (IsLeaf(dest)) {
		*pdest = NULL;
		v[--l] = NULL;
	} else if ((tmp = AChildOf(dest)) != NULL) {
		*pdest = tmp;
		v[--l] = NULL;
	} else {
		ldest = l;
		if ((v = RefToRelNodesOfNextToLastOf(v)) == NULL) return NULL;
		l = LengthOf(v);
		pnext = v[--l];
		next = *pnext;
		*pnext = AChildOf(next);
		*pdest = next;
		next->r = dest->r;
		next->l = dest->l;
		v[ldest] = &next->r;
	}
	RemoveNode(dest, keyfree, valfree);

	for (l > 0 && l--; l > 0; l--) {
		(*v[l - 1])->h = CalcHeight(*v[l - 1]);
		*v[l - 1] = Balance(*v[l - 1]);
	}
	free(v);
	return Balance(root);
}

/*
 * RemoveTree -- Remove all node from Tree
 */
static struct node *RemoveTree(struct node *root, void (*keyfree)(void *),
				void (*valfree)(void *))
{
	if (root == NULL) return NULL;
	RemoveTree(root->l, keyfree, valfree);
	RemoveTree(root->r, keyfree, valfree);
	RemoveNode(root, keyfree, valfree);
	return NULL;
}

/*****************************************************************************/

/*
 * NewMap -- Create New Map
 */
mknmap NewMap(int (*keycmp)(const void *, const void *),
				void *(*keycpy)(void *, const void *),
				void *(*valcpy)(void *, const void *),
				void (*keyfree)(void *),
				void (*valfree)(void *))
{
	mknmap map = malloc(sizeof(*map));

	if (map == NULL) return NULL;
	map->tree = NULL;
	map->keycmp  = keycmp;
	map->keycpy  = keycpy;
	map->valcpy  = valcpy;
	map->keyfree = keyfree;
	map->valfree = valfree;
	return map;
}

/*
 * GetItem -- Get Item from Map
 */
void *GetItem(const mknmap map, const void *key)
{
	struct node *tmp = NodeOf(map->tree, key, map->keycmp);

	return tmp == NULL ? NULL : tmp->v;
}

/*
 * PutItem -- Put Item to Map
 */
int PutItem(mknmap map, const void *key, const void *val)
{
	struct node *tmp = Insert(map->tree, key, val, map->keycmp,
						map->keycpy, map->valcpy);

	if (tmp == NULL) return !0;
	map->tree = tmp;
	return 0;
}

/*
 * FirstItem -- Get Pointer to First Item
 */
mapitem FirstItem(const mknmap map)
{
	return MinOf(map->tree);
}

/*
 * LastItem -- Get Pointer to Last Item
 */
mapitem LastItem(const mknmap map)
{
	return MaxOf(map->tree);
}

/*
 * RemoveItem -- Remove Item from Map
 */
int RemoveItem(mknmap map, const void *key)
{
	struct node *tmp = Remove(map->tree, key, map->keycmp, map->keyfree,
								map->valfree);

	//if (tmp == NULL) return !0;
	map->tree = tmp;
	return 0;
}

/*
 * IsEmpty -- True if Map has no Item
 */
int IsEmpty(const mknmap map)
{
	return map->tree == NULL;
}

/*
 * RemoveAll -- Remove all Items from Map
 */
void RemoveAll(mknmap map)
{
	map->tree = RemoveTree(map->tree, map->keyfree, map->valfree);
	return;
}

/*
 * DeleteMap -- Delete the Map
 */
void DeleteMap(mknmap map)
{
	RemoveTree(map->tree, map->keyfree, map->valfree);
	free(map);
	return;
}

/*
 * Traversal -- Traversal Tree
 */
void Traversal(struct node *root, void (*proc)(const void *, void *, va_list),
								va_list ap)
{
//	size_t StackSize = 32, Level = 32;
//	struct node **stack = NULL;
//	size_t sp = 0;
//
//	if (!(stack = malloc(StackSize * sizeof(*stack)))) return;
//	stack[sp++] = root;
//
//	while (sp) {
//		struct node *next = stack[sp - 1];
//		int finishedSubtrees = (next->l == root || next->r == root);
//		
//		if (finishedSubtrees || IsLeaf(next)) {
//			stack[--sp];
//			(*proc)(next->k, next->v, ap);
//			root = next;
//		} else {
//			if (sp == StackSize) {
//				StackSize += Level;
//				stack = realloc(stack,
//						StackSize * sizeof(*stack));
//				if (!stack) return;
//			}
//			if (next->r != NULL) stack[sp++] = next->r;
//			if (sp == StackSize) {
//				StackSize += Level;
//				stack = realloc(stack,
//						StackSize * sizeof(*stack));
//				if (!stack) return;
//			}
//			if (next->l != NULL) stack[sp++] = next->l;
//		}
//	}
//	free(stack);

	if (root->r) Traversal(root->r, proc, ap);
	(*proc)(root->k, root->v, ap);
	if (root->l) Traversal(root->l, proc, ap);
	return;
}

/*
 * ForEach -- Execute the function once for each element
 */
void ForEach(mknmap map, void (*proc)(const void *, void *, va_list), ...)
{
	va_list ap;
	va_start(ap, proc);
	if (map->tree) Traversal(map->tree, proc, ap);
	va_end(ap);
	return;
}
