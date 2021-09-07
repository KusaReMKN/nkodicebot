#ifndef MKNMAP_H
#define MKNMAP_H

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct node {
	struct node *l;	/* Left Tree */
	struct node *r;	/* Right Tree */
	void *k;	/* Key */
	void *v;	/* Value */
	size_t h;	/* Height of Tree */
} *mapitem;

typedef struct mknmap_t {
	struct node *tree;				/* Binary Tree */
	int (*keycmp)(const void *, const void *);	/* Comper for Key */
	void *(*keycpy)(void *, const void *);		/* Copier for Key */
	void *(*valcpy)(void *, const void *);		/* Copier for Value */
	void (*keyfree)(void *);		/* Releaser for Key */
	void (*valfree)(void *);		/* Releaser for Value */
} *mknmap;

extern mknmap NewMap(int (*keycmp)(const void *, const void *),
				void *(*keycpy)(void *, const void *),
				void *(*valcpy)(void *, const void *),
				void (*keyfree)(void *),
				void (*valfree)(void *));
extern void *GetItem(const mknmap map, const void *key);
extern int PutItem(mknmap map, const void *key, const void *val);
extern mapitem FirstItem(const mknmap map);
extern mapitem LastItem(const mknmap map);
extern int RemoveItem(mknmap map, const void *key);
extern int IsEmpty(const mknmap map);
extern void RemoveAll(mknmap map);
extern void DeleteMap(mknmap map);
extern void ForEach(mknmap map, void (*proc)(const void *, void *, va_list),
									...);

#endif /* MKNMAP_H */
